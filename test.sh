#!/usr/bin/env bash

set -e

function test_path_or_exit {
	if [ ! -f "$1" ]; then
		echo "Test FAILED: '$1' does not exist"
	fi
}

git clean -fx example
bazel build //example/...

bazel run --config=test //example:copy_example_txt
test_path_or_exit ./example/ignored_folder/example.txt

bazel run --config=test //example:copy_example_txt_file_path
test_path_or_exit ./example/ignored_folder/example/example.txt

bazel run --config=test //example:copy_example_txt_file_path
test_path_or_exit ./example/ignored_folder/external/faux_repo/example.txt
