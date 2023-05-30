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
            "/usr/include/julia/",
            PathVariable.PathIsDir,
        ),
        PathVariable(
            "julia_lib",
            "Directory containing libjulia.so",
            "/usr/lib/x86_64-linux-gnu/",
            PathVariable.PathIsDir,
        ),
    ]
