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

	for(const auto& info : options.srcs_info) {
		const auto& src_path = info.src_path;
		const auto& new_src_path = info.new_src_path;
		std::error_code ec;

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

		std::cout
			<< src_path.generic_string() << " <-> "
			<< fs::relative(new_src_path.string(), workspace_dir).generic_string()
			<< std::endl;

		fs::create_symlink(src_path, new_src_path, ec);
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
