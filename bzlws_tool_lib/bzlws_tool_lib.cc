#include "bzlws_tool_lib.hh"

#include <fstream>
#include <array>
#include <cstdio>
#include <memory>
#include <sstream>
#include <yaml-cpp/yaml.h>

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
	, int              argc
	, char**           argv
	, std::string      owner_label_str
	, fs::path         src_path
	, bool             force
	)
{
	auto label = parse_label_string(owner_label_str);

	auto out_dir_input = std::string(argv[argc - 1]);
	auto ext_str = src_path.extension().string();
	auto extname = ext_str.empty() ? "" : ext_str.substr(1);

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
				<< "Out path directory does not exist: " << out_dir << std::endl;
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

bzlws_tool_lib::options bzlws_tool_lib::parse_argv
	( const fs::path&  workspace_dir
	, int              argc
	, char**           argv
	)
{
	bzlws_tool_lib::options options;

	for(int i=1; argc-1 > i; i++) {
		auto next_arg = [&] {
			if(i+1 > argc-1) {
				std::cerr
					<< "Argv access out of range at index " << i
					<< " (improperly formated) " << std::endl;
				tool_exit(exit_code::invalid_arguments);
			}

			return std::string(argv[++i]);
		};

		auto arg = std::string(argv[i]);
		if(arg == "--force") {
			options.force = true;
			continue;
		}

		if(arg == "--metafile_out") {
			options.metafile_path = next_arg();
			continue;
		}

		if(arg == "--subst") {
			options.substitution_keys[next_arg()].push_back(next_arg());
			continue;
		}

		auto target_str = arg;
		auto src_path = workspace_dir / next_arg();

		if(!fs::exists(src_path)) {
			std::cerr
				<< "Source path does not exist: "
				<< src_path.string() << std::endl;
			tool_exit(exit_code::source_path_does_not_exist);
		}

		auto src_out_path = get_src_out_path(
			workspace_dir,
			argc,
			argv,
			target_str,
			src_path,
			options.force
		);

		options.srcs_info.push_back({src_path, src_out_path});
	}

	if(!options.substitution_keys.empty()) {
		std::vector<std::string> subst_keys;
		for(const auto& entry : options.substitution_keys) {
			subst_keys.push_back(entry.first);
		}

		auto curr_path = fs::current_path();
		fs::current_path(workspace_dir);
		auto subst_values = get_bazel_info(subst_keys);
		fs::current_path(curr_path);

		for(size_t i=0; subst_keys.size() > i; ++i) {
			const auto& key = subst_keys[i];
			const auto& value = subst_values[i];
			options.substitution_values[key] = value;
		}
	}

	return options;
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
		for(const auto& entry : options.substitution_keys) {
			const auto& value = options.substitution_values.at(entry.first);
			if(value.empty()) continue;

			for(const auto& key : entry.second) {
				auto key_idx = line.find(key);
				while(key_idx != std::string::npos) {
					line.replace(key_idx, key.size(), value);
					key_idx = line.find(key);
				}
			}
		}

		out_stream << line << '\n';
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
