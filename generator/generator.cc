#include <filesystem>
#include <string>
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


	std::string output;
	std::string tool;
	std::vector<std::string> paths;
	std::string out_path = std::string(argv[argc-1]);

	for(int i=1; argc-1 > i; ++i) {
		auto arg = std::string(argv[i]);

		if(arg == "--output") {
			output = std::string(argv[++i]);
		} else
		if(arg == "--tool") {
			tool = std::string(argv[++i]);
		} else {
			paths.push_back(arg);
		}
	}

	if(output.empty()) {
		std::cerr << "missing --output" << std::endl;
		return 1;
	}

	if(tool.empty()) {
		std::cerr << "missing --tool" << std::endl;
		return 1;
	}

	if(paths.empty()) {
		std::cerr << "No sources provided" << std::endl;
		return 1;
	}

	std::ofstream out(output);

	out << SCRIPT_SRC_START << "$(rlocation " << tool << ") \\\n";

	for(const auto& path : paths) {
		out << "  " << path << " \\\n";
	}

	out << "  $BUILD_WORKSPACE_DIRECTORY/" << out_path << "\n";

	return 0;
}
