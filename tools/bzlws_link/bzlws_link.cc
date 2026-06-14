#include "tools/bzlws_tool_lib/bzlws_tool_lib.hh"

int bzlws_link
	( const char*                      argv0
	, const std::vector<std::string>&  args
	)
{
	using namespace bzlws_tool_lib;

	auto workspace_dir = get_build_workspace_dir();
	auto bzlignore = parse_bazelignore(workspace_dir);
	auto options = parse_args(workspace_dir, argv0, args);
	
	for(const auto& info : options.srcs_info) {
		bzlignore.assert_ignored_path(info.new_src_path);
	}

	auto wsDirSz = workspace_dir.generic_string().size();

	if(!options.metafile_path.empty()) {
		bzlws_tool_lib::remove_previous_generated_files(
			workspace_dir,
			options.metafile_path
		);
	}

	std::map<fs::path, fs::path> written_paths;

	for(const auto& info : options.srcs_info) {
		const auto& src_path = info.src_path;
		const auto& new_src_path = info.new_src_path;
		std::error_code ec;

		auto it = written_paths.find(new_src_path);
		if (it != written_paths.end()) {
			if (!files_are_identical(it->second, src_path)) {
				std::cerr << "[ERROR] Conflict: Trying to link different files to the same output location: " << new_src_path.generic_string() << std::endl;
				std::exit(1);
			}
			continue;
		}
		written_paths[new_src_path] = src_path;

		if(fs::exists(new_src_path)) {
			auto existing_is_symlink = fs::is_symlink(new_src_path, ec);
			if(ec) {
				std::cerr
					<< "[ERROR] Failed to check existing symlink \""
					<< new_src_path.generic_string() << "\" - " << ec.message()
					<< std::endl;
				std::exit(1);
			}

			if(!existing_is_symlink && !options.force) {
				std::cerr
					<< "[ERROR] Cannot create symlink since \""
					<< new_src_path.generic_string() << "\" exists and is a non-symlink. "
					<< "Use --force to override this behaviour."
					<< std::endl;
				std::exit(1);
			}

			force_remove(new_src_path, ec);
			if(ec) {
				std::cerr
					<< "Unable to remove \"" << new_src_path.generic_string() << "\" - "
					<< ec.message() << std::endl;
				std::exit(1);
			}
		}

		auto out_dir = new_src_path.parent_path();
		if(!fs::exists(out_dir)) {
			fs::create_directories(out_dir);
		}

		std::cout
			<< get_relative_path(src_path, workspace_dir).generic_string() << " <-> "
			<< get_relative_path(new_src_path, workspace_dir).generic_string()
			<< std::endl;

		fs::path absolute_src_path = src_path;
		if (!absolute_src_path.is_absolute()) {
			absolute_src_path = workspace_dir / absolute_src_path;
		}

		fs::create_symlink(absolute_src_path, new_src_path, ec);
		if(ec) {
			std::cerr
				<< "[ERROR] Failed to create symlink "
				<< src_path.generic_string() << " <-> "
				<< new_src_path.generic_string() << std::endl << ec.message()
				<< std::endl;
			std::exit(1);
		}
	}

	if(!options.metafile_path.empty()) {
		bzlws_tool_lib::write_generated_metadata_file(
			workspace_dir,
			options.metafile_path,
			options.srcs_info
		);
	}

	return 0;
}
