target("server-home-scripting")
    set_kind("shared")
    set_basename("home-scripting")
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
        "server-home-common",
        "server-home-database",
        "server-home-api"
    )