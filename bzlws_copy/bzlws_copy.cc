#include <cstdlib>
#include <string>
#include <iostream>
#include "bzlws_tool_lib/bzlws_tool_lib.hh"

static void copy_files
	( const std::filesystem::path&    workspace_dir
	, const bzlws_tool_lib::options&  options
	);

static void copy_file
	( const bzlws_tool_lib::options&   options
	, const bzlws_tool_lib::src_info&  src_info
	);

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

	copy_files(workspace_dir, options);

	if(!options.metafile_path.empty()) {
		bzlws_tool_lib::write_generated_metadata_file(
			workspace_dir,
			options.metafile_path,
			options.srcs_info
		);
	}

	auto ibazel = std::string(std::getenv("IBAZEL_NOTIFY_CHANGES"));
	if(ibazel == "y") {
		std::string notice;
		do {
			std::getline(std::cin, notice);
			if(notice == "IBAZEL_BUILD_COMPLETED SUCCESS") {
				copy_files(workspace_dir, options);
			}
		} while(!notice.empty());
	}

	return 0;
}

void copy_files
	( const std::filesystem::path&    workspace_dir
	, const bzlws_tool_lib::options&  options
	)
{
	using namespace bzlws_tool_lib;

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

		if(options.subst_values.empty()) {
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
}

void copy_file
	( const bzlws_tool_lib::options&   options
	, const bzlws_tool_lib::src_info&  src_info
	)
{

}
