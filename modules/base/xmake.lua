target("SkrBase")
    set_group("01.modules")
    set_exceptions("no-cxx")
    add_rules("skr.static_module", {api = "SKR_BASE"})
    add_deps("SkrCompileFlags", {public = true})
    add_includedirs("include", {public = true})
    add_files("src/build.*.c")