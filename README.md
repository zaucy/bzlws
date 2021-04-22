# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-722c0333fbae81192b950b788a37ed8ff737b900",
    url = "https://github.com/zaucy/bzlws/archive/722c0333fbae81192b950b788a37ed8ff737b900.zip",
    sha256 = "028caea91de46b64ef6379b86fe8b2014e71280914e7208c924be3f4441ccc8e",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
