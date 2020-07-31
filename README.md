# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-3779c423cf41c358fb8257e4545ae37d91379e1f",
    url = "https://github.com/zaucy/bzlws/archive/3779c423cf41c358fb8257e4545ae37d91379e1f.zip",
    sha256 = "fc146f2f2a8b4ac8aeaf02887c8420b544f9d02f2a8f1f059f6bb35ec25d2113",
)
```

## License

MIT
