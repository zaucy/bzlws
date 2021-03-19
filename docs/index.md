<!-- Generated with Stardoc: http://skydoc.bazel.build -->

<a id="#bzlws_copy"></a>

## bzlws_copy

<pre>
bzlws_copy(<a href="#bzlws_copy-name">name</a>, <a href="#bzlws_copy-srcs">srcs</a>, <a href="#bzlws_copy-out">out</a>, <a href="#bzlws_copy-force">force</a>, <a href="#bzlws_copy-strip_filepath_prefix">strip_filepath_prefix</a>, <a href="#bzlws_copy-metafile_path">metafile_path</a>, <a href="#bzlws_copy-substitutions">substitutions</a>,
           <a href="#bzlws_copy-stamp_substitutions">stamp_substitutions</a>, <a href="#bzlws_copy-visibility">visibility</a>, <a href="#bzlws_copy-kwargs">kwargs</a>)
</pre>

Copy generated files into workspace directory

```python
load("@bzlws//:index.bzl", "bzlws_copy")
```


**PARAMETERS**


| Name  | Description | Default Value |
| :------------- | :------------- | :------------- |
| <a id="bzlws_copy-name"></a>name |  Name used for executable target   |  <code>None</code> |
| <a id="bzlws_copy-srcs"></a>srcs |  List of files that should be copied   |  <code>None</code> |
| <a id="bzlws_copy-out"></a>out |  Output path within the workspace. Certain strings get replaced with     workspace status values and information about the <code>srcs</code>. This     happens in 2 phases.<br><br>    **Phase 1:**<br><br>    The [workspace status](https://docs.bazel.build/versions/master/user-manual.html#workspace_status)     stamp values get replaced in this format: <code>{KEY}</code>. For example if     you would like to have the name of the host machine in your output     path you would put <code>out = "my/path/{BUILD_HOST}/{FILENAME}"</code><br><br>    **Phase 2:**<br><br>    The following gets replaced about each items in <code>srcs</code><br><br>    <code>{BAZEL_LABEL_NAME}</code> - Label name<br><br>    <code>{BAZEL_LABEL_PACKAGE}</code> - Label package<br><br>    <code>{BAZEL_LABEL_WORKSPACE_NAME}</code>  - Workspace name of the label<br><br>    <code>{BAZEL_FULL_LABEL}</code> - Fulll label string<br><br>    <code>{BAZEL_LABEL}</code> - Full label without the workspace name<br><br>    <code>{EXT}</code> - File extension (with the dot)<br><br>    <code>{EXTNAME}</code> - File extension name (without the dot)<br><br>    <code>{FILENAME}</code> - Full file name with extension<br><br>    <code>{FILEPATH}</code> - Fulle file path. Any relative paths are stripped<br><br>    <code>{BASENAME}</code> - Path basename   |  <code>None</code> |
| <a id="bzlws_copy-force"></a>force |  Overwrite existing paths even if they are not files   |  <code>None</code> |
| <a id="bzlws_copy-strip_filepath_prefix"></a>strip_filepath_prefix |  Strip prefix of <code>{FILEPATH}</code>   |  <code>""</code> |
| <a id="bzlws_copy-metafile_path"></a>metafile_path |  Path to metafile   |  <code>""</code> |
| <a id="bzlws_copy-substitutions"></a>substitutions |  BzlwsInfo label keyed, string valued, dictionary. The     values will be replaced in the source files with the values from the     <code>bazel info</code> command. The available BzlwsInfo targets are in the     <code>@bzlws//info</code> package.   |  <code>{}</code> |
| <a id="bzlws_copy-stamp_substitutions"></a>stamp_substitutions |  Workspace status keyed, string valued, dictionary.     The values will be replaced in the sources files the values from the     workspace status matching the key.   |  <code>{}</code> |
| <a id="bzlws_copy-visibility"></a>visibility |  visibility of the executable target   |  <code>None</code> |
| <a id="bzlws_copy-kwargs"></a>kwargs |  rest of arguments get passed to underlying targets   |  none |


<a id="#bzlws_link"></a>

## bzlws_link

<pre>
bzlws_link(<a href="#bzlws_link-name">name</a>, <a href="#bzlws_link-srcs">srcs</a>, <a href="#bzlws_link-out">out</a>, <a href="#bzlws_link-force">force</a>, <a href="#bzlws_link-strip_filepath_prefix">strip_filepath_prefix</a>, <a href="#bzlws_link-metafile_path">metafile_path</a>, <a href="#bzlws_link-visibility">visibility</a>, <a href="#bzlws_link-kwargs">kwargs</a>)
</pre>

 Symlink generated files into workspace directory

```python
load("@bzlws//:index.bzl", "bzlws_link")
```


**PARAMETERS**


| Name  | Description | Default Value |
| :------------- | :------------- | :------------- |
| <a id="bzlws_link-name"></a>name |  Name used for executable target   |  <code>None</code> |
| <a id="bzlws_link-srcs"></a>srcs |  List of files that should be symlinked   |  <code>None</code> |
| <a id="bzlws_link-out"></a>out |  Output path within the workspace. Certain strings get replaced with     workspace status values and information about the <code>srcs</code>. This     happens in 2 phases.<br><br>    **Phase 1:**<br><br>    The [workspace status](https://docs.bazel.build/versions/master/user-manual.html#workspace_status)     stamp values get replaced in this format: <code>{KEY}</code>. For example if     you would like to have the name of the host machine in your output     path you would put <code>out = "my/path/{BUILD_HOST}/{FILENAME}"</code><br><br>    **Phase 2:**<br><br>    The following gets replaced about each items in <code>srcs</code><br><br>    <code>{BAZEL_LABEL_NAME}</code> - Label name<br><br>    <code>{BAZEL_LABEL_PACKAGE}</code> - Label package<br><br>    <code>{BAZEL_LABEL_WORKSPACE_NAME}</code>  - Workspace name of the label<br><br>    <code>{BAZEL_FULL_LABEL}</code> - Fulll label string<br><br>    <code>{BAZEL_LABEL}</code> - Full label without the workspace name<br><br>    <code>{EXT}</code> - File extension (with the dot)<br><br>    <code>{EXTNAME}</code> - File extension name (without the dot)<br><br>    <code>{FILENAME}</code> - Full file name with extension<br><br>    <code>{FILEPATH}</code> - Fulle file path. Any relative paths are stripped<br><br>    <code>{BASENAME}</code> - Path basename   |  <code>None</code> |
| <a id="bzlws_link-force"></a>force |  Overwrite existing paths even if they are not symlinks   |  <code>None</code> |
| <a id="bzlws_link-strip_filepath_prefix"></a>strip_filepath_prefix |  Strip prefix of <code>{FILEPATH}</code>   |  <code>""</code> |
| <a id="bzlws_link-metafile_path"></a>metafile_path |  Path to metafile   |  <code>""</code> |
| <a id="bzlws_link-visibility"></a>visibility |  visibility of the executable target   |  <code>None</code> |
| <a id="bzlws_link-kwargs"></a>kwargs |  rest of arguments get passed to underlying targets   |  none |


