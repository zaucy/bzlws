#include "bzlws_tool_lib.hh"

#include <fstream>
#include <array>
#include <cstdio>
#include <memory>
#include <sstream>
#include <cstring>
#include <string>
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
}


bool bzlws_tool_lib::force_remove
	( const fs::path& path
	, std::error_code& ec
	) noexcept
{
	fs::permissions(path, fs::perms::others_write, ec);
	if(ec) return false;

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

void bzlws_tool_lib::substr_str
	( std::string&        str
	, const char*         subst_id
	, const std::string&  subst_value
	)
{
	const auto subst_id_len = strlen(subst_id);
	auto substr_id_idx = str.find(subst_id);
	while(substr_id_idx != std::string::npos) {
		str.replace(substr_id_idx, subst_id_len, subst_value);
		substr_id_idx = str.find(subst_id);
	}
}

fs::path bzlws_tool_lib::get_src_out_path
	( const fs::path&  workspace_dir
	, std::string      out_dir_input
	, std::string      owner_label_str
	, fs::path         src_path
	, bool             force
	, std::string      strip_filepath_prefix
	)
{
	auto label = parse_label_string(owner_label_str);

	auto ext_str = src_path.extension().string();
	auto extname = ext_str.empty() ? "" : ext_str.substr(1);
	auto filepath = fs::relative(src_path).generic_string();
	while(filepath.find("../") != std::string::npos) {
		substr_str(filepath, "../", "");
	}

	if(!strip_filepath_prefix.empty()) {
		if(filepath.rfind(strip_filepath_prefix, 0) == 0) {
			filepath = filepath.substr(strip_filepath_prefix.length());
		}
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
	if(fs::is_directory(out_path)) {
		out_path = out_path / src_path.filename();
	}

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

	auto target_str = arg;
	const std::string potential_src_path = next_arg();
	std::string src_path = potential_src_path;

	if(!fs::exists(src_path)) {
		// If the file doesn't exist from our current directory check the runfiles
		src_path = runfiles.Rlocation(potential_src_path);
	}

	if(src_path.empty() || !fs::exists(src_path)) {
		// If we still cannot find the file try from the workspace directory
		src_path = (workspace_dir / potential_src_path).make_preferred().string();
	}

	if(src_path.empty() || !fs::exists(src_path)) {
		std::cerr
			<< "Source path does not exist: "
			<< potential_src_path << std::endl;
		tool_exit(bzlws_tool_lib::exit_code::source_path_does_not_exist);
	}

	if(options.output_path.empty()) {
		std::cerr
			<< "[ERROR] --output flag must come before any non-flag arugments"
			<< std::endl;
		tool_exit(bzlws_tool_lib::exit_code::invalid_arguments);
	}

	if(fs::is_directory(src_path)) {

		for(auto other_src_path : fs::recursive_directory_iterator(src_path)) {
			if(fs::is_directory(other_src_path)) {
				continue;
			}

			auto src_out_path = bzlws_tool_lib::get_src_out_path(
				workspace_dir,
				options.output_path,
				target_str,
				other_src_path,
				options.force,
				options.strip_filepath_prefix
			);

			options.srcs_info.push_back({other_src_path, src_out_path});
		}

	} else {
		auto src_out_path = bzlws_tool_lib::get_src_out_path(
			workspace_dir,
			options.output_path,
			target_str,
			src_path,
			options.force,
			options.strip_filepath_prefix
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
	std::unique_ptr<Runfiles> runfiles(Runfiles::Create(arv0, &error));
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
					<< " (improperly formated) " << std::endl;
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
						<< " (improperly formated) " << std::endl;
					tool_exit(exit_code::invalid_arguments);
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
			for(auto file_path : file_paths) {
				std::error_code ec;
				force_remove(file_path, ec);
				if(ec) {
					std::cerr
						<< "[WARN] Failed to remove '" << file_path << "'" 
						<< std::endl;
				} else {
					remove_count += 1;
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
