BzlwsInfo = provider()

def _bzlws_info(ctx):
    return BzlwsInfo(name = ctx.attr.name)

# bzlws_info is just a token rule to tell the bzlws tools which bazel
# information should be retrieved
bzlws_info = rule(
    implementation = _bzlws_info,
)
