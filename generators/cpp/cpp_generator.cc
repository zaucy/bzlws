#include <filesystem>
#include <string>
#include <array>
#include <vector>
#include <iostream>
#include <fstream>

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

int main(int argc, char* argv[]) {
	bool force = false;
	std::string output;
	std::string tool;
	std::vector<std::string> forwarded_args;
	std::vector<std::array<std::string, 2>> paths;
	std::string out_path = std::string(argv[argc-1]);

	for(int i=1; argc-1 > i; ++i) {
		auto arg = std::string(argv[i]);

		if(arg == "--output") {
			output = std::string(argv[++i]);
		} else
		if(arg == "--tool") {
			tool = std::string(argv[++i]);
		} else
		if(arg == "--force") {
			forwarded_args.push_back(arg);
		} else
		if(arg == "--subst") {
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

	if(output.empty()) {
		std::cerr << "[ERROR] missing --output" << std::endl;
		return 1;
	}

	if(tool.empty()) {
		std::cerr << "[ERROR] missing --tool" << std::endl;
		return 1;
	}

	if(paths.empty()) {
		std::cerr << "[ERROR] No sources provided" << std::endl;
		return 1;
	}

	std::ofstream out(output);

	out << SCRIPT_SRC_START;

#if _WIN32
	tool += ".exe";
#endif

	out << "\tcmd += runfiles->Rlocation(" << escaped_string(tool) << ");\n";
	out << "\tcmd += \" \";\n";

	for(const auto& forwarded_arg : forwarded_args) {
		out << "\tcmd += " << escaped_string(" " + forwarded_arg + " ") << ";\n";
	}

	auto idx = 0;
	for(const auto& [target_str, path] : paths) {
		const std::string path_var_name = "p" + std::to_string(idx) + "_";
		out << "\tcmd += " << escaped_string("  " + target_str + " ") << ";\n";

		out
			<< "\n\tauto " << path_var_name << " = "
			<< "runfiles->Rlocation(" << escaped_string(path) << ");\n"
			<< "\tif(" << path_var_name << ".empty()) {\n"
			<< "\t\tstd::cerr << \"[ERROR] Cannot find runfile '\" << "
			<< escaped_string(path) << " << \"'\" << std::endl;\n"
			<< "\t\tstd::exit(1);\n"
			<< "\t}\n";

		out
			<< "\tif(!std::filesystem::exists(" + path_var_name + ")) {\n"
			<< "\t\t" << path_var_name << " = " << escaped_string(path) << ";\n"
			<< "\t}\n";

		out
			<< "\tcmd += std::filesystem::path("
			<< path_var_name
			<< ").make_preferred().string() + \" \";\n";

		idx += 1;
	}

	out
		<< "\tcmd += ("
		<< "get_build_workspace_dir() / " << escaped_string(out_path)
		<< ").string();\n";

	out << SCRIPT_SRC_END;

	return 0;
}