load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")
load("@rules_shell//shell:sh_binary.bzl","sh_binary")

# bazel run :refresh_compile_commands
refresh_compile_commands(
    name = "refresh_compile_commands",
    exclude_external_sources = True,
    exclude_headers = "all",

    # Specify the targets of interest.
    # For example, specify a dict of targets and any flags required to build.
    targets = [
        "//src/...",
    ],
)

sh_binary(
    name = "helloworld_build",
    srcs = ["helloworld_build.sh"],
    visibility = ["//visibility:public"],
)

exports_files(["mrpc_gen_cc.bzl"])
exports_files(["mrpc_gen_go.bzl"])
exports_files(["mrpc_gen_py.bzl"])
