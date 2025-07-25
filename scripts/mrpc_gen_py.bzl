""" 
generate python stub file
"""

load("@rules_python//python:py_library.bzl", "py_library")

def mrpc_gen_py(name, src):
    """ 
    generate .mrpc.py

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
        name = name + "_mrpc_gen_py",
        srcs = [src],
        outs = [
            proto + "/" + proto + "_mrpc.py",
            proto + "/__init__.py",
        ],
        cmd = """
            out_dirs=$$(dirname $(OUTS)) && \
            out_dir=$$(echo $$out_dirs | cut -d' ' -f1)  && \
            mkdir -p $$out_dir && \
            $(location @mrpc//src/gen:gen_py) $< $$out_dir
        """,
        tools = ["@mrpc//src/gen:gen_py"],
        visibility = ["//visibility:public"],
    )

    py_library(
        name = name,
        srcs = [name + "_mrpc_gen_py"],
        visibility = ["//visibility:public"],
        deps = [
            "@mrpc//:mrpc-py",
        ],
    )
