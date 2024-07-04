#!/usr/bin/env bash

set -e

function test_path_or_exit {
	if [ ! -f "$1" ]; then
		echo "Test FAILED: '$1' does not exist"
	fi
}

git clean -fx . -e test/bazel-*
bazel build //...

bazel run --config=test //:copy_example_txt
test_path_or_exit ./ignored_folder/example.txt

bazel run --config=test //:copy_example_txt_file_path
test_path_or_exit ./ignored_folder/example/example.txt

bazel run --config=test //:copy_example_txt_file_path
test_path_or_exit ./ignored_folder/external/faux_repo/example.txt

git clean -fx . -e test/bazel-*

bazel run --config=test --config=ci //:copy_example_txt_file_path_external
test_path_or_exit ./ignored_folder/external/faux_repo/example.txt
