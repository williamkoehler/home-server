target("server-home-scripting")
    set_kind("shared")
    add_files("./**.cpp")
    add_packages(
        "spdlog", 
        "boost", 
        "rapidjson", 
        "robin-hood-hashing", 
        "xxhash"
    )

    add_deps(
        "server-home-common",
        "server-home-database"
    )