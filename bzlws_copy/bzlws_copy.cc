#include "bzlws_tool_lib/bzlws_tool_lib.hh"

int main(int argc, char* argv[]) {
	using namespace bzlws_tool_lib;

	auto workspace_dir = get_build_workspace_dir();
	auto bzlignore = parse_bazelignore(workspace_dir);
	auto srcs_info = get_srcs_info(workspace_dir, argc, argv);
	
	auto wsDirSz = workspace_dir.generic_string().size();

	for(const auto& info : srcs_info) {
		bzlignore.assert_ignored_path(info.new_src_path);
	}

	for(const auto& info : srcs_info) {
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
			<< src_path.generic_string().substr(wsDirSz + 1) << " -> "
			<< fs::relative(new_src_path.string(), workspace_dir).generic_string()
			<< std::endl;

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
