load("@rules_go//go:def.bzl", "go_binary", "go_library")
load("@mrpc//scripts:mrpc_gen_go.bzl","mrpc_gen_go")

mrpc_gen_go(
    name = "helloworld_stub",
    src = "//:helloworld.yaml",
)

go_binary(
    name = "greeter_client",
    srcs = ["greeter_client.go"],
    deps = [
        ":helloworld_stub"
    ],
)

go_binary(
    name = "greeter_server",
    srcs = ["greeter_server.go"],
    deps = [
        ":helloworld_stub"
    ],
)
