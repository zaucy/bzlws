#include "bzlws_tool_lib.hh"

#include <fstream>
#include <array>
#include <cstdio>
#include <memory>
#include <sstream>
#include <cstring>
#include <string>
#include <iostream>
#include <yaml-cpp/yaml.h>
#include "tools/cpp/runfiles/runfiles.h"

#if _WIN32
	#define popen _popen
	#define pclose _pclose 
#endif // _WIN32

namespace fs = bzlws_tool_lib::fs;
using bzlws_tool_lib::bazelignore_parse_results;
using bzlws_tool_lib::src_info;

namespace {
	struct bazel_label_info {
		std::string name;
		std::string package;
		std::string workspace_name;
	};

	bazel_label_info parse_label_string
		( const std::string& label_str
		)
	{
		bazel_label_info info;
		auto ws_name_end = label_str.find("//");

		if(ws_name_end == std::string::npos) return info;
		info.workspace_name = label_str.substr(1, ws_name_end - 1);

		auto package_name_end = label_str.find_last_of(":");
		if(package_name_end == std::string::npos) return info;
		info.package = label_str.substr(
			ws_name_end + 2,
			package_name_end - (ws_name_end + 2)
		);
		info.name = label_str.substr(package_name_end + 1);

		return info;
	}

	[[noreturn]]
	void tool_exit
		( bzlws_tool_lib::exit_code code
		)
	{
		std::exit(static_cast<int>(code));
	}

	std::string get_apparent_repo_name(const std::string& canonical) {
		if (canonical.empty()) return "";

		if (canonical[0] == '+') {
			auto last_plus = canonical.find_last_of('+');
			if (last_plus != std::string::npos && last_plus < canonical.size() - 1) {
				return canonical.substr(last_plus + 1);
			}
		}

		auto last_tilde = canonical.find_last_of('~');
		if (last_tilde != std::string::npos) {
			auto first_tilde = canonical.find_first_of('~');
			if (first_tilde == last_tilde) {
				return canonical.substr(0, first_tilde);
			} else {
				return canonical.substr(last_tilde + 1);
			}
		}

		return canonical;
	}
}

bool bzlws_tool_lib::force_remove
	( const fs::path& path
	, std::error_code& ec
	) noexcept
{
	fs::permissions(path, 
		fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write, 
		fs::perm_options::add, ec);
	if(ec) {
		// Ignore permission change errors, try to remove anyway
		ec.clear();
	}

	return fs::remove(path, ec);
}

void bzlws_tool_lib::trim_ws
	( std::string& s
	)
{
	using std::isspace;
	using std::find_if;

	auto not_isspace = [](int c) { return !isspace(c); };
	s.erase(s.begin(), find_if(s.begin(), s.end(), not_isspace));
	s.erase(find_if(s.rbegin(), s.rend(), not_isspace).base(), s.end());
}

fs::path bzlws_tool_lib::get_build_workspace_dir() {
	char* bwd_env = std::getenv("BUILD_WORKSPACE_DIRECTORY");
	if(bwd_env != nullptr) {
		return fs::path(bwd_env).make_preferred();
	}

	std::cerr
		<< "[ERROR] Missing 'BUILD_WORKSPACE_DIRECTORY' environment variable"
		<< std::endl;
	tool_exit(exit_code::missing_environment_variable);
}

bool bazelignore_parse_results::is_ignored_path
	( const fs::path& path
	)
{
	auto pathPrefix = fs::absolute(path).make_preferred().string();

	for(auto ignore_path : ignore_paths) {
		auto ignorePathPrefix =
			fs::absolute(ignore_path).make_preferred().string();

		if(pathPrefix.rfind(ignorePathPrefix) == 0) {
			return true;
		}
	}

	return false;
}

void bazelignore_parse_results::assert_ignored_path
	( const fs::path& path
	)
{
	if(!is_ignored_path(path)) {
		std::cerr
			<< "[ERROR] \"" << path.generic_string()
			<< "\" is not within a bazel ignored directory. "
			<< "Please check your .bazelignore file" << std::endl;
		tool_exit(exit_code::bazelignore_error);
	}
}

