# bzlws

Rules for manipulating your Bazel workspace

## Install

Add to your `MODULE.bazel` file:

```python
bazel_dep(name = "bzlws")
git_override(
    name = "bzlws",
    remote = "https://github.com/zaucy/bzlws.git",
    commit = "80c1bb3227579e134cdb8bdcfc245581fbfcd566",
)
```

On windows runfiles aren't enabled by default. `bzlws` needs runfiles in order to work. Enable them by adding this to your `.bazelrc`:

```bazelrc
build --enable_runfiles
```

See [Documentation](docs/README.md) for usage

## License

MIT
