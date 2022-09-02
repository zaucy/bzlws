workspace(name = "bzlws")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

http_archive(
    name = "com_github_jbeder_yaml_cpp",
    sha256 = "4843ba598d1d29e3daefae008d26f95c3de3117dc707757190f6c28e076573a6",
    strip_prefix = "yaml-cpp-27d8a0e302c26153b1611c65f4c233ef5db0ba32",
    url = "https://github.com/jbeder/yaml-cpp/archive/27d8a0e302c26153b1611c65f4c233ef5db0ba32.zip",
)

load("//:faux_repo.bzl", "faux_repo")

faux_repo(name = "faux_repo")

git_repository(
    name = "io_bazel_stardoc",
    commit = "8275ced1b6952f5ad17ec579a5dd16e102479b72",
    remote = "https://github.com/bazelbuild/stardoc.git",
    shallow_since = "1611944874 -0500",
)

load("@io_bazel_stardoc//:setup.bzl", "stardoc_repositories")

stardoc_repositories()

http_archive(
    name = "hedron_compile_commands",
    sha256 = "f5cf960d7477b95546a96b05397129969133ec0c1af9889e5d02ebe42dba6abd",
    strip_prefix = "bazel-compile-commands-extractor-2a72a3b761e21a0405995b323a3b765d93bd6df4",
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/2a72a3b761e21a0405995b323a3b765d93bd6df4.tar.gz",
)

load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")

hedron_compile_commands_setup()

http_archive(
    name = "com_grail_bazel_toolchain",
    sha256 = "7fa5a8624b1148c36e09c7fa29ef6ee8b83f865219c9c219c9125aac78924758",
    strip_prefix = "bazel-toolchain-c3131a6894804ee586d059c57ffe8e88d44172e1",
    url = "https://github.com/grailbio/bazel-toolchain/archive/c3131a6894804ee586d059c57ffe8e88d44172e1.zip",
)

load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain",
    cxx_standard = {"linux": "c++20"},
    llvm_version = "14.0.0",
)

load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")

llvm_register_toolchains()
