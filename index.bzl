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
        "_generator": attr.label(
            default = "@bzlws//generator",
            executable = True,
            cfg = "host",
        )
    },
)

def bzlws_copy(name = None, srcs = None, out = None, visibility = None):

    if out.startswith("/"):
        fail("out cannot start with '/'")

    sh_script_name = name + _sh_binary_suffix
    _bzlws_tool_shell_script_src(
        name = sh_script_name,
        srcs = srcs,
        out = out,
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

def bzlws_link(name = None, srcs = None, out = None, visibility = None):

    if out.startswith("/"):
        fail("out cannot start with '/'")

    sh_script_name = name + _sh_binary_suffix
    _bzlws_tool_shell_script_src(
        name = sh_script_name,
        srcs = srcs,
        out = out,
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