bazelignore_parse_results bzlws_tool_lib::parse_bazelignore
	( const fs::path& workspace_dir
	)
{
	auto bazelignore_path = workspace_dir / ".bazelignore";

	{
		std::error_code ec;
		auto bazelignore_exists = fs::exists(bazelignore_path, ec);
		if(ec) {
			std::cerr
				<< "Error while checking if " << bazelignore_path << " exists: "
				<< ec.message()
				<< std::endl;
			tool_exit(exit_code::filesystem_error);
		}

		if(!bazelignore_exists) {
			std::cerr
				<< "[ERROR] " << bazelignore_path <<  " does not exist" << std::endl;
			tool_exit(exit_code::filesystem_error);
		}
	}

	bazelignore_parse_results results;
	std::ifstream bazelignore_file(bazelignore_path);

	{
		std::string line;
		while(std::getline(bazelignore_file, line)) {
			trim_ws(line);
			if(!line.empty()) {
				results.ignore_paths.push_back(workspace_dir / line);
			}
		}
	}

	return results;
}

std::string python_like_slice(std::string value, const std::string& slice) {
	auto colon_count = std::count_if(
		slice.begin(),
		slice.end(),
		[](char c) { return c == ':'; }
	);

	if(colon_count > 1 || slice[0] != ':') {
		throw std::invalid_argument(
			"bzlws slice syntax is very limited right now. only [:start] is allows"
		);
	}

	auto offset = std::stoi(slice.substr(1));
	if(offset < 0) {
		value = value.substr(0, value.length() + offset);
	} else {
		value = value.substr(offset);
	}

	return value;
}

void bzlws_tool_lib::substr_str
	( std::string&        str
	, const char*         subst_id
	, const std::string&  subst_value
	)
{
	const auto subst_id_len = strlen(subst_id);
	auto substr_id_idx = str.find(subst_id);
	while(substr_id_idx != std::string::npos) {
		auto slice_start_idx = substr_id_idx + subst_id_len;
		if(str.size() > slice_start_idx && str[slice_start_idx] == '[') {
			auto slice_end_idx = str.find(']', slice_start_idx);
			if(slice_end_idx == std::string::npos) {
				throw std::invalid_argument("Expected ']' after '[' in slice");
			}

			auto slice = str.substr(
				slice_start_idx + 1,
				slice_end_idx - slice_start_idx - 1
			);
			auto sliced_subst_value = python_like_slice(subst_value, slice);
			
			str.replace(slice_start_idx, slice_end_idx - slice_start_idx + 1, "");
			str.replace(substr_id_idx, subst_id_len, sliced_subst_value);
		} else {
			str.replace(substr_id_idx, subst_id_len, subst_value);
		}

		substr_id_idx = str.find(subst_id);
	}
}

