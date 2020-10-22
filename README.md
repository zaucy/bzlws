# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-bfd19cc5d1d85fc7c45ea1c186824bd38abdcab4",
    url = "https://github.com/zaucy/bzlws/archive/bfd19cc5d1d85fc7c45ea1c186824bd38abdcab4.zip",
    sha256 = "e77b287efd27508e27bc3c11ccbddb57a270dcdb59e2b5e105a52a71c24c510f",
)

load("@bzlws//:index.bzl", "bzlws_deps")
bzlws_deps()

```

## License

MIT
