_msvc_copts = ["/std:c++17"]
_gcc_copts = ["-std=c++17"]

copts = select({
    "@bazel_tools//tools/cpp:msvc": _msvc_copts,
    "//conditions:default": _gcc_copts,
})