fs::path bzlws_tool_lib::get_src_out_path
	( const fs::path&  workspace_dir
	, std::string      out_dir_input
	, std::string      owner_label_str
	, fs::path         src_path
	, std::string      bzl_file_path
	, bool             force
	, std::string      strip_filepath_prefix
	, const std::vector<std::pair<std::string, std::string>>& remap_paths
	, std::string      target_os
	, std::string      target_cpu
	)
{
	auto label = parse_label_string(owner_label_str);
	label.workspace_name = get_apparent_repo_name(label.workspace_name);

	auto ext_str = src_path.extension().string();
	auto extname = ext_str.empty() ? "" : ext_str.substr(1);
	auto filepath = bzl_file_path;

	// NOTE: this is kind of a hack, I'm not sure if there's a better way to detect this.
	// Strip bazel-out/<config>/<tree>/ prefix from generated file paths so that
	// {FILEPATH} is workspace-relative rather than including the output tree.
	if (filepath.compare(0, 10, "bazel-out/") == 0) {
		auto second_slash = filepath.find('/', 10);
		if (second_slash != std::string::npos) {
			auto third_slash = filepath.find('/', second_slash + 1);
			if (third_slash != std::string::npos) {
				filepath = filepath.substr(third_slash + 1);
			}
		}
	}

	if (filepath.compare(0, 9, "external/") == 0) {
		auto slash_idx = filepath.find('/', 9);
		if (slash_idx != std::string::npos) {
			auto canonical_repo = filepath.substr(9, slash_idx - 9);
			auto apparent_repo = get_apparent_repo_name(canonical_repo);
			filepath = "external/" + apparent_repo + filepath.substr(slash_idx);
		}
	}

	substr_str(out_dir_input, "{TARGET_OS}", target_os);
	substr_str(out_dir_input, "{TARGET_CPU}", target_cpu);
	if(!strip_filepath_prefix.empty()) {
		substr_str(strip_filepath_prefix, "{TARGET_OS}", target_os);
		substr_str(strip_filepath_prefix, "{TARGET_CPU}", target_cpu);
	}

	if(!strip_filepath_prefix.empty()) {
		substr_str(strip_filepath_prefix, "{BAZEL_LABEL_NAME}", label.name);
		substr_str(strip_filepath_prefix, "{BAZEL_LABEL_PACKAGE}", label.package);
		substr_str(strip_filepath_prefix, "{BAZEL_LABEL_WORKSPACE_NAME}",
			label.workspace_name);
		substr_str(strip_filepath_prefix, "{BAZEL_FULL_LABEL}",
			label.workspace_name + "/" + label.package + "/" + label.name);
		substr_str(strip_filepath_prefix, "{BAZEL_LABEL}", label.package + "/" + label.name);

		if(filepath.rfind(strip_filepath_prefix, 0) == 0) {
			filepath = filepath.substr(strip_filepath_prefix.length());
		}
	}

	std::string longest_matched_prefix = "";
	std::string replacement_for_longest_match = "";
	bool found_match = false;

	for(const auto& remap : remap_paths) {
		std::string prefix = remap.first;
		std::string replacement = remap.second;

		substr_str(prefix, "\\", "/");
		substr_str(replacement, "\\", "/");

		substr_str(prefix, "{TARGET_OS}", target_os);
		substr_str(prefix, "{TARGET_CPU}", target_cpu);
		substr_str(prefix, "{BAZEL_LABEL_NAME}", label.name);
		substr_str(prefix, "{BAZEL_LABEL_PACKAGE}", label.package);
		substr_str(prefix, "{BAZEL_LABEL_WORKSPACE_NAME}", label.workspace_name);
		substr_str(prefix, "{BAZEL_FULL_LABEL}", label.workspace_name + "/" + label.package + "/" + label.name);
		substr_str(prefix, "{BAZEL_LABEL}", label.package + "/" + label.name);

		substr_str(replacement, "{TARGET_OS}", target_os);
		substr_str(replacement, "{TARGET_CPU}", target_cpu);
		substr_str(replacement, "{BAZEL_LABEL_NAME}", label.name);
		substr_str(replacement, "{BAZEL_LABEL_PACKAGE}", label.package);
		substr_str(replacement, "{BAZEL_LABEL_WORKSPACE_NAME}", label.workspace_name);
		substr_str(replacement, "{BAZEL_FULL_LABEL}", label.workspace_name + "/" + label.package + "/" + label.name);
		substr_str(replacement, "{BAZEL_LABEL}", label.package + "/" + label.name);

		if(filepath.rfind(prefix, 0) == 0) {
			if (!found_match || prefix.length() > longest_matched_prefix.length()) {
				found_match = true;
				longest_matched_prefix = prefix;
				replacement_for_longest_match = replacement;
			}
		}
	}

	if(found_match) {
		filepath = replacement_for_longest_match + filepath.substr(longest_matched_prefix.length());
	}

	substr_str(out_dir_input, "{BAZEL_LABEL_NAME}", label.name);
	substr_str(out_dir_input, "{BAZEL_LABEL_PACKAGE}", label.package);
	substr_str(out_dir_input, "{BAZEL_LABEL_WORKSPACE_NAME}",
		label.workspace_name);
	substr_str(out_dir_input, "{BAZEL_FULL_LABEL}",
		label.workspace_name + "/" + label.package + "/" + label.name);
	substr_str(out_dir_input, "{BAZEL_LABEL}", label.package + "/" + label.name);

	substr_str(out_dir_input, "{EXT}", ext_str);
	substr_str(out_dir_input, "{EXTNAME}", extname);
	substr_str(out_dir_input, "{FILENAME}", src_path.filename().string());
	substr_str(out_dir_input, "{FILEPATH}", filepath);

	substr_str(out_dir_input, "{BASENAME}",
		src_path.filename().replace_extension().string());

	fs::path out_path = workspace_dir / out_dir_input;
	fs::path out_dir = fs::path(out_path).remove_filename();

	if(!fs::exists(out_dir)) {
		if(!force) {
			std::cerr
				<< "Out path directory does not exist: "
				<< out_dir.generic_string() << std::endl;
			tool_exit(exit_code::filesystem_no_force_error);
		}

		fs::create_directories(out_dir);
	}

	if(!fs::is_directory(out_dir)) {
		std::cerr << "Out path is not a directory " << out_dir << std::endl;
		tool_exit(exit_code::existing_output_error);
	}

	return out_path;
}

