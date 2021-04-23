#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <locale>
#include <map>

namespace bzlws_tool_lib {
	namespace fs = std::filesystem;

	enum class exit_code : int {
		unknown = 1,
		bazelignore_error = 2,
		missing_environment_variable = 3,
		source_path_does_not_exist = 4,
		filesystem_error = 5,
		filesystem_no_force_error = 6,
		existing_output_error = 7,
		invalid_arguments = 8,
	};

	struct bazelignore_parse_results {
		std::vector<fs::path> ignore_paths;

		bool is_ignored_path
			( const fs::path& path
			);

		void assert_ignored_path
			( const fs::path& path
			);
	};

	struct src_info {
		fs::path src_path;
		fs::path new_src_path;
	};

	struct options {
		std::vector<src_info> srcs_info;
		std::map<std::string, std::vector<std::string>> bazel_info_subst_keys;
		std::map<std::string, std::vector<std::string>> subst_values;
		std::string metafile_path;
		std::string strip_filepath_prefix;
		std::string output_path;
		bool force = false;
	};

	options parse_argv
		( const fs::path&  workspace_dir
		, int              argc
		, char**           argv
		);
	options parse_args
		( const fs::path&                  workspace_dir
		, const char*                      arv0
		, const std::vector<std::string>&  args
		);

	std::vector<std::string> get_bazel_info
		( const std::vector<std::string>& info_keys
		);

	fs::path get_build_workspace_dir();

	bazelignore_parse_results parse_bazelignore
		( const fs::path& workspace_dir
		);

	void copy_with_substitutions
		( const options& options
		, const src_info& src_info
		);

	void substr_str
		( std::string&        str
		, const char*         subst_id
		, const std::string&  subst_value
		);

	fs::path get_src_out_path
		( const fs::path&  workspace_dir
		, std::string      out_dir_input
		, std::string      owner_label_str
		, fs::path         src_path
		, bool             force
		, std::string      strip_filepath_prefix
		);

	void remove_previous_generated_files
		( const fs::path&               workspace_dir
		, const std::string             metafile_path
		);

	void write_generated_metadata_file
		( const fs::path&               workspace_dir
		, const std::string             metafile_path
		, const std::vector<src_info>&  srcs_info
		);

	bool force_remove
		( const fs::path& path
		, std::error_code& ec
		) noexcept;

	void trim_ws
		( std::string& str
		);
}
