<!-- Generated with Stardoc: http://skydoc.bazel.build -->



<a id="bzlws_link"></a>

## bzlws_link

<pre>
load("@bzlws//rules:bzlws_link.bzl", "bzlws_link")

bzlws_link(<a href="#bzlws_link-name">name</a>, <a href="#bzlws_link-srcs">srcs</a>, <a href="#bzlws_link-out">out</a>, <a href="#bzlws_link-force">force</a>, <a href="#bzlws_link-strip_filepath_prefix">strip_filepath_prefix</a>, <a href="#bzlws_link-metafile_path">metafile_path</a>, <a href="#bzlws_link-visibility">visibility</a>, <a href="#bzlws_link-kwargs">kwargs</a>)
</pre>

Symlink generated files into workspace directory

**PARAMETERS**


| Name  | Description | Default Value |
| :------------- | :------------- | :------------- |
| <a id="bzlws_link-name"></a>name |  Name used for executable target   |  `None` |
| <a id="bzlws_link-srcs"></a>srcs |  List of files that should be symlinked   |  `None` |
| <a id="bzlws_link-out"></a>out |  Output path within the workspace. Certain strings get replaced with workspace status values and information about the `srcs`. This happens in 2 phases.<br><br>**Phase 1:**<br><br>The [workspace status](https://docs.bazel.build/versions/master/user-manual.html#workspace_status) stamp values get replaced in this format: `{KEY}`. For example if you would like to have the name of the host machine in your output path you would put `out = "my/path/{BUILD_HOST}/{FILENAME}"`<br><br>**Phase 2:**<br><br>The following gets replaced about each items in `srcs`<br><br>`{BAZEL_LABEL_NAME}` - Label name<br><br>`{BAZEL_LABEL_PACKAGE}` - Label package<br><br>`{BAZEL_LABEL_WORKSPACE_NAME}`  - Workspace name of the label<br><br>`{BAZEL_FULL_LABEL}` - Fulll label string<br><br>`{BAZEL_LABEL}` - Full label without the workspace name<br><br>`{EXT}` - File extension (with the dot)<br><br>`{EXTNAME}` - File extension name (without the dot)<br><br>`{FILENAME}` - File name with extension<br><br>`{FILEPATH}` - File path. https://bazel.build/rules/lib/File#path<br><br>`{BASENAME}` - Path basename   |  `None` |
| <a id="bzlws_link-force"></a>force |  Overwrite existing paths even if they are not symlinks   |  `None` |
| <a id="bzlws_link-strip_filepath_prefix"></a>strip_filepath_prefix |  Strip prefix of `{FILEPATH}`   |  `""` |
| <a id="bzlws_link-metafile_path"></a>metafile_path |  Path to metafile   |  `""` |
| <a id="bzlws_link-visibility"></a>visibility |  visibility of the executable target   |  `None` |
| <a id="bzlws_link-kwargs"></a>kwargs |  rest of arguments get passed to underlying targets   |  none |


