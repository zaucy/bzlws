# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-7ab46be1c37d60f41977f3155b4425ca5166aa53",
    url = "https://github.com/zaucy/bzlws/archive/7ab46be1c37d60f41977f3155b4425ca5166aa53.zip",
    sha256 = "82d6acf64757a756c14eca246d8133c54afdcf6402aab0af2486ead5e61fa9eb",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()
```

See [Documentation](docs/index.md) for usage

## License

MIT
