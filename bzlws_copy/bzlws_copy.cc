#include "bzlws_tool_lib/bzlws_tool_lib.hh"

int main(int argc, char* argv[]) {
	using namespace bzlws_tool_lib;

	auto workspace_dir = get_build_workspace_dir();
	auto bzlignore = parse_bazelignore(workspace_dir);
	auto options = parse_argv(workspace_dir, argc, argv);

	
	auto wsDirSz = workspace_dir.generic_string().size();

	for(const auto& info : options.srcs_info) {
		bzlignore.assert_ignored_path(info.new_src_path);
	}

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
			force_remove(new_src_path, ec);
			if(ec) {
				std::cerr
					<< "Unable to remove \"" << new_src_path.generic_string() << "\" - "
					<< ec.message() << std::endl;
				std::exit(1);
			}
		}

		std::cout
			<< src_path.generic_string() << " -> "
			<< fs::relative(new_src_path.string(), workspace_dir).generic_string()
			<< std::endl;

		if(options.substitution_keys.empty()) {
			fs::copy_file(src_path, new_src_path, ec);
			if(ec) {
				std::cerr
					<< "[ERROR] Failed to copy " << src_path.generic_string() << " -> "
					<< new_src_path.generic_string() << std::endl << ec.message()
					<< std::endl;
				std::exit(1);
			}
		} else {
			bzlws_tool_lib::copy_with_substitutions(options, info);
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
