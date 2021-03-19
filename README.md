# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-dc415bdd14c30f48690ead5a9bd0eaf8cd63d856",
    url = "https://github.com/zaucy/bzlws/archive/dc415bdd14c30f48690ead5a9bd0eaf8cd63d856.zip",
    sha256 = "fe192bec51557d8f4d3302ea9eb2e7e463ee937d25f1e00cad1b16baa323a7b7",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
