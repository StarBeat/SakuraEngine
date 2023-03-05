target("OpenGUI_DemoResources")
    set_kind("static")
    add_rules("utils.install-resources", {
        extensions = {".png"},
        outdir = "/../resources/OpenGUI", 
        rootdir = os.curdir().."/resources"})
    add_files("resources/**.png")
    add_files("resources/dummy.c")

includes("gdi/xmake.lua")