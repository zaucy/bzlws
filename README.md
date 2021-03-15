# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-557f409ffcb2425b0fc4cb4526b2e2e9fa9601f0",
    url = "https://github.com/zaucy/bzlws/archive/557f409ffcb2425b0fc4cb4526b2e2e9fa9601f0.zip",
    sha256 = "887dd772883d975b766b7224b18f6d288d3723929c300474262b3ca5c8fc8e2c",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