template<typename NextArgFnT>
static void parse_arg
	( const std::string&                      arg
	, const fs::path&                         workspace_dir
	, bzlws_tool_lib::options&                options
	, NextArgFnT                              next_arg
	, bazel::tools::cpp::runfiles::Runfiles&  runfiles
	)
{
	if(arg == "--force") {
		options.force = true;
		return;
	}

	if(arg == "--metafile_out") {
		options.metafile_path = next_arg();
		return;
	}

	if(arg == "--strip_filepath_prefix") {
		options.strip_filepath_prefix = next_arg();
		bzlws_tool_lib::substr_str(options.strip_filepath_prefix, "\\", "/");
		return;
	}

	if(arg == "--remap_path") {
		auto prefix = next_arg();
		auto replacement = next_arg();
		options.remap_paths.push_back({prefix, replacement});
		return;
	}

	if(arg == "--bazel_info_subst") {
		options.bazel_info_subst_keys[next_arg()].push_back(next_arg());
		return;
	}

	if(arg == "--subst") {
		options.subst_values[next_arg()].push_back(next_arg());
		return;
	}

	if(arg == "--output") {
		options.output_path = next_arg();
		return;
	}

	if(arg == "--platform_manifest") {
		options.platform_manifest_path = next_arg();
		std::string manifest_abs_path = options.platform_manifest_path;
		if(!fs::exists(manifest_abs_path)) {
			manifest_abs_path = runfiles.Rlocation(options.platform_manifest_path);
		}
		if(manifest_abs_path.empty() || !fs::exists(manifest_abs_path)) {
			manifest_abs_path = (workspace_dir / options.platform_manifest_path).generic_string();
		}
		if(fs::exists(manifest_abs_path)) {
			std::ifstream infile(manifest_abs_path);
			std::string line;
			while(std::getline(infile, line)) {
				if(line.empty()) continue;
				std::istringstream iss(line);
				std::string path, os, cpu;
				if(iss >> path >> os >> cpu) {
					options.platform_manifest[path] = {os, cpu};
				}
			}
		} else {
			std::cerr << "[WARNING] Platform manifest not found: " << options.platform_manifest_path << std::endl;
		}
		return;
	}

	if(arg == "--default_target_os") {
		options.default_target_os = next_arg();
		return;
	}

	if(arg == "--default_target_cpu") {
		options.default_target_cpu = next_arg();
		return;
	}

	auto target_str = arg;
	const std::string potential_src_path = next_arg();
	std::string src_path = potential_src_path;

	if(!fs::exists(src_path)) {
		// If the file doesn't exist from our current directory check the runfiles
		src_path = runfiles.Rlocation(potential_src_path);
		if (src_path.empty() || !fs::exists(src_path)) {
			if (potential_src_path.compare(0, 9, "external/") == 0) {
				src_path = runfiles.Rlocation(potential_src_path.substr(9));
			}
		}
	}

	if(src_path.empty() || !fs::exists(src_path)) {
		src_path = (workspace_dir / potential_src_path).generic_string();
	}

	if(src_path.empty() || !fs::exists(src_path)) {
		std::cerr
			<< "Source path does not exist: "
			<< potential_src_path << std::endl;
		tool_exit(bzlws_tool_lib::exit_code::source_path_does_not_exist);
	}

	if(options.output_path.empty()) {
		std::cerr
			<< "[ERROR] --output flag must come before any non-flag arguments"
			<< std::endl;
		tool_exit(bzlws_tool_lib::exit_code::invalid_arguments);
	}

	if(fs::is_directory(src_path)) {

		for(auto other_src_path : fs::recursive_directory_iterator(src_path)) {
			if(fs::is_directory(other_src_path)) {
				continue;
			}

			std::string target_os = options.default_target_os;
			std::string target_cpu = options.default_target_cpu;
			auto path_str = other_src_path.path().generic_string();
			auto it = options.platform_manifest.find(path_str);
			if (it == options.platform_manifest.end()) {
				it = options.platform_manifest.find(potential_src_path);
			}
			if (it != options.platform_manifest.end()) {
				target_os = it->second.first;
				target_cpu = it->second.second;
			}

			auto rel_path = fs::relative(other_src_path.path(), src_path);
			auto bzl_file_path = (fs::path(potential_src_path) / rel_path).generic_string();

			auto src_out_path = bzlws_tool_lib::get_src_out_path(
				workspace_dir,
				options.output_path,
				target_str,
				other_src_path.path(),
				bzl_file_path,
				options.force,
				options.strip_filepath_prefix,
				options.remap_paths,
				target_os,
				target_cpu
			);

			options.srcs_info.push_back({other_src_path.path(), src_out_path});
		}

	} else {
		std::string target_os = options.default_target_os;
		std::string target_cpu = options.default_target_cpu;
		auto it = options.platform_manifest.find(potential_src_path);
		if (it != options.platform_manifest.end()) {
			target_os = it->second.first;
			target_cpu = it->second.second;
		}

		auto src_out_path = bzlws_tool_lib::get_src_out_path(
			workspace_dir,
			options.output_path,
			target_str,
			src_path,
			potential_src_path,
			options.force,
			options.strip_filepath_prefix,
			options.remap_paths,
			target_os,
			target_cpu
		);

		options.srcs_info.push_back({src_path, src_out_path});
	}
}

