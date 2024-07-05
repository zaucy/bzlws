<!-- Generated with Stardoc: http://skydoc.bazel.build -->



<a id="bzlws_copy"></a>

## bzlws_copy

<pre>
load("@bzlws//rules:bzlws_copy.bzl", "bzlws_copy")

bzlws_copy(<a href="#bzlws_copy-name">name</a>, <a href="#bzlws_copy-srcs">srcs</a>, <a href="#bzlws_copy-out">out</a>, <a href="#bzlws_copy-force">force</a>, <a href="#bzlws_copy-strip_filepath_prefix">strip_filepath_prefix</a>, <a href="#bzlws_copy-metafile_path">metafile_path</a>, <a href="#bzlws_copy-substitutions">substitutions</a>,
           <a href="#bzlws_copy-stamp_substitutions">stamp_substitutions</a>, <a href="#bzlws_copy-visibility">visibility</a>, <a href="#bzlws_copy-tags">tags</a>, <a href="#bzlws_copy-kwargs">kwargs</a>)
</pre>

Copy generated files into workspace directory

```python
load("@bzlws//rules:bzlws_copy.bzl", "bzlws_copy")
```


**PARAMETERS**


| Name  | Description | Default Value |
| :------------- | :------------- | :------------- |
| <a id="bzlws_copy-name"></a>name |  Name used for executable target   |  `None` |
| <a id="bzlws_copy-srcs"></a>srcs |  List of files that should be copied   |  `None` |
| <a id="bzlws_copy-out"></a>out |  Output path within the workspace. Certain strings get replaced with workspace status values and information about the `srcs`. This happens in 2 phases.<br><br>**Phase 1:**<br><br>The [workspace status](https://docs.bazel.build/versions/master/user-manual.html#workspace_status) stamp values get replaced in this format: `{KEY}`. For example if you would like to have the name of the host machine in your output path you would put `out = "my/path/{BUILD_HOST}/{FILENAME}"`<br><br>**Phase 2:**<br><br>The following gets replaced about each items in `srcs`<br><br>`{BAZEL_LABEL_NAME}` - Label name<br><br>`{BAZEL_LABEL_PACKAGE}` - Label package<br><br>`{BAZEL_LABEL_WORKSPACE_NAME}`  - Workspace name of the label<br><br>`{BAZEL_FULL_LABEL}` - Fulll label string<br><br>`{BAZEL_LABEL}` - Full label without the workspace name<br><br>`{EXT}` - File extension (with the dot)<br><br>`{EXTNAME}` - File extension name (without the dot)<br><br>`{FILENAME}` - File name with extension<br><br>`{FILEPATH}` - File path. https://bazel.build/rules/lib/File#path<br><br>`{BASENAME}` - Path basename   |  `None` |
| <a id="bzlws_copy-force"></a>force |  Overwrite existing paths even if they are not files   |  `None` |
| <a id="bzlws_copy-strip_filepath_prefix"></a>strip_filepath_prefix |  Strip prefix of `{FILEPATH}`   |  `""` |
| <a id="bzlws_copy-metafile_path"></a>metafile_path |  Path to metafile   |  `""` |
| <a id="bzlws_copy-substitutions"></a>substitutions |  BzlwsInfo label keyed, string valued, dictionary. The values will be replaced in the source files with the values from the `bazel info` command. The available BzlwsInfo targets are in the `@bzlws//info` package.   |  `{}` |
| <a id="bzlws_copy-stamp_substitutions"></a>stamp_substitutions |  Workspace status keyed, string valued, dictionary. The values will be replaced in the sources files the values from the workspace status matching the key.   |  `{}` |
| <a id="bzlws_copy-visibility"></a>visibility |  visibility of the executable target   |  `None` |
| <a id="bzlws_copy-tags"></a>tags |  forwarded to underlying targets   |  `[]` |
| <a id="bzlws_copy-kwargs"></a>kwargs |  rest of arguments get passed to underlying targets   |  none |


