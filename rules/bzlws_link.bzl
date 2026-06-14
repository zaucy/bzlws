load("//rules/private:bzlws_tool_cc_binary.bzl", "bzlws_tool_cc_binary")
load("//rules/private:bzlws_util.bzl", "bzlws_check_common_required_attrs")

def bzlws_link(name = None, srcs = None, excludes = [], out = None, force = None, strip_filepath_prefix = "", remap_paths = {}, metafile_path = "", visibility = None, **kwargs):
    """ Symlink generated files into workspace directory

    Args:
        name: Name used for executable target

        srcs: List of files that should be symlinked

        excludes: Files to exclude from the output directory. Each element must
            refer to an individual file in src. All exclusions must be used.

        out: Output path within the workspace. Certain strings get replaced with
            workspace status values and information about the `srcs`. This
            happens in 2 phases.

            **Phase 1:**

            The [workspace status](https://docs.bazel.build/versions/master/user-manual.html#workspace_status)
            stamp values get replaced in this format: `{KEY}`. For example if
            you would like to have the name of the host machine in your output
            path you would put `out = "my/path/{BUILD_HOST}/{FILENAME}"`

            The following target platform values are also replaced:

            `{TARGET_OS}` - Target platform OS (e.g. `linux`, `windows`, `macos`)

            `{TARGET_CPU}` - Target platform CPU (e.g. `x86_64`, `aarch64`, `arm`)

            These are resolved from `@platforms//os` and `@platforms//cpu`
            constraint values at analysis time. They work correctly with
            cross-compilation transition rules.

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

        force: Overwrite existing paths even if they are not symlinks

        metafile_path: Path to metafile

        remap_paths: Dictionary mapping path prefixes to replacements. When
            `{FILEPATH}` is used in `out`, any filepath that starts with a key
            in this dictionary will have that prefix replaced with the
            corresponding value. For example,
            `{"src/zcpx": "", "external/bimg": "tools"}` would strip the
            `src/zcpx` prefix and remap `external/bimg` to `tools`.

        visibility: visibility of the executable target

        **kwargs: rest of arguments get passed to underlying targets
    """
    bzlws_check_common_required_attrs("bzlws_link", name, srcs, out)

    bzlws_tool_cc_binary(
        name = name,
        srcs = srcs,
        excludes = excludes,
        out = out,
        strip_filepath_prefix = strip_filepath_prefix,
        remap_paths = remap_paths,
        force = force,
        metafile_path = metafile_path,
        tool = "bzlws_link",
        deps = ["@bzlws//tools/bzlws_link"],
        visibility = ["//visibility:private"],
        **kwargs
    )
