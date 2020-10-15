# bzlws

Rules for manipulating your Bazel workspace

## Install

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-fb00a201627acc197564a42d4c7e4b768a662037",
    url = "https://github.com/zaucy/bzlws/archive/fb00a201627acc197564a42d4c7e4b768a662037.zip",
    sha256 = "ad96968f9534b9146d72d45902f50eae75ff1418cc481da556b12a6910635fb6",
)
```

## License

MIT