bzlws_tool_lib::options bzlws_tool_lib::parse_args
	( const fs::path&                  workspace_dir
	, const char*                      arv0
	, const std::vector<std::string>&  args
	)
{
	using bazel::tools::cpp::runfiles::Runfiles;

	std::string cmd;
	std::string error;
	std::unique_ptr<Runfiles> runfiles(Runfiles::Create(arv0, BAZEL_CURRENT_REPOSITORY,  &error));
	if(!error.empty()) {
		std::cerr
			<< "[[RUNFILES ERROR]]" << std::endl
			<< "  " << error << std::endl;
		std::exit(1);
	}

	bzlws_tool_lib::options options;

	for(int i=0; args.size() > i; ++i) {
		auto next_arg = [&] {
			if(i+1 > args.size()-1) {
				std::cerr
					<< "Argv access out of range at index " << i
					<< " (improperly formatted) " << std::endl;
				tool_exit(exit_code::invalid_arguments);
			}

			return std::string(args[++i]);
		};

		auto arg = std::string(args[i]);

		if(arg == "--params_file") {
			auto param_file = next_arg();
			if(!fs::exists(workspace_dir / param_file)) {
				std::cerr
					<< "[ERROR] Unable to read params file: " << param_file << std::endl;
				tool_exit(exit_code::filesystem_error);
			}

			std::ifstream param_file_stream(workspace_dir / param_file);

			auto get_next_line = [&]{
				std::string next_line;
				if(!std::getline(param_file_stream, next_line, '\n')) {
					std::cerr
						<< "Argv access out of range at index " << i
						<< " (improperly formatted) " << std::endl;
					tool_exit(exit_code::invalid_arguments);
				}
				if(next_line.starts_with('\'') && next_line.ends_with('\'')) {
					next_line = next_line.substr(1, next_line.size() - 2);
				}

				return next_line;
			};

			std::string line;
			while(std::getline(param_file_stream, line, '\n')) {
				parse_arg(line, workspace_dir, options, get_next_line, *runfiles);
			}

		} else {
			parse_arg(arg, workspace_dir, options, next_arg, *runfiles);
		}
	}

	if(!options.bazel_info_subst_keys.empty()) {
		std::vector<std::string> subst_keys;
		for(const auto& entry : options.bazel_info_subst_keys) {
			subst_keys.push_back(entry.first);
		}

		auto curr_path = fs::current_path();
		fs::current_path(workspace_dir);
		auto subst_values = get_bazel_info(subst_keys);
		fs::current_path(curr_path);

		for(size_t i=0; subst_keys.size() > i; ++i) {
			const auto& bazel_info_key = subst_keys[i];
			const auto& value = subst_values[i];
			for(auto subst_key : options.bazel_info_subst_keys[bazel_info_key]) {
				options.subst_values[subst_key].push_back(value);
			}
		}
	}

	return options;
}

