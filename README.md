# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-0177afcbe9fd9026e008f22dde45af64aea2f30c",
    url = "https://github.com/zaucy/bzlws/archive/0177afcbe9fd9026e008f22dde45af64aea2f30c.zip",
    sha256 = "a6a560788f32d0fdc37327d24a68eebeb0d2aef8adc1e82c6c388e032f3c47ed",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
