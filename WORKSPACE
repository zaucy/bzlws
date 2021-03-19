workspace(name = "bzlws")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

http_archive(
    name = "com_github_jbeder_yaml_cpp",
    url = "https://github.com/jbeder/yaml-cpp/archive/27d8a0e302c26153b1611c65f4c233ef5db0ba32.zip",
    strip_prefix = "yaml-cpp-27d8a0e302c26153b1611c65f4c233ef5db0ba32",
    sha256 = "4843ba598d1d29e3daefae008d26f95c3de3117dc707757190f6c28e076573a6",
)

git_repository(
    name = "io_bazel_stardoc",
    remote = "https://github.com/bazelbuild/stardoc.git",
    commit = "8275ced1b6952f5ad17ec579a5dd16e102479b72",
    shallow_since = "1611944874 -0500",
)

load("@io_bazel_stardoc//:setup.bzl", "stardoc_repositories")
stardoc_repositories()
