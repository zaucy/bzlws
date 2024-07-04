load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//tools:copts.bzl", "copts")
load("//rules/private:bzlws_tool_shell_script_src.bzl", "bzlws_tool_shell_script_src", "bzlws_sh_binary_suffix")
load("//rules/private:bzlws_util.bzl", "bzlws_check_common_required_attrs")

def bzlws_extract(name = None, srcs = None, out = None, force = None, strip_filepath_prefix = "", metafile_path = "", visibility = None, **kwargs):
    bzlws_check_common_required_attrs("bzlws_extract", name, srcs, out)

    sh_script_name = name + bzlws_sh_binary_suffix
    bzlws_tool_shell_script_src(
        name = sh_script_name,
        srcs = srcs,
        out = out,
        strip_filepath_prefix = strip_filepath_prefix,
        force = force,
        metafile_path = metafile_path,
        tool = "bzlws_extract",
        visibility = ["//visibility:private"],
        **kwargs
    )

    cc_binary(
        name = name,
        srcs = [":" + sh_script_name],
        deps = ["@bzlws//tools/bzlws_extract"],
        data = srcs,
        copts = copts,
        visibility = visibility,
        **kwargs
    )
