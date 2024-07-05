load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load("@bzlws//rules/private:bzlws_info.bzl", "BzlwsInfo")
load("@bzlws//rules/private:bzlws_util.bzl", "bzlws_get_full_label_string")
load("@rules_cc//cc:find_cc_toolchain.bzl", "find_cc_toolchain", "use_cc_toolchain")

def _bzlws_tool_cc_binary(ctx):
    name = ctx.attr.name
    src_filename = name + ".bzlws__tool.cc"
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
        src_label_str = bzlws_get_full_label_string(src_file.owner)
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
        executable = ctx.executable._generator,
        arguments = [action_args],
    )

    cc_toolchain = find_cc_toolchain(ctx)
    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
    )
    variables = cc_common.create_link_variables(
        cc_toolchain = cc_toolchain,
        feature_configuration = feature_configuration,
    )
    env = cc_common.get_environment_variables(
        feature_configuration = feature_configuration,
        action_name = ACTION_NAMES.cpp_link_dynamic_library,
        variables = variables,
    )

    compilation_contexts = []
    for dep in ctx.attr.deps:
        compilation_contexts.append(dep[CcInfo].compilation_context)

    (compilation_context, compilation_output) = cc_common.compile(
        name = ctx.attr.name,
        actions = ctx.actions,
        feature_configuration = feature_configuration,
        cc_toolchain = cc_toolchain,
        srcs = [src],
        compilation_contexts = compilation_contexts,
    )

    linking_contexts = []
    for dep in ctx.attr.deps:
        linking_contexts.append(dep[CcInfo].linking_context)

    output = cc_common.link(
        actions = ctx.actions,
        name = ctx.attr.name,
        feature_configuration = feature_configuration,
        cc_toolchain = cc_toolchain,
        compilation_outputs = compilation_output,
        linking_contexts = linking_contexts,
    )

    return DefaultInfo(
        files = depset([output.executable]),
        default_runfiles = ctx.runfiles(ctx.files.srcs + [output.executable, params_file]),
        executable = output.executable,
    )

bzlws_tool_cc_binary = rule(
    implementation = _bzlws_tool_cc_binary,
    attrs = {
        "srcs": attr.label_list(mandatory = True, allow_files = True, allow_empty = False),
        "out": attr.string(mandatory = True),
        "tool": attr.string(mandatory = True),
        "force": attr.bool(default = False),
        "deps": attr.label_list(providers = [CcInfo], allow_empty = True),
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
        "_generator": attr.label(
            default = "@bzlws//tools/generators/cpp",
            executable = True,
            cfg = "exec",
        ),
    },
    toolchains = use_cc_toolchain(),
    fragments = ["cpp"],
    executable = True,
)
