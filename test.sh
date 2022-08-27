#!/usr/bin/env bash

set -e

function test_path_or_exit {
	if [ ! -f "$1" ]; then
		echo "Test FAILED: '$1' does not exist"
	fi
}

bazel run //example:copy_example_txt
test_path_or_exit ./example/ignored_folder/example.txt

bazel run //example:copy_example_txt_file_path
test_path_or_exit ./example/ignored_folder/example/example.txt
