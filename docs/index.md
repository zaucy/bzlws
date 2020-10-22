<!-- Generated with Stardoc: http://skydoc.bazel.build -->

<a name="#bzlws_copy"></a>

## bzlws_copy

<pre>
bzlws_copy(<a href="#bzlws_copy-name">name</a>, <a href="#bzlws_copy-srcs">srcs</a>, <a href="#bzlws_copy-out">out</a>, <a href="#bzlws_copy-force">force</a>, <a href="#bzlws_copy-metafile_path">metafile_path</a>, <a href="#bzlws_copy-visibility">visibility</a>)
</pre>

Copy generated files into workspace directory

**PARAMETERS**


| Name  | Description | Default Value |
| :-------------: | :-------------: | :-------------: |
| name |  Name used for executable target   |  <code>None</code> |
| srcs |  List of files that should be copied   |  <code>None</code> |
| out |  Output path within the workspace. Certain strings get replaced with      various information about      {BAZEL_LABEL_NAME} - Label name      {BAZEL_LABEL_PACKAGE} - Label package      {BAZEL_LABEL_WORKSPACE_NAME}  - Workspace name of the label      {BAZEL_FULL_LABEL} - Fulll label string      {BAZEL_LABEL} - Full label without the workspace name      {EXT} - File extension (with the dot)      {EXTNAME} - File extension name (without the dot)      {BASENAME} - Path basename   |  <code>None</code> |
| force |  Overwrite existing paths even if they are not files   |  <code>None</code> |
| metafile_path |  Path to metafile   |  <code>""</code> |
| visibility |  visibility of the executable target   |  <code>None</code> |


<a name="#bzlws_deps"></a>

## bzlws_deps

<pre>
bzlws_deps()
</pre>



**PARAMETERS**



<a name="#bzlws_link"></a>

## bzlws_link

<pre>
bzlws_link(<a href="#bzlws_link-name">name</a>, <a href="#bzlws_link-srcs">srcs</a>, <a href="#bzlws_link-out">out</a>, <a href="#bzlws_link-force">force</a>, <a href="#bzlws_link-metafile_path">metafile_path</a>, <a href="#bzlws_link-visibility">visibility</a>)
</pre>

Symlink generated files into workspace directory

**PARAMETERS**


| Name  | Description | Default Value |
| :-------------: | :-------------: | :-------------: |
| name |  Name used for executable target   |  <code>None</code> |
| srcs |  List of files that should be symlinked   |  <code>None</code> |
| out |  Output path within the workspace. Certain strings get replaced with      various information about each source target from <code>srcs</code>      {BAZEL_LABEL_NAME} - Label name      {BAZEL_LABEL_PACKAGE} - Label package      {BAZEL_LABEL_WORKSPACE_NAME}  - Workspace name of the label      {BAZEL_FULL_LABEL} - Fulll label string      {BAZEL_LABEL} - Full label without the workspace name      {EXT} - File extension (with the dot)      {EXTNAME} - File extension name (without the dot)      {BASENAME} - Path basename   |  <code>None</code> |
| force |  Overwrite existing paths even if they are not symlinks   |  <code>None</code> |
| metafile_path |  Path to metafile   |  <code>""</code> |
| visibility |  visibility of the executable target   |  <code>None</code> |


