load(":bzlws_info.bzl", "BzlwsInfo")

bzlws_sh_binary_suffix = "__bzlws_generator_output"

def _bzlws_tool_shell_script_src(ctx):
    name = ctx.attr.name
    src_filename = name[:-len(bzlws_sh_binary_suffix)] + ".cc"
    src = ctx.actions.declare_file(src_filename)
    args = ctx.actions.args()

    if ctx.attr.force == True:
        args.add("--force")

    if ctx.attr.metafile_path:
        args.add("--metafile_out", ctx.attr.metafile_path)

    if ctx.attr.strip_filepath_prefix:
        args.add("--strip_filepath_prefix", ctx.attr.strip_filepath_prefix)

    for key in ctx.attr.substitutions:
        value = ctx.attr.substitutions[key]
        args.add_all(["--bazel_info_subst", key[BzlwsInfo].name, value])

    for key in ctx.attr.stamp_substitutions:
        value = ctx.attr.stamp_substitutions[key]
        args.add_all(["--stamp_subst", key, value])

    for src_file in ctx.files.srcs:
        src_label_str = _get_full_label_string(src_file.owner)
        args.add(src_label_str)
        args.add(src_file.path)

    params_file = ctx.actions.declare_file(ctx.attr.name + ".params")

    ctx.actions.write(
        output = params_file,
        content = args,
    )

    action_args = ctx.actions.args()

    # needs to be added well before srcs because the underlying tool uses this
    # option to parse other options
    action_args.add("--output", ctx.attr.out)
    action_args.add("--tool", ctx.attr.tool)
    action_args.add("--stable_status", ctx.info_file)
    action_args.add("--volatile_status", ctx.version_file)
    action_args.add("--generated_script_path", src)
    action_args.add("--params_file", params_file)

    ctx.actions.run(
        outputs = [src],
        inputs = ctx.files.srcs + [params_file],
        executable = ctx.executable.generator,
        arguments = [action_args],
    )

    return DefaultInfo(files = depset([src, params_file]))

bzlws_tool_shell_script_src = rule(
    implementation = _bzlws_tool_shell_script_src,
    attrs = {
        "srcs": attr.label_list(mandatory = True, allow_files = True, allow_empty = False),
        "out": attr.string(mandatory = True),
        "tool": attr.string(mandatory = True),
        "force": attr.bool(default = False),
        "strip_filepath_prefix": attr.string(default = "", mandatory = False),
        "metafile_path": attr.string(default = "", mandatory = False),
        "substitutions": attr.label_keyed_string_dict(
            default = {},
            providers = [BzlwsInfo],
            mandatory = False,
        ),
        "stamp_substitutions": attr.string_dict(
            default = {},
            mandatory = False,
        ),
        "generator": attr.label(
            default = "@bzlws//generators/cpp",
            executable = True,
            cfg = "exec",
        ),
    },
)
