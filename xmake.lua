add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

add_requires("conan::spdlog/1.11.0", { 
    alias = "spdlog",
    configs = {
        settings = {
            "spdlog:compiler=gcc",
            "spdlog:compiler.version=10",
            "spdlog:compiler.libcxx=libstdc++11",
            "fmt:compiler=gcc",
            "fmt:compiler.version=10",
            "fmt:compiler.libcxx=libstdc++11",
        },
        options = {
            "spdlog:no_exceptions=True",
        }
    }
})
add_requires("conan::openssl/1.1.1m", { alias = "openssl" })
add_requires("conan::boost/1.80.0", { 
    alias = "boost",
    configs = {
        settings = {
            "boost:compiler=gcc",
            "boost:compiler.version=10",
            "boost:compiler.libcxx=libstdc++11",
        },
        options = {
            "boost:zlib=True",
            "boost:bzip2=True",
            "boost:numa=False",
            "boost:without_atomic=False",
            "boost:without_chrono=False",
            "boost:without_container=False",
            "boost:without_context=True",
            "boost:without_contract=True",
            "boost:without_coroutine=True",
            "boost:without_date_time=False",
            "boost:without_exception=False",
            "boost:without_fiber=True",
            "boost:without_filesystem=False",
            "boost:without_graph=True",
            "boost:without_graph_parallel=True",
            "boost:without_iostreams=True",
            "boost:without_json=False",
            "boost:without_locale=True",
            "boost:without_log=True",
            "boost:without_math=True",
            "boost:without_mpi=True",
            "boost:without_nowide=True",
            "boost:without_program_options=True",
            "boost:without_python=True",
            "boost:without_random=True",
            "boost:without_regex=True",
            "boost:without_serialization=True",
            "boost:without_stacktrace=True",
            "boost:without_system=False",
            "boost:without_test=True",
            "boost:without_thread=False",
            "boost:without_timer=True",
            "boost:without_type_erasure=True",
            "boost:without_wave=True",
        }
    }
})
add_requires("conan::rapidjson/cci.20220822", { alias = "rapidjson" })
add_requires("conan::sqlite3/3.39.4", { alias = "sqlite3" })
add_requires("conan::robin-hood-hashing/3.11.5", { alias = "robin-hood-hashing" })
add_requires("conan::xxhash/0.8.1", { alias = "xxhash" })
add_requires("conan::cppcodec/0.2", { alias = "cppcodec" })
add_requires("conan::jwt-cpp/0.6.0", { alias = "jwt-cpp" })
add_requires("conan::stb/cci.20210910", { alias = "stb" })

-- "-fsanitize=thread", 
add_cxxflags("-fPIC")
add_cxxflags("-Wall", "-Wextra", "-Wpedantic")

if is_mode("debug") then
    add_cxxflags("-fstandalone-debug")
    set_optimize("none")
    add_defines("DEBUG")
else
    add_defines("NDEBUG")
end

set_languages("cxx20")

add_includedirs("server")

includes("server")