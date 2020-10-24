# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-f2a5eaf83de60e37386c4c9d6f56736afbfbc8c5",
    url = "https://github.com/zaucy/bzlws/archive/f2a5eaf83de60e37386c4c9d6f56736afbfbc8c5.zip",
    sha256 = "d540922e25483d9261541f14fff986853251a949cb50d4d6f8b6a5f28a355d35",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
