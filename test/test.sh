#!/usr/bin/env bash

set -e

function test_path_or_exit {
	if [ ! -f "$1" ]; then
		echo "Test FAILED: '$1' does not exist"
		exit 1
	fi
}

git clean -fx . -e test/bazel-*
bazel build //...

bazel run --config=test //:copy_example_txt
test_path_or_exit ./ignored_folder/example.txt

bazel run --config=test //:copy_example_txt_file_path
test_path_or_exit ./ignored_folder/exampledir/example.txt

bazel run --config=test //:copy_example_txt_file_path_external
test_path_or_exit ./ignored_folder/external/faux_repo/example.txt

git clean -fx . -e test/bazel-*

bazel run --config=test --config=ci //:copy_example_txt_file_path_external
test_path_or_exit ./ignored_folder/external/faux_repo/example.txt

bazel run --config=test //:remap_paths_example
test_path_or_exit ./ignored_folder/remapped_dir/example.txt

bazel run --config=test //:link_remap_paths_example
test_path_or_exit ./ignored_folder/link_remapped_dir/example.txt

bazel run --config=test //:excludes_example
test_path_or_exit ./ignored_folder/exampledir/example.txt
if [ -f "./ignored_folder/info.txt" ]; then
    echo "Test FAILED: info.txt was not excluded"
    exit 1
fi

bazel run --config=test //:link_excludes_example
test_path_or_exit ./ignored_folder/link_excludes_example.txt
if [ -f "./ignored_folder/link_excludes_info.txt" ]; then
    echo "Test FAILED: link_excludes_info.txt was not excluded"
    exit 1
fi
