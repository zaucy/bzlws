#include <filesystem>
#include <string>
#include <array>
#include <vector>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

static const char SCRIPT_SRC_START[] = R"______(
# --- begin runfiles.bash initialization v2 ---
# Copy-pasted from the Bazel Bash runfiles library v2.
set -uo pipefail; f=bazel_tools/tools/bash/runfiles/runfiles.bash
source "${RUNFILES_DIR:-/dev/null}/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "${RUNFILES_MANIFEST_FILE:-/dev/null}" | cut -f2- -d' ')" 2>/dev/null || \
  source "$0.runfiles/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  { echo>&2 "ERROR: cannot find $f"; exit 1; }; f=; set -e
# --- end runfiles.bash initialization v2 ---

)______";


int main(int argc, char* argv[]) {
	bool force = false;
	std::string generated_script_path;
	std::string tool;
	std::vector<std::string> forwarded_args;
	std::vector<std::array<std::string, 2>> paths;
	std::string out_path;

	for(int i=1; argc-1 > i; ++i) {
		auto arg = std::string(argv[i]);

		if(arg == "--generated_script_path") {
			generated_script_path = std::string(argv[++i]);
		} else
		if(arg == "--output") {
			out_path = std::string(argv[++i]);
			forwarded_args.push_back("--output");
			forwarded_args.push_back(out_path);
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

	if(paths.empty()) {
		std::cerr << "[ERROR] No sources provided" << std::endl;
		return 1;
	}

	std::ofstream out(generated_script_path);

	out << SCRIPT_SRC_START << "$(rlocation " << tool << ") \\\n";

	for(const auto& forwarded_arg : forwarded_args) {
		out << "  " << forwarded_arg << " \\\n";
	}

	for(const auto& [target_str, path] : paths) {
		out << "  " << target_str << " " << path << " \\\n";
	}

	out << "  $BUILD_WORKSPACE_DIRECTORY/" << out_path << "\n";

	return 0;
}
