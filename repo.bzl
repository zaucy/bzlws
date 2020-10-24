load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def bzlws_deps():
    if not native.existing_rule("com_github_jbeder_yaml_cpp"):
        http_archive(
            name = "com_github_jbeder_yaml_cpp",
            url = "https://github.com/jbeder/yaml-cpp/archive/27d8a0e302c26153b1611c65f4c233ef5db0ba32.zip",
            strip_prefix = "yaml-cpp-27d8a0e302c26153b1611c65f4c233ef5db0ba32",
            sha256 = "4843ba598d1d29e3daefae008d26f95c3de3117dc707757190f6c28e076573a6",
        )
