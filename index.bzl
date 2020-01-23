_sh_binary_suffix = "__bzlws_sh_binary_src"

def _bzlws_tool_shell_script_src_impl(ctx):
    name = ctx.attr.name
    src_filename = name[:-len(_sh_binary_suffix)] + ".sh"
    src = ctx.actions.declare_file(src_filename)
    args = ctx.actions.args()

    args.add("--tool", ctx.attr.tool)
    args.add("--output",    src)
    args.add_all(ctx.files.srcs)
    args.add(ctx.attr.out_dir)

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
        "out_dir": attr.string(mandatory = True),
        "tool": attr.string(mandatory = True),
        "_generator": attr.label(
            default = "@bzlws//generator",
            executable = True,
            cfg = "host",
        )
    },
)

def bzlws_copy(name = None, srcs = None, out_dir = None, visibility = None):

    if out_dir.startswith("/"):
        fail("out_dir cannot start with '/'")

    sh_script_name = name + _sh_binary_suffix
    _bzlws_tool_shell_script_src(
        name = sh_script_name,
        srcs = srcs,
        out_dir = out_dir,
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
