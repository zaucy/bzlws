# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-03390b445725c495d7a236b69e172e5f445d32fd",
    url = "https://github.com/zaucy/bzlws/archive/03390b445725c495d7a236b69e172e5f445d32fd.zip",
    sha256 = "602b938382a570a4eec2db3c7a1582b70909c3c35fd0f656ce4dd3729cafe27c",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
