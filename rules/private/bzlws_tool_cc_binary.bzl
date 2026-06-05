load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load("@bzlws//rules/private:bzlws_info.bzl", "BzlwsInfo")
load("@bzlws//rules/private:bzlws_platform_info.bzl", "BzlwsPlatformInfo")
load("@bzlws//rules/private:bzlws_util.bzl", "bzlws_get_full_label_string")
load("@rules_cc//cc:find_cc_toolchain.bzl", "find_cc_toolchain", "use_cc_toolchain")
load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")



# Well-known @platforms//os constraint values
_OS_CONSTRAINT_VALUES = [
    "android",
    "freebsd",
    "ios",
    "linux",
    "macos",
    "netbsd",
    "openbsd",
    "qnx",
    "windows",
]

# Well-known @platforms//cpu constraint values
_CPU_CONSTRAINT_VALUES = [
    "aarch64",
    "arm",
    "armv6",
    "armv7",
    "ppc",
    "riscv32",
    "riscv64",
    "s390x",
    "wasm32",
    "wasm64",
    "x86_32",
    "x86_64",
]

def _resolve_constraint(ctx, attr_name_prefix, constraint_values):
    """Resolve which constraint value matches the target platform."""
    for name in constraint_values:
        attr_name = attr_name_prefix + name
        constraint_value = getattr(ctx.attr, attr_name)
        if ctx.target_platform_has_constraint(
            constraint_value[platform_common.ConstraintValueInfo],
        ):
            return name
    return ""

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

    # Resolve target platform constraint values
    target_os = _resolve_constraint(ctx, "_os_", _OS_CONSTRAINT_VALUES)
    target_cpu = _resolve_constraint(ctx, "_cpu_", _CPU_CONSTRAINT_VALUES)

    # Generate the platform manifest file
    manifest_file = ctx.actions.declare_file(ctx.attr.name + ".platforms.txt")

    file_platforms = {}
    for src_target in ctx.attr.srcs:
        if BzlwsPlatformInfo in src_target:
            for entry in src_target[BzlwsPlatformInfo].mapping.to_list():
                file_platforms[entry.file.path] = (entry.os, entry.cpu)

    manifest_content = []
    for path, (os, cpu) in file_platforms.items():
        manifest_content.append("{} {} {}".format(path, os, cpu))

    ctx.actions.write(
        output = manifest_file,
        content = "\n".join(manifest_content) + "\n",
    )

    args.add("--platform_manifest")
    args.add(manifest_file.path)
    args.add("--default_target_os", target_os)
    args.add("--default_target_cpu", target_cpu)

    for src_file in ctx.files.srcs:
        src_label_str = bzlws_get_full_label_string(src_file.owner)
        args.add(src_label_str)
        args.add(src_file.path)

    params_file = ctx.actions.declare_file(ctx.attr.name + ".params")

    ctx.actions.write(
        output = params_file,
        content = args,
    )

    # Resolve target platform constraint values and substitute into out path
    out_path = ctx.attr.out

    action_args = ctx.actions.args()

    # needs to be added well before srcs because the underlying tool uses this
    # option to parse other options
    action_args.add("--output", out_path)
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
        default_runfiles = ctx.runfiles(
            files = ctx.files.srcs + [output.executable, manifest_file],
        ),
        executable = output.executable,
    )

# Build the platform constraint attrs dynamically
_PLATFORM_ATTRS = {}
_PLATFORM_ATTRS.update({
    "_os_" + name: attr.label(
        default = Label("@platforms//os:" + name),
        providers = [platform_common.ConstraintValueInfo],
    )
    for name in _OS_CONSTRAINT_VALUES
})
_PLATFORM_ATTRS.update({
    "_cpu_" + name: attr.label(
        default = Label("@platforms//cpu:" + name),
        providers = [platform_common.ConstraintValueInfo],
    )
    for name in _CPU_CONSTRAINT_VALUES
})

def _bzlws_platform_aspect_impl(target, ctx):
    direct = []
    transitive = []

    # 1. Collect from dependency attributes
    for attr_name in ["srcs", "deps", "actual", "dep", "exports"]:
        if hasattr(ctx.rule.attr, attr_name):
            attr_val = getattr(ctx.rule.attr, attr_name)
            if type(attr_val) == "list":
                for dep in attr_val:
                    if BzlwsPlatformInfo in dep:
                        transitive.append(dep[BzlwsPlatformInfo].mapping)
            elif type(attr_val) == "Target":
                if BzlwsPlatformInfo in attr_val:
                    transitive.append(attr_val[BzlwsPlatformInfo].mapping)

    # 2. For the target itself, associate its output files with the target OS/CPU
    target_os = _resolve_constraint(ctx, "_os_", _OS_CONSTRAINT_VALUES)
    target_cpu = _resolve_constraint(ctx, "_cpu_", _CPU_CONSTRAINT_VALUES)

    if target_os or target_cpu:
        if DefaultInfo in target:
            for f in target[DefaultInfo].files.to_list():
                if f.owner == target.label:
                    direct.append(struct(
                        file = f,
                        os = target_os,
                        cpu = target_cpu,
                    ))

    return [BzlwsPlatformInfo(
        mapping = depset(direct, transitive = transitive),
    )]

bzlws_platform_aspect = aspect(
    implementation = _bzlws_platform_aspect_impl,
    attr_aspects = ["srcs", "deps", "actual", "dep", "exports"],
    attrs = _PLATFORM_ATTRS,
)

bzlws_tool_cc_binary = rule(
    implementation = _bzlws_tool_cc_binary,
    attrs = dict({
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = True,
            allow_empty = False,
            aspects = [bzlws_platform_aspect],
        ),
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
    }, **_PLATFORM_ATTRS),
    toolchains = use_cc_toolchain(),
    fragments = ["cpp"],
    executable = True,
)
