#!/usr/bin/env pwsh

$ErrorActionPreference = 'Stop'

function Test-Path-Or-Exit($Path) {
	if (!(Test-Path -Path $Path)) {
		Write-Error "Test FAILED: '$Path' does not exist"
		exit 1
	}
}

git clean -fx example
bazel build //example/...

bazel run --config=test //example:copy_example_txt
Test-Path-Or-Exit .\example\ignored_folder\example.txt

bazel run --config=test //example:copy_example_txt_file_path
Test-Path-Or-Exit .\example\ignored_folder\example\example.txt
