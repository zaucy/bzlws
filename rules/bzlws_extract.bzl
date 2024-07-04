load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//rules/private:bzlws_tool_cc_binary.bzl", "bzlws_tool_cc_binary")
load("//rules/private:bzlws_util.bzl", "bzlws_check_common_required_attrs")

def bzlws_extract(name = None, srcs = None, out = None, force = None, strip_filepath_prefix = "", metafile_path = "", visibility = None, **kwargs):
    bzlws_check_common_required_attrs("bzlws_extract", name, srcs, out)

    bzlws_tool_cc_binary(
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
