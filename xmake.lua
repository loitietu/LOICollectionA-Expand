add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("groupmountain-repo https://github.com/GroupMountain/xmake-repo.git")

set_xmakever("3.0.0")

package("LOICollectionA")
    add_urls("https://github.com/loitietu/LOICollectionA.git")
    add_versions("1.9.0", "add868afcaf4314c561a029338d73bff343597fb")

    on_install(function (package)
        import("package.tools.xmake").install(package)
    end)
package_end()

add_requires("levilamina 1.7.3", {configs = {target_type = "server"}})
add_requires("LOICollectionA 1.9.0")
add_requires("gmlib 1.7.0")
add_requires(
    "levibuildscript",
    "nlohmann_json 3.12.0"
)

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

set_version("1.0.0")

target("LOICollectionA-Expand")
    add_rules("@levibuildscript/linkrule")
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/permissive-",
        "/Ob3",
        "/W4",
        "/w44265",
        "/w44289",
        "/w44296",
        "/w45263",
        "/w44738",
        "/w45204"
    )
    add_defines(
        "NOMINMAX",
        "UNICODE",
        "_HAS_CXX23=1"
    )
    add_files(
        "src/plugin/**.cpp"
    )
    add_includedirs(
        "src/plugin"
    )
    add_packages(
        "levilamina",
        "LOICollectionA",
        "gmlib"
    )
    set_exceptions("none")
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")

    if is_mode("debug") then
        add_defines("DEBUG")
    elseif is_mode("release") then
        add_defines("NDEBUG")
    end

    after_build(function (target)
        local plugin_packer = import("scripts.after_build")

        local major, minor, patch = target:version():match("(%d+)%.(%d+)%.(%d+)")
        local plugin_define = {
            pluginName = target:name(),
            pluginFile = path.filename(target:targetfile()),
            pluginVersion = major .. "." .. minor .. "." .. patch,
        }
        
        plugin_packer.pack_plugin(target, plugin_define)
    end)
