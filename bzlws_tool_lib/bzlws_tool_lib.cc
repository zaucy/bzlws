#include "bzlws_tool_lib.hh"

namespace fs = bzlws_tool_lib::fs;
using bzlws_tool_lib::bazelignore_parse_results;
using bzlws_tool_lib::src_info;

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
	std::exit(1);
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
		std::exit(1);
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
			exit(1);
		}

		if(!bazelignore_exists) {
			std::cerr
				<< "[ERROR] " << bazelignore_path <<  " does not exist" << std::endl;
			exit(1);
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
	, fs::path         src_path
	)
{
	auto out_dir_input = std::string(argv[argc - 1]);
	auto ext_str = src_path.extension().string();
	auto extname = ext_str.empty() ? "" : ext_str.substr(1);

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
		std::cerr << "Out path directory does not exist: " << out_dir << std::endl;
		std::exit(1);
	}

	if(!fs::is_directory(out_dir)) {
		std::cerr << "Out path is not a directory " << out_dir << std::endl;
		std::exit(1);
	}

	return out_path;
}

std::vector<src_info> bzlws_tool_lib::get_srcs_info
	( const fs::path&  workspace_dir
	, int              argc
	, char**           argv
	)
{
	std::vector<src_info> srcs_info;

	for(int i=1; argc-1 > i; ++i) {
		auto arg = std::string(argv[i]);
		auto src_path = workspace_dir / arg;

		if(!fs::exists(src_path)) {
			std::cerr << "Source path does not exist: " << src_path << std::endl;
			std::exit(1);
		}

		auto src_out_path = get_src_out_path(workspace_dir, argc, argv, src_path);

		srcs_info.push_back({src_path, src_out_path});
	}

	return srcs_info;
}
