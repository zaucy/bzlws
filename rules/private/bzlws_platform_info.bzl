BzlwsPlatformInfo = provider(
    doc = "Propagates target platform information for transitioned source files",
    fields = {
        "mapping": "depset of structs, each has: file (File), os (str), cpu (str)",
    },
)
