load("@rules_python//python:py_binary.bzl", "py_binary")
load("@rules_python//python:py_library.bzl", "py_library")
load("@mrpc//scripts:mrpc_gen_py.bzl","mrpc_gen_py")

# mrpc_gen_py(
#     name = "helloworld_stub",
#     src = "//:helloworld.yaml",
# )

py_binary(
    name = "greeter_client",
    srcs = glob(["*.py"]),
    imports = ["."],
    deps = [
        "helloworld_mrpc",
    ],
)

py_binary(
    name = "greeter_server",
    srcs = glob(["*.py"]),
    imports = ["."],
    deps = [
        "helloworld_mrpc",
    ],
)

py_library(
    name = "helloworld_mrpc",
    srcs = glob(["helloworld_mrpc/*.py"]),
    visibility = ["//visibility:__pkg__"],
    deps = [
        "@mrpc//:mrpc-py",
    ],
)
