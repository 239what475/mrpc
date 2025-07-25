""" 
generate cc stub file
"""

load("@rules_cc//cc:cc_library.bzl", "cc_library")

def mrpc_gen_cc(name, src):
    """ 
    generate .mrpc.h

    Args:
        name: just for genrule
        src: filename
    """
    if ":" in src:
        f = src.split(":")[-1]  # 取冒号后部分
    else:
        f = src

    proto = f.split(".")[0]

    native.genrule(
        name = name + "_mrpc_gen_cc_h",
        srcs = [src],
        outs = [
            proto + ".mrpc.h",
            proto + ".mrpc.cc",
        ],
        cmd = """
            out_dirs=$$(dirname $(OUTS)) && \
            out_dir=$$(echo $$out_dirs | cut -d' ' -f1)  && \
            $(location @mrpc//src/gen:gen_cc) $< $$out_dir
        """,
        tools = ["@mrpc//src/gen:gen_cc"],
        visibility = ["//visibility:public"],
    )

    cc_library(
        name = name,
        hdrs = [proto + ".mrpc.h"],
        srcs = [proto + ".mrpc.cc"],
        deps = ["@mrpc//:mrpc-cpp"],
        # alwayslink = True,
        visibility = ["//visibility:public"],
    )
