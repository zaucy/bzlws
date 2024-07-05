load("//rules/private:bzlws_tool_cc_binary.bzl", "bzlws_tool_cc_binary")
load("//rules/private:bzlws_util.bzl", "bzlws_check_common_required_attrs")

def bzlws_copy(name = None, srcs = None, out = None, force = None, strip_filepath_prefix = "", metafile_path = "", substitutions = {}, stamp_substitutions = {}, visibility = None, tags = [], **kwargs):
    """Copy generated files into workspace directory

    ```python
    load("@bzlws//rules:bzlws_copy.bzl", "bzlws_copy")
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

            `{FILENAME}` - File name with extension

            `{FILEPATH}` - File path. https://bazel.build/rules/lib/File#path

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
    bzlws_check_common_required_attrs("bzlws_copy", name, srcs, out)

    bzlws_tool_cc_binary(
        name = name,
        srcs = srcs,
        out = out,
        strip_filepath_prefix = strip_filepath_prefix,
        force = force,
        metafile_path = metafile_path,
        substitutions = substitutions,
        stamp_substitutions = stamp_substitutions,
        tool = "bzlws_copy",
        deps = ["@bzlws//tools/bzlws_copy"],
        visibility = ["//visibility:private"],
        tags = tags + ["ibazel_notify_changes"],
        **kwargs
    )
