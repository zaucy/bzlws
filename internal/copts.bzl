_msvc_copts = ["/std:c++17"]
_clang_cl_copts = ["/std:c++17"]
_gcc_copts = ["-std=c++17"]

copts = select({
    "@bazel_tools//tools/cpp:msvc": _msvc_copts,
    "@bazel_tools//tools/cpp:clang-cl": _clang_cl_copts,
    "//conditions:default": _gcc_copts,
})
