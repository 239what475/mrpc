load("@rules_python//python:py_binary.bzl", "py_binary")
load("@mrpc//scripts:mrpc_gen_py.bzl","mrpc_gen_py")

mrpc_gen_py(
    name = "helloworld_stub",
    src = "//:helloworld.yaml",
)

py_binary(
    name = "greeter_client",
    srcs = glob(["*.py"]),
    imports = ["."],
    deps = [
        ":helloworld_stub"
    ],
)

py_binary(
    name = "greeter_server",
    srcs = glob(["*.py"]),
    imports = ["."],
    deps = [
        ":helloworld_stub"
    ],
)