#include "bzlws_extract.hh"

#include <cstdlib>
#include <string>
#include <iostream>
#include <exception>
#include <filesystem>
#include "tools/bzlws_tool_lib/bzlws_tool_lib.hh"

namespace fs = std::filesystem;

static void extract_files
	( const std::filesystem::path&    workspace_dir
	, const bzlws_tool_lib::options&  options
	);

int bzlws_extract
	( const char*                      argv0
	, const std::vector<std::string>&  args
	)
{
	using namespace bzlws_tool_lib;

	std::set_terminate([] {
		std::exception_ptr eptr = std::current_exception();
		try {
			if(eptr) {
				std::rethrow_exception(eptr);
			}
		} catch(const std::exception& err) {
			std::cerr << "[FATAL] Unhandled exception: " << err.what() << std::endl;
		}

		std::abort();
	});

	auto workspace_dir = get_build_workspace_dir();
	auto bzlignore = parse_bazelignore(workspace_dir);
	auto options = parse_args(workspace_dir, argv0, args);
	
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

	extract_files(workspace_dir, options);

	if(!options.metafile_path.empty()) {
		bzlws_tool_lib::write_generated_metadata_file(
			workspace_dir,
			options.metafile_path,
			options.srcs_info
		);
	}

	auto ibazel = std::getenv("IBAZEL_NOTIFY_CHANGES");
	if(ibazel != nullptr && std::string(ibazel) == "y") {
		std::string notice;
		do {
			std::getline(std::cin, notice);
			if(notice == "IBAZEL_BUILD_COMPLETED SUCCESS") {
				extract_files(workspace_dir, options);
			}
		} while(!notice.empty());
	}

	std::cout << "Success!" << std::endl;
	return 0;
}

void extract_files
	( const std::filesystem::path&    workspace_dir
	, const bzlws_tool_lib::options&  options
	)
{
	for(auto& src : options.srcs_info) {
		if(!fs::exists(src.new_src_path)) continue;

		for(auto p : fs::recursive_directory_iterator(src.new_src_path)) {
			std::error_code ec;
			bzlws_tool_lib::force_remove(src.new_src_path, ec);
		}
	}

	for(auto& src : options.srcs_info) {
		std::string tar_args = "tar xf ";
		tar_args += "\"" + src.src_path.string() + "\" ";
		tar_args += "-C \"" + src.new_src_path.string() + "\" ";

		fs::create_directories(src.new_src_path);

		auto exit_code = std::system(tar_args.c_str());
		if(exit_code == 0) {
			std::cout
				<< src.src_path.string() << " -> "
				<< src.new_src_path.string() << "\n";
		} else {
			std::cerr << "tar command failed: exit code " << exit_code << "\n";
		}
	}
}
