#!/bin/bash

cd $BUILD_WORKSPACE_DIRECTORY/example/helloworld

bazel build \
  //cpp:greeter_client \
  //cpp:greeter_server \
  //go:greeter_client \
  //go:greeter_server \
  //python:greeter_client \
  //python:greeter_server

bazel run :refresh_compile_commands