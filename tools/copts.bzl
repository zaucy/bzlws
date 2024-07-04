load("@bazel_skylib//lib:selects.bzl", "selects")

copts = selects.with_or({
    ("@rules_cc//cc/compiler:clang"): [
        "-std=c++20",
    ],
    ("@rules_cc//cc/compiler:msvc-cl", "@rules_cc//cc/compiler:clang-cl"): [
        "/std:c++20",
        "/permissive-",
        "/Zc:preprocessor",
    ],
})
