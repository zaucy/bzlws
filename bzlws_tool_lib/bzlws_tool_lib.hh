#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <locale>

namespace bzlws_tool_lib {
	namespace fs = std::filesystem;

	bool force_remove
		( const fs::path& path
		, std::error_code& ec
		) noexcept;

	void trim_ws
		( std::string& str
		);

	fs::path get_build_workspace_dir();

	struct bazelignore_parse_results {
		std::vector<fs::path> ignore_paths;

		bool is_ignored_path
			( const fs::path& path
			);

		void assert_ignored_path
			( const fs::path& path
			);
	};

	bazelignore_parse_results parse_bazelignore
		( const fs::path& workspace_dir
		);

	void substr_str
		( std::string&        str
		, const char*         subst_id
		, const std::string&  subst_value
		);

	struct src_info {
		fs::path src_path;
		fs::path new_src_path;
	};

	fs::path get_src_out_path
		( const fs::path&  workspace_dir
		, int              argc
		, char**           argv
		, std::string      owner_label_str
		, fs::path         src_path
		, bool             force
		);

	std::vector<src_info> get_srcs_info
		( const fs::path&  workspace_dir
		, bool&            out_force
		, int              argc
		, char**           argv
		);

}
