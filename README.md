# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-016675ffb772aa74820ee8427a4968fe0284df93",
    url = "https://github.com/zaucy/bzlws/archive/016675ffb772aa74820ee8427a4968fe0284df93.zip",
    sha256 = "4e3059d5c813dcd7c7af993b2b10610e447f016ced81ee313d15d8a0ec6bc483",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
