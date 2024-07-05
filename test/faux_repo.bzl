"""
"""

def _faux_repo(rctx):
    rctx.file("example.txt", "example.txt")
    rctx.file("BUILD.bazel", "exports_files([\"example.txt\"], visibility = [\"//visibility:public\"])")

faux_repo = repository_rule(
    implementation = _faux_repo,
    attrs = {},
)
