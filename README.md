# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-514741f7e984b83ed97ad6edf316009c0c6b616b",
    url = "https://github.com/zaucy/bzlws/archive/514741f7e984b83ed97ad6edf316009c0c6b616b.zip",
    sha256 = "290489baad8a498884829d346cf20379ec1956f7aaff12ec342ae508ca90d2ea",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
