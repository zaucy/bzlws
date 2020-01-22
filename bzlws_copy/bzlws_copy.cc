#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <locale>

namespace fs = std::filesystem;

bool force_remove(const fs::path& path, std::error_code& ec) noexcept {
	fs::permissions(path, fs::perms::others_write, ec);
	if(ec) return false;

	return fs::remove(path, ec);
}

void trim_ws(std::string& s) {
	using std::isspace;
	using std::find_if;

	auto not_isspace = [](int c) { return !isspace(c); };
	s.erase(s.begin(), find_if(s.begin(), s.end(), not_isspace));
	s.erase(find_if(s.rbegin(), s.rend(), not_isspace).base(), s.end());
}

fs::path get_build_workspace_dir() {
	char* bwd_env = std::getenv("BUILD_WORKSPACE_DIRECTORY");
	if(bwd_env != nullptr) {
		return fs::path(bwd_env).make_preferred();
	}

	std::cerr
		<< "[ERROR] Missing 'BUILD_WORKSPACE_DIRECTORY' environment variable"
		<< std::endl;
	std::exit(1);
}

struct bazelignore_parse_results {
	std::vector<fs::path> ignore_paths;

	inline bool is_ignored_path
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
};

bazelignore_parse_results parse_bazelignore
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

std::vector<fs::path> get_src_paths
	( const fs::path&  workspace_dir
	, int              argc
	, char**           argv
	)
{
	std::vector<fs::path> src_paths;

	for(int i=1; argc-1 > i; ++i) {
		auto arg = std::string(argv[i]);
		auto src_path = workspace_dir / arg;

		if(!fs::exists(src_path)) {
			std::cerr << "Source path does not exist: " << src_path << std::endl;
			std::exit(1);
		}

		src_paths.push_back(src_path);
	}

	return src_paths;
}

fs::path get_out_path
	( const fs::path&  workspace_dir
	, int              argc
	, char**           argv
	)
{
	fs::path out_path = workspace_dir / std::string(argv[argc - 1]);

	if(!fs::exists(out_path)) {
		std::cerr << "Out path does not exist: " << out_path << std::endl;
		std::exit(1);
	}

	if(!fs::is_directory(out_path)) {
		std::cerr << "Out path is not a directory " << out_path << std::endl;
		std::exit(1);
	}

	return out_path;
}

int main(int argc, char* argv[]) {

	auto workspace_dir = get_build_workspace_dir();
	auto bzlignore = parse_bazelignore(workspace_dir);
	auto src_paths = get_src_paths(workspace_dir, argc, argv);
	auto out_path = get_out_path(workspace_dir, argc, argv);

	if(!bzlignore.is_ignored_path(out_path)) {
		std::cerr
			<< "[ERROR] bzlws_copy out path must be within your .bazelignore \""
			<< out_path.string() << "\" does not reside in any of the following:"
			<< std::endl;

		for(auto ignore_path : bzlignore.ignore_paths) {
			std::cerr
				<< "  " << ignore_path.string() << std::endl;
		}

		std::exit(1);
	}
	
	auto wsDirSz = workspace_dir.generic_string().size();

	for(auto src_path : src_paths) {
		auto new_src_path = out_path / src_path.filename();
		std::error_code ec;

		std::cout
			<< src_path.generic_string().substr(wsDirSz + 1) << " -> "
			<< fs::relative(new_src_path.string(), workspace_dir).generic_string()
			<< std::endl;

		if(fs::exists(new_src_path)) {
			force_remove(new_src_path, ec);
			if(ec) {
				std::cerr
					<< "Unable to remove \"" << new_src_path.generic_string() << "\" - "
					<< ec.message() << std::endl;
				std::exit(1);
			}
		}

		fs::copy_file(src_path, new_src_path, ec);
		if(ec) {
			std::cerr
				<< "[ERROR] Failed to copy " << src_path.generic_string() << " -> "
				<< new_src_path.generic_string() << std::endl << ec.message()
				<< std::endl;
			std::exit(1);
		}
	}

	return 0;
}
