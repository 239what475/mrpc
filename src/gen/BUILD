load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "gen_cc",
    srcs = [
        "gen_cc_stub.cc",
        "gen_stub.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@yaml-cpp//:yaml-cpp",
    ],
)

cc_binary(
    name = "gen_go",
    srcs = [
        "gen_go_stub.cc",
        "gen_stub.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@yaml-cpp//:yaml-cpp",
    ],
)

cc_binary(
    name = "gen_py",
    srcs = [
        "gen_py_stub.cc",
        "gen_stub.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@yaml-cpp//:yaml-cpp",
    ],
)
