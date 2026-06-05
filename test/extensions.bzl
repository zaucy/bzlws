load("//:faux_repo.bzl", "faux_repo")

def _faux_repo_extension_impl(module_ctx):
    faux_repo(name = "faux_repo")

faux_repo_extension = module_extension(
    implementation = _faux_repo_extension_impl,
)
