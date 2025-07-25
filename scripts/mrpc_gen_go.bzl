""" 
generate go stub file
"""

load("@rules_go//go:def.bzl", "go_library")

def mrpc_gen_go(name, src):
    """ 
    generate .mrpc.go

    Args:
        name: just for genrule
        src: filename
    """
    if ":" in src:
        f = src.split(":")[-1]  # 取冒号后部分
    else:
        f = src

    proto = f.split(".")[0]

    # TODO: 目前生成工具暂时未完成，此处生成的包暂不可用
    native.genrule(
        name = name + "_mrpc_gen_go",
        srcs = [src],
        outs = [proto + "/" + proto + ".mrpc.go"],
        cmd = """
            out_dir=$$(dirname $(OUTS)) && \
            mkdir -p $$out_dir && \
            $(location @mrpc//src/gen:gen_go) $< $$out_dir
        """,
        tools = ["@mrpc//src/gen:gen_go"],
        visibility = ["//visibility:public"],
    )

    # 这里的importpath也需要修改
    go_library(
        name = name,
        srcs = [name + "_mrpc_gen_go"],
        importpath = "mrpc/go/" + proto,
        deps = [
            "@mrpc//:mrpc-go",
        ],
    )
