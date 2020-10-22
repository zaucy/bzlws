load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

_sh_binary_suffix = "__bzlws_sh_binary_src"

def _get_full_label_string(label):
    if not label:
        return "@"
    
    return "@" + label.workspace_name + "//" + label.package + ":" + label.name 

def _file_owner_label_pair(file):
    return [_get_full_label_string(file.owner), file.path]

def _bzlws_tool_shell_script_src_impl(ctx):
    name = ctx.attr.name
    src_filename = name[:-len(_sh_binary_suffix)] + ".sh"
    src = ctx.actions.declare_file(src_filename)
    args = ctx.actions.args()

    args.add("--tool", ctx.attr.tool)
    if ctx.attr.force == True:
        args.add("--force")

    if ctx.attr.metafile_path:
        args.add("--metafile_out", ctx.attr.metafile_path)

    args.add("--output", src)
    args.add_all(ctx.files.srcs, map_each = _file_owner_label_pair)
    args.add(ctx.attr.out)

    ctx.actions.run(
        outputs = [src],
        inputs = ctx.files.srcs,
        executable = ctx.executable._generator,
        arguments = [args],
    )

    return DefaultInfo(files = depset([src]))

_bzlws_tool_shell_script_src = rule(
    implementation = _bzlws_tool_shell_script_src_impl,
    attrs = {
        "srcs": attr.label_list(mandatory = True, allow_files = True),
        "out": attr.string(mandatory = True),
        "tool": attr.string(mandatory = True),
        "force": attr.bool(default = False),
        "metafile_path": attr.string(default = "", mandatory = False),
        "_generator": attr.label(
            default = "@bzlws//generator",
            executable = True,
            cfg = "host",
        )
    },
)

def bzlws_copy(name = None, srcs = None, out = None, force = None, metafile_path = "", visibility = None):
    """Copy generated files into workspace directory

    Args:
        name: Name used for executable target
        srcs: List of files that should be copied
        out: Output path within the workspace. Certain strings get replaced with
             various information about

             `{BAZEL_LABEL_NAME}` - Label name
             `{BAZEL_LABEL_PACKAGE}` - Label package
             `{BAZEL_LABEL_WORKSPACE_NAME}`  - Workspace name of the label
             `{BAZEL_FULL_LABEL}` - Fulll label string
             `{BAZEL_LABEL}` - Full label without the workspace name
             `{EXT}` - File extension (with the dot)
             `{EXTNAME}` - File extension name (without the dot)
             `{BASENAME}` - Path basename

        force: Overwrite existing paths even if they are not files
        metafile_path: Path to metafile
        visibility: visibility of the executable target
    """
    if out.startswith("/"):
        fail("out cannot start with '/'")

    sh_script_name = name + _sh_binary_suffix
    _bzlws_tool_shell_script_src(
        name = sh_script_name,
        srcs = srcs,
        out = out,
        force = force,
        metafile_path = metafile_path,
        tool = "bzlws/bzlws_copy/bzlws_copy.exe",
        visibility = ["//visibility:private"],
    )

    native.sh_binary(
        name = name,
        srcs = [":" + sh_script_name],
        deps = ["@bazel_tools//tools/bash/runfiles"],
        data = ["@bzlws//bzlws_copy:bzlws_copy"] + srcs,
        visibility = visibility,
    )

def bzlws_link(name = None, srcs = None, out = None, force = None, metafile_path = "", visibility = None):
    """Symlink generated files into workspace directory

    Args:
        name: Name used for executable target
        srcs: List of files that should be symlinked
        out: Output path within the workspace. Certain strings get replaced with
             various information about each source target from `srcs`

             `{BAZEL_LABEL_NAME}` - Label name
             `{BAZEL_LABEL_PACKAGE}` - Label package
             `{BAZEL_LABEL_WORKSPACE_NAME}`  - Workspace name of the label
             `{BAZEL_FULL_LABEL}` - Fulll label string
             `{BAZEL_LABEL}` - Full label without the workspace name
             `{EXT}` - File extension (with the dot)
             `{EXTNAME}` - File extension name (without the dot)
             `{BASENAME}` - Path basename

        force: Overwrite existing paths even if they are not symlinks
        metafile_path: Path to metafile
        visibility: visibility of the executable target
    """
    if out.startswith("/"):
        fail("out cannot start with '/'")

    sh_script_name = name + _sh_binary_suffix
    _bzlws_tool_shell_script_src(
        name = sh_script_name,
        srcs = srcs,
        out = out,
        force = force,
        metafile_path = metafile_path,
        tool = "bzlws/bzlws_link/bzlws_link.exe",
        visibility = ["//visibility:private"],
    )

    native.sh_binary(
        name = name,
        srcs = [":" + sh_script_name],
        deps = ["@bazel_tools//tools/bash/runfiles"],
        data = ["@bzlws//bzlws_link:bzlws_link"] + srcs,
        visibility = visibility,
    )

def bzlws_deps():
    if not native.existing_rule("com_github_jbeder_yaml_cpp"):
        http_archive(
            name = "com_github_jbeder_yaml_cpp",
            url = "https://github.com/jbeder/yaml-cpp/archive/27d8a0e302c26153b1611c65f4c233ef5db0ba32.zip",
            strip_prefix = "yaml-cpp-27d8a0e302c26153b1611c65f4c233ef5db0ba32",
            sha256 = "4843ba598d1d29e3daefae008d26f95c3de3117dc707757190f6c28e076573a6",
        )
