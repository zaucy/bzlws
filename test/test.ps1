#!/usr/bin/env pwsh

$ErrorActionPreference = 'Stop'

function Test-Path-Or-Exit($Path) {
	if (!(Test-Path -Path $Path)) {
		Write-Error "Test FAILED: '$Path' does not exist"
		exit 1
	}
}

git clean -fx . -e test/bazel-*
bazel build //...

bazel run --config=test //:copy_example_txt
Test-Path-Or-Exit .\ignored_folder\example.txt

bazel run --config=test //:copy_example_txt_file_path
Test-Path-Or-Exit .\ignored_folder\exampledir\example.txt

bazel run --config=test //:copy_example_txt_file_path_external
Test-Path-Or-Exit .\ignored_folder\external\faux_repo\example.txt

git clean -fx . -e test/bazel-*

bazel run --config=test --config=ci //:copy_example_txt_file_path_external
Test-Path-Or-Exit .\ignored_folder\external\faux_repo\example.txt

bazel run --config=test //:remap_paths_example
Test-Path-Or-Exit .\ignored_folder\remapped_dir\example.txt

bazel run --config=test //:link_remap_paths_example
Test-Path-Or-Exit .\ignored_folder\link_remapped_dir\example.txt

bazel run --config=test //:excludes_example
Test-Path-Or-Exit .\ignored_folder\exampledir\example.txt
if (Test-Path -Path .\ignored_folder\info.txt) {
    Write-Error "Test FAILED: info.txt was not excluded"
    exit 1
}

bazel run --config=test //:link_excludes_example
Test-Path-Or-Exit .\ignored_folder\link_excludes_example.txt
if (Test-Path -Path .\ignored_folder\link_excludes_info.txt) {
    Write-Error "Test FAILED: link_excludes_info.txt was not excluded"
    exit 1
}