bzlws_tool_lib::options bzlws_tool_lib::parse_argv
	( const fs::path&  workspace_dir
	, int              argc
	, char**           argv
	)
{
	std::vector<std::string> args;
	for(int i=1; argc > i; ++i) {
		args.emplace_back(argv[i]);
	}
	return parse_args(workspace_dir, argv[0], args);
}

void bzlws_tool_lib::copy_with_substitutions
	( const options& options
	, const src_info& src_info
	)
{
	std::ifstream in_stream(src_info.src_path);
	std::ofstream out_stream(src_info.new_src_path);
	std::string line;

	while(!in_stream.eof()) {
		std::getline(in_stream, line, '\n');
		for(const auto& entry : options.subst_values) {
			const auto& key = entry.first;

			for(const auto& value : entry.second) {
				auto key_idx = line.find(key);
				while(key_idx != std::string::npos) {
					line.replace(key_idx, key.size(), value);
					key_idx = line.find(key);
				}
			}
		}

		out_stream << line;

		if(!in_stream.eof() && !in_stream.fail()) {
			out_stream << '\n';
		}
	}
}

void bzlws_tool_lib::remove_previous_generated_files
	( const fs::path&    workspace_dir
	, const std::string  metafile_path
	)
{
	auto full_metafile_path = workspace_dir / metafile_path;
	if(!fs::exists(full_metafile_path)) {
		return;
	}

	auto metafile = YAML::LoadFile(full_metafile_path.string());

	for(const auto& field : metafile) {
		const auto key = field.first.as<std::string>();

		if(key == "files") {
			const auto file_paths = field.second.as<std::vector<std::string>>();

			auto remove_count = 0;
			std::vector<fs::path> dirs_to_check;
			for(auto file_path : file_paths) {
				std::error_code ec;
				force_remove(file_path, ec);
				if(ec) {
					std::cerr
						<< "[WARN] Failed to remove '" << file_path << "'" 
						<< std::endl;
				} else {
					remove_count += 1;
					fs::path parent_dir = fs::path(file_path).parent_path();
					if (parent_dir != workspace_dir) {
						dirs_to_check.push_back(parent_dir);
					}
				}
			}

			for(auto dir : dirs_to_check) {
				while(dir != workspace_dir) {
					std::error_code ec;
					if(!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
						break;
					}
					std::error_code rm_ec;
					fs::remove(dir, rm_ec);
					if(rm_ec) {
						break;
					}
					dir = dir.parent_path();
				}
			}

			if(remove_count > 0) {
				std::cout
					<< "Successfully removed " << remove_count << "/" << file_paths.size()
					<< " existing files" << std::endl;
			}
		}
	}
}

