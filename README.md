# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-81d564c831ff92c3678ef8f0d2f2ebbb0f18566a",
    url = "https://github.com/zaucy/bzlws/archive/81d564c831ff92c3678ef8f0d2f2ebbb0f18566a.zip",
    sha256 = "c818beae95a3dff26549ecabb1b67748fb1dbe36a2102896723077f2939b2d40",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
