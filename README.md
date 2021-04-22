# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-28f2e452668df90f38d65d81bcd55073d6ca3477",
    url = "https://github.com/zaucy/bzlws/archive/28f2e452668df90f38d65d81bcd55073d6ca3477.zip",
    sha256 = "9f5fe7304b53917fdd6fc3ddd6dcf6f55148f4fc617bcd829ff6e4a88b099624",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