void bzlws_tool_lib::write_generated_metadata_file
	( const fs::path&               workspace_dir
	, const std::string             metafile_path
	, const std::vector<src_info>&  srcs_info
	)
{
	auto full_metafile_path = workspace_dir / metafile_path;
	YAML::Node metafile;

	for(auto src : srcs_info) {
		metafile["files"].push_back(src.new_src_path.generic_string());
	}

	std::ofstream output(full_metafile_path);

	output << metafile;

	output.close();
}

std::vector<std::string> bzlws_tool_lib::get_bazel_info
	( const std::vector<std::string>& info_keys
	)
{	
	if(info_keys.empty()) {
		return {};
	}

	std::string cmd = "bazel info";
	std::vector<std::string> key_values;
	std::stringstream result;

	for(const auto& key : info_keys) {
		cmd += " " + key;
	}

	{
		std::array<char, 512> stdout_buf;
		std::unique_ptr<FILE, decltype(&pclose)> pipe(
			popen(cmd.c_str(), "r"),
			pclose
		);

		if(!pipe) {
			throw std::runtime_error("get_bazel_info popen() failed!");
		}

		while (fgets(stdout_buf.data(), stdout_buf.size(), pipe.get()) != nullptr) {
			result << stdout_buf.data();
		}
	}

	result.seekp(0);

	std::array<char, 128> key_value_buf;
	while(!result.eof()) {
		if(info_keys.size() > 1) {
			result.ignore(512, ':');
		}

	result.getline(key_value_buf.data(), key_value_buf.size(), '\n');
		auto value_length = strlen(key_value_buf.data());

		auto& key_value = key_values.emplace_back(
			key_value_buf.data(),
			value_length
		);

		trim_ws(key_value);
	}

	return key_values;
}

bool bzlws_tool_lib::files_are_identical(const fs::path& p1, const fs::path& p2) {
	std::error_code ec;
	if (fs::equivalent(p1, p2, ec)) return true;
	if (!fs::exists(p1) || !fs::exists(p2)) return false;
	if (fs::file_size(p1, ec) != fs::file_size(p2, ec)) return false;
	
	std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
	std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);
	if (f1.fail() || f2.fail()) return false;
	if (f1.tellg() != f2.tellg()) return false;
	
	f1.seekg(0, std::ifstream::beg);
	f2.seekg(0, std::ifstream::beg);
	return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
					  std::istreambuf_iterator<char>(),
					  std::istreambuf_iterator<char>(f2.rdbuf()));
}

fs::path bzlws_tool_lib::get_relative_path(const fs::path& path, const fs::path& base) {
#if _WIN32
	std::string path_str = path.generic_string();
	std::string base_str = base.generic_string();
	if (path_str.size() >= base_str.size()) {
		std::string path_prefix = path_str.substr(0, base_str.size());
		bool match = true;
		for (size_t i = 0; i < base_str.size(); ++i) {
			if (std::tolower((unsigned char)path_prefix[i]) != std::tolower((unsigned char)base_str[i])) {
				match = false;
				break;
			}
		}
		if (match) {
			auto rel = path_str.substr(base_str.size());
			if (!rel.empty() && rel[0] == '/') rel = rel.substr(1);
			if (rel.empty()) return ".";
			return rel;
		}
	}
#endif
	return fs::relative(path, base);
}
