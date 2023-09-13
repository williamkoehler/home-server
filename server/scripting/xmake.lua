target("server-scripting")
    set_kind("shared")
    set_basename("scripting")
    add_files("./**.cpp")
    set_pcxxheader("common.hpp")
    add_packages(
        "spdlog", 
        "boost", 
        "rapidjson", 
        "robin-hood-hashing", 
        "xxhash"
    )

    add_deps(
        "server-common",
        "server-database",
        "server-api"
    )