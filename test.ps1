#!/usr/bin/env pwsh

$ErrorActionPreference = 'Stop'

function Test-Path-Or-Exit($Path) {
	if (!(Test-Path -Path $Path)) {
		Write-Error "Test FAILED: '$Path' does not exist"
		exit 1
	}
}

bazel run //example:copy_example_txt
Test-Path-Or-Exit .\example\ignored_folder\example.txt

bazel run //example:copy_example_txt_file_path
Test-Path-Or-Exit .\example\ignored_folder\example\example.txt
