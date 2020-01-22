_sh_binary_suffix = "__bzlws_sh_binary_src"
_sh_binary_contents = """
#!/bin/bash

# --- begin runfiles.bash initialization v2 ---
# Copy-pasted from the Bazel Bash runfiles library v2.
set -uo pipefail; f=bazel_tools/tools/bash/runfiles/runfiles.bash
source "${{RUNFILES_DIR:-/dev/null}}/$f" 2>/dev/null || \\
  source "$(grep -sm1 "^$f " "${{RUNFILES_MANIFEST_FILE:-/dev/null}}" | cut -f2- -d' ')" 2>/dev/null || \\
  source "$0.runfiles/$f" 2>/dev/null || \\
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \\
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \\
  {{ echo>&2 "ERROR: cannot find $f"; exit 1; }}; f=; set -e
# --- end runfiles.bash initialization v2 ---

tool_path=$(rlocation {tool})
$tool_path {srcs_list_str} "{out_dir}"
"""

def _bzlws_tool_shell_script_src_impl(ctx):
    name = ctx.attr.name
    src_filename = name[:-len(_sh_binary_suffix)] + ".sh"
    src = ctx.actions.declare_file(src_filename)
    srcs_list_str = ""

    for file in ctx.files.srcs:
        srcs_list_str += "\"{}\" ".format(file.path)

    contents = _sh_binary_contents.format(
        # tool = ctx.executable.tool.path,
        tool = "bzlws/bzlws_copy/bzlws_copy.exe",
        out_dir = ctx.attr.out_dir,
        srcs_list_str = srcs_list_str,
    )

    ctx.actions.write(src, contents, True)
    ctx.runfiles(files = [ctx.executable.tool])

    return DefaultInfo(files = depset([src]))

_bzlws_tool_shell_script_src = rule(
    implementation = _bzlws_tool_shell_script_src_impl,
    attrs = {
        "srcs": attr.label_list(mandatory = True, allow_files = True),
        "out_dir": attr.string(mandatory = True),
        "tool": attr.label(
            mandatory = True,
            executable = True,
            cfg = "host",
        ),
    },
)

def bzlws_copy(name = None, srcs = None, out_dir = None, visibility = None):

    if out_dir.startswith("/"):
        fail("out_dir cannot start with '/'")

    tool = "@bzlws//bzlws_copy:bzlws_copy"

    sh_script_name = name + _sh_binary_suffix
    _bzlws_tool_shell_script_src(
        name = sh_script_name,
        srcs = srcs,
        out_dir = out_dir,
        tool = tool,
        visibility = ["//visibility:private"],
    )

    native.sh_binary(
        name = name,
        srcs = [":" + sh_script_name],
        deps = ["@bazel_tools//tools/bash/runfiles"],
        data = [tool] + srcs,
        visibility = visibility,
    )
