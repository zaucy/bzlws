load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//internal:bzlws_info.bzl", "BzlwsInfo")

_sh_binary_suffix = "__bzlws_generator_output"

def _get_full_label_string(label):
    if not label:
        return "@"
    
    return "@" + label.workspace_name + "//" + label.package + ":" + label.name 

def _get_file_path(ctx, file):
    if file.path.startswith("external/"):
        return file.path[len("external/"):]

    return file.path

def _file_owner_label_pair(file):
    return [_get_full_label_string(file.owner), _get_file_path(file)]

def _bzlws_tool_shell_script_src_impl(ctx):
    name = ctx.attr.name
    src_filename = name[:-len(_sh_binary_suffix)] + ".cc"
    src = ctx.actions.declare_file(src_filename)
    args = ctx.actions.args()

    args.add("--tool", ctx.attr.tool)
    args.add("--stable_status", ctx.info_file)
    args.add("--volatile_status", ctx.version_file)

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

    args.add("--output", src)
    for src_file in ctx.files.srcs:
        src_label_str = _get_full_label_string(src_file.owner)
        src_file_path = _get_file_path(ctx, src_file)
        args.add(src_label_str)
        args.add(src_file_path)

    args.add(ctx.attr.out)

    ctx.actions.run(
        outputs = [src],
        inputs = ctx.files.srcs,
        executable = ctx.executable.generator,
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
        )
    },
)

def _validate_required_attrs(rule_name, name, srcs, out):
    
    if not name:
        fail("{} - missing name attribute".format(rule_name))

    if not srcs or len(srcs) == 0:
        fail("{} - missing srcs attribute or is empty".format(rule_name))

    if not out:
        fail("{} - missing out attribute".format(rule_name))

    if out.startswith("/"):
        fail("{} - out attribute cannot start with '/'".format(rule_name))

def bzlws_copy(name = None, srcs = None, out = None, force = None, strip_filepath_prefix = "", metafile_path = "", substitutions = {}, stamp_substitutions = {}, visibility = None, tags = [], **kwargs):
    """Copy generated files into workspace directory

    ```python
    load("@bzlws//:index.bzl", "bzlws_copy")
    ```

    Args:
        name: Name used for executable target

        srcs: List of files that should be copied

        out: Output path within the workspace. Certain strings get replaced with
            workspace status values and information about the `srcs`. This
            happens in 2 phases.

            **Phase 1:**

            The [workspace status](https://docs.bazel.build/versions/master/user-manual.html#workspace_status)
            stamp values get replaced in this format: `{KEY}`. For example if
            you would like to have the name of the host machine in your output
            path you would put `out = "my/path/{BUILD_HOST}/{FILENAME}"`

            **Phase 2:**

            The following gets replaced about each items in `srcs`

            `{BAZEL_LABEL_NAME}` - Label name

            `{BAZEL_LABEL_PACKAGE}` - Label package

            `{BAZEL_LABEL_WORKSPACE_NAME}`  - Workspace name of the label

            `{BAZEL_FULL_LABEL}` - Fulll label string

            `{BAZEL_LABEL}` - Full label without the workspace name

            `{EXT}` - File extension (with the dot)

            `{EXTNAME}` - File extension name (without the dot)

            `{FILENAME}` - Full file name with extension

            `{FILEPATH}` - Fulle file path. Any relative paths are stripped

            `{BASENAME}` - Path basename

        strip_filepath_prefix: Strip prefix of `{FILEPATH}`

        force: Overwrite existing paths even if they are not files

        metafile_path: Path to metafile

        substitutions: BzlwsInfo label keyed, string valued, dictionary. The
            values will be replaced in the source files with the values from the
            `bazel info` command. The available BzlwsInfo targets are in the
            `@bzlws//info` package.

        stamp_substitutions: Workspace status keyed, string valued, dictionary.
            The values will be replaced in the sources files the values from the
            workspace status matching the key.

        visibility: visibility of the executable target

        tags: forwarded to underlying targets

        **kwargs: rest of arguments get passed to underlying targets
    """
    _validate_required_attrs("bzlws_copy", name, srcs, out)

    sh_script_name = name + _sh_binary_suffix
    _bzlws_tool_shell_script_src(
        name = sh_script_name,
        srcs = srcs,
        out = out,
        strip_filepath_prefix = strip_filepath_prefix,
        force = force,
        metafile_path = metafile_path,
        substitutions = substitutions,
        stamp_substitutions = stamp_substitutions,
        tool = "bzlws/bzlws_copy/bzlws_copy",
        visibility = ["//visibility:private"],
        tags = tags,
        **kwargs,
    )

    cc_binary(
        name = name,
        srcs = [":" + sh_script_name],
        deps = ["@bazel_tools//tools/cpp/runfiles"],
        data = ["@bzlws//bzlws_copy:bzlws_copy"] + srcs,
        visibility = visibility,
        tags = tags + ["ibazel_notify_changes"],
        **kwargs
    )

def bzlws_link(name = None, srcs = None, out = None, force = None, strip_filepath_prefix = "", metafile_path = "", visibility = None, **kwargs):
    """ Symlink generated files into workspace directory
    
    ```python
    load("@bzlws//:index.bzl", "bzlws_link")
    ```

    Args:
        name: Name used for executable target

        srcs: List of files that should be symlinked

        out: Output path within the workspace. Certain strings get replaced with
            workspace status values and information about the `srcs`. This
            happens in 2 phases.

            **Phase 1:**

            The [workspace status](https://docs.bazel.build/versions/master/user-manual.html#workspace_status)
            stamp values get replaced in this format: `{KEY}`. For example if
            you would like to have the name of the host machine in your output
            path you would put `out = "my/path/{BUILD_HOST}/{FILENAME}"`

            **Phase 2:**

            The following gets replaced about each items in `srcs`

            `{BAZEL_LABEL_NAME}` - Label name

            `{BAZEL_LABEL_PACKAGE}` - Label package

            `{BAZEL_LABEL_WORKSPACE_NAME}`  - Workspace name of the label

            `{BAZEL_FULL_LABEL}` - Fulll label string

            `{BAZEL_LABEL}` - Full label without the workspace name

            `{EXT}` - File extension (with the dot)

            `{EXTNAME}` - File extension name (without the dot)

            `{FILENAME}` - Full file name with extension

            `{FILEPATH}` - Fulle file path. Any relative paths are stripped

            `{BASENAME}` - Path basename

        strip_filepath_prefix: Strip prefix of `{FILEPATH}`

        force: Overwrite existing paths even if they are not symlinks

        metafile_path: Path to metafile

        visibility: visibility of the executable target

        **kwargs: rest of arguments get passed to underlying targets
    """
    _validate_required_attrs("bzlws_link", name, srcs, out)

    sh_script_name = name + _sh_binary_suffix
    _bzlws_tool_shell_script_src(
        name = sh_script_name,
        srcs = srcs,
        out = out,
        strip_filepath_prefix = strip_filepath_prefix,
        force = force,
        metafile_path = metafile_path,
        tool = "bzlws/bzlws_link/bzlws_link",
        visibility = ["//visibility:private"],
        **kwargs,
    )

    cc_binary(
        name = name,
        srcs = [":" + sh_script_name],
        deps = ["@bazel_tools//tools/cpp/runfiles"],
        data = ["@bzlws//bzlws_link:bzlws_link"] + srcs,
        visibility = visibility,
        **kwargs
    )
