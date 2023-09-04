target("AlgoTest")
    set_kind("binary")
    set_group("05.tests/base")
    public_dependency("SkrBase", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("algo/*.cpp")

target("ContainersTest")
    set_kind("binary")
    set_group("05.tests/base")
    public_dependency("SkrBase", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("containers/*.cpp")