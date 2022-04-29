if (os.host() =="macosx") then 
    import("lib.detect.find_tool")
    local brew = find_tool("brew")
    if(brew == nil) then
        os.runv("/bin/bash", {"-c", "\"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""})
    end
    os.exec("brew install sdl2")
    os.exec("brew install grpc")
    os.exec("brew install googletest")
end
import("find_sdk")

find_sdk.sdk_from_github("SourceSansPro-Regular.ttf")
if (os.host() == "windows") then
    find_sdk.sdk_from_github("wasm-clang-windows-x64.zip")
    find_sdk.sdk_from_github("WinPixEventRuntime-windows-x64.zip")
    find_sdk.sdk_from_github("amdags-windows-x64.zip")
    find_sdk.sdk_from_github("dxc-windows-x64.zip")
    find_sdk.sdk_from_github("grpc-windows-x64.zip")
    find_sdk.sdk_from_github("grpcc-windows-x64.zip")
    find_sdk.sdk_from_github("grpc_d-windows-x64.zip")
    find_sdk.sdk_from_github("m3-windows-x64.zip")
    find_sdk.sdk_from_github("m3_d-windows-x64.zip")
    find_sdk.sdk_from_github("nvapi-windows-x64.zip")
    find_sdk.sdk_from_github("reflector-windows-x64.zip")
    find_sdk.sdk_from_github("SDL2-windows-x64.zip")
    find_sdk.sdk_from_github("tracyclient-windows-x64.zip")
    find_sdk.sdk_from_github("tracyclient_d-windows-x64.zip")
end

if (os.host() == "macosx") then
    if (os.arch() == "x86_64") then
        find_sdk.sdk_from_github("dxc-macosx-x86_64.zip")
        find_sdk.sdk_from_github("grpc-macosx-x86_64.zip")
        find_sdk.sdk_from_github("m3-macosx-x86_64.zip")
        find_sdk.sdk_from_github("m3_d-macosx-x86_64.zip")
        find_sdk.sdk_from_github("reflector-macosx-x86_64.zip")
        find_sdk.sdk_from_github("tracyclient-macosx-x86_64.zip")
    else
        find_sdk.sdk_from_github("dxc-macosx-arm64.zip")
        find_sdk.sdk_from_github("m3-macosx-arm64.zip")
        find_sdk.sdk_from_github("m3_d-macosx-arm64.zip")
    end
end

find_sdk.install_tool("dxc")
find_sdk.install_tool("reflector")
if (os.host() == "windows") then
    find_sdk.install_tool("wasm-clang")
    find_sdk.install_tool("grpcc")
end