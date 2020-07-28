# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-dcb09d3a1feaa50188da8c39d6392650ceb5df50",
    url = "https://github.com/zaucy/bzlws/archive/dcb09d3a1feaa50188da8c39d6392650ceb5df50.zip",
    sha256 = "d0eca0c55aa19de16ff46c286c90426898d65da36e134cddbb7154b29d8d4514",
)
```

## License

MIT
