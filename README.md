# bzlws

Rules for bringing bazel artifacts out of the [bazel output directories](https://bazel.build/remote/output-directories) and **into your workspace**.

## Install

Add to your `MODULE.bazel` file:

```python
bazel_dep(name = "bzlws", version = "0.2.0")
```

On windows runfiles aren't enabled by default. `bzlws` needs runfiles in order to work. Enable them by adding this to your `.bazelrc`:

```bazelrc
build --enable_runfiles
```

See [Documentation](docs/README.md) for usage

## License

MIT
