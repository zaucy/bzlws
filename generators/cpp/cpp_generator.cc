#include <filesystem>
#include <string>
#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <set>

namespace fs = std::filesystem;

static const char SCRIPT_SRC_START[] = R"______(
#include <cstdlib>
#include <memory>
#include <string>
#include <iostream>
#include <filesystem>

#include "tools/cpp/runfiles/runfiles.h"
using bazel::tools::cpp::runfiles::Runfiles;

std::filesystem::path get_build_workspace_dir() {
	return std::filesystem::path(
		std::getenv("BUILD_WORKSPACE_DIRECTORY")
	).make_preferred();
}

int main(int argc, char* argv[]) {
	std::string cmd;
	std::string error;
	std::unique_ptr<Runfiles> runfiles(Runfiles::Create(argv[0], &error));
	if(!error.empty()) {
		std::cerr << error << std::endl;
		std::exit(1);
	}
)______";

static const char SCRIPT_SRC_END[] = R"______(

	return std::system(cmd.c_str());
} // end of main
)______";

std::string escaped_string
	( const std::string& str
	)
{
	return "R\"______(" + str + ")______\"";
}

template<typename Fn>
void for_each_status_item
  ( std::string  status_file_path
  , Fn&&         fn
  )
{
  std::string line;
  std::string::size_type ws_idx;

	std::ifstream status_file_stream(status_file_path);

  // Reading file based on:
  // https://docs.bazel.build/versions/master/user-manual.html#workspace_status
  while(status_file_stream.good()) {
    line.clear();
    std::getline(status_file_stream, line);
    if(line.empty()) continue;

    ws_idx = line.find_first_of(' ');

		fn(line.substr(0, ws_idx), line.substr(ws_idx + 1));
  }
}

static void substr_str
	( std::string&        str
	, const std::string&  subst_id
	, const std::string&  subst_value
	)
{
	const auto subst_id_len = subst_id.length();
	auto substr_id_idx = str.find(subst_id);
	while(substr_id_idx != std::string::npos) {
		str.replace(substr_id_idx, subst_id_len, subst_value);
		substr_id_idx = str.find(subst_id);
	}
}

int main(int argc, char* argv[]) {
	bool force = false;
	std::string generated_script_path;
	std::string tool;
	std::vector<std::string> forwarded_args;
	std::vector<std::array<std::string, 2>> paths;
	std::string out_path;

	std::map<std::string, std::string> stamp_subst_map;
	std::set<std::string> stamp_subst_used;
	std::string stable_status_file_path;
	std::string volatile_status_file_path;

	for(int i=1; argc-1 > i; ++i) {
		auto arg = std::string(argv[i]);

		if(arg == "--generated_script_path") {
			generated_script_path = std::string(argv[++i]);
		} else
		if(arg == "--output") {
			out_path = std::string(argv[++i]);
		} else
		if(arg == "--tool") {
			tool = std::string(argv[++i]);
		} else
		if(arg == "--stamp_subst") {
			auto key = argv[++i];
			auto value = argv[++i];
			stamp_subst_map[key] = value;
		} else
		if(arg == "--stable_status") {
			stable_status_file_path = argv[++i];
		} else
		if(arg == "--volatile_status") {
			volatile_status_file_path = argv[++i];
		} else
		if(arg == "--force") {
			forwarded_args.push_back(arg);
		} else
		if(arg == "--params_file") {
			forwarded_args.push_back(arg);
			forwarded_args.push_back(argv[++i]);
		} else
		if(arg == "--bazel_info_subst") {
			forwarded_args.push_back(arg);
			forwarded_args.push_back(argv[++i]);
			forwarded_args.push_back(argv[++i]);
		} else
		if(arg == "--metafile_out") {
			forwarded_args.push_back(arg);
			forwarded_args.push_back(argv[++i]);
		} else {
			if(i + 1 > argc) {
				std::cerr << "Uneven pairs of inputs" << std::endl;
				return 1;
			}

			paths.push_back({arg, std::string(argv[++i])});
		}
	}

	if(generated_script_path.empty()) {
		std::cerr << "[ERROR] missing --generated_script_path" << std::endl;
		return 1;
	}

	if(out_path.empty()) {
		std::cerr << "[ERROR] missing --output" << std::endl;
		return 1;
	}

	if(tool.empty()) {
		std::cerr << "[ERROR] missing --tool" << std::endl;
		return 1;
	}

	if(!stable_status_file_path.empty()) {
		for_each_status_item(stable_status_file_path, [&](auto key, auto value) {
			substr_str(out_path, "{" + key + "}", value);

			auto find_itr = stamp_subst_map.find(key);
			if(find_itr != stamp_subst_map.end()) {
				forwarded_args.push_back("--subst");
				forwarded_args.push_back("\"" + find_itr->second + "\"");
				forwarded_args.push_back("\"" + value + "\"");

				stamp_subst_map.erase(find_itr);
			}
		});
	}

	if(!volatile_status_file_path.empty()) {
		for_each_status_item(stable_status_file_path, [&](auto key, auto value) {
			substr_str(out_path, "{" + key + "}", value);\

			auto find_itr = stamp_subst_map.find(key);
			if(find_itr != stamp_subst_map.end()) {
				forwarded_args.push_back("--subst");
				forwarded_args.push_back("\"" + find_itr->second + "\"");
				forwarded_args.push_back("\"" + value + "\"");

				stamp_subst_map.erase(find_itr);
			}
		});
	}

	std::cerr << "out_path = " << out_path << std::endl;

	forwarded_args.insert(forwarded_args.begin(), out_path);
	forwarded_args.insert(forwarded_args.begin(), "--output");

	if(!stamp_subst_map.empty()) {
		std::cerr << "[WARNING] Unused stamp_substitutions: " << std::endl;

		for(const auto& entry : stamp_subst_map) {
			std::cerr << "  " << entry.first << std::endl;
		}
	}

	std::ofstream out(generated_script_path);

	out
		<< "#include <string>\n"
		<< "#include <vector>\n"
		<< "#include \"" << tool << "/" << tool << ".hh\"\n"
		<< "int main(int argc, char* argv[]) {\n"
		<< "\tstd::vector<std::string> args;\n";

	for(const auto& forwarded_arg : forwarded_args) {
		out << "\targs.emplace_back(" << escaped_string(forwarded_arg) << ");\n";
	}

	auto idx = 0;
	for(const auto& [target_str, path] : paths) {
		const std::string path_var_name = "p" + std::to_string(idx) + "_";
		out << "\targs.emplace_back(" << escaped_string(target_str) << ");\n";

		out
			<< "\targs.emplace_back(std::filesystem::path("
			<< path_var_name << ").make_preferred().string());\n";

		idx += 1;
	}

	out
		<< "\treturn " << tool << "(argv[0], args);\n"
		<< "}\n";


	return 0;
}
