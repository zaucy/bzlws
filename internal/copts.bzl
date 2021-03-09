_msvc_copts = ["/std:c++17"]
_gcc_copts = ["-std=c++17"]

copts = select({
    "@bazel_tools//src/conditions:windows": _msvc_copts,
    "@bazel_tools//src/conditions:windows_msvc": _msvc_copts,
    "//conditions:default": _gcc_copts,
})
