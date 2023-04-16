target("server-home-main")
    set_kind("shared")
    add_files("./**.cpp")
    add_packages(
        "spdlog", 
        "openssl", 
        "boost", 
        "rapidjson", 
        "robin-hood-hashing", 
        "xxhash"
    )

    add_deps(
        "server-home-common",
        "server-home-database",
        "server-home-scripting",
        "server-home-api"
    )