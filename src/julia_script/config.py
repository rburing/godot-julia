def can_build(env, platform):
    return True


def configure(env):
    pass


def get_opts(platform):
    from SCons.Variables import PathVariable

    return [
        PathVariable(
            "julia_include",
            "Directory containing julia.h",
            "",
            PathVariable.PathIsDir,
        ),
        PathVariable(
            "julia_lib",
            "Directory containing libjulia.so",
            "",
            PathVariable.PathIsDir,
        ),
    ]


def get_doc_classes():
    return [
        "JuliaScript",
        "JuliaBindingsGenerator",
    ]


def get_doc_path():
    return "doc_classes"
