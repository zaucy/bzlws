# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-d0cd8ebfc77149ce1af53358421d17d35e84a4ad",
    url = "https://github.com/zaucy/bzlws/archive/d0cd8ebfc77149ce1af53358421d17d35e84a4ad.zip",
    sha256 = "cb66427258a5d97f73a3e77e7e04d83604f4a73fd9112fa02a8f6ff60e2ef124",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
