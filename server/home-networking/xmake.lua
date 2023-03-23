target("server-home-networking")
    set_kind("shared")
    add_files("./**.cpp")
    add_packages(
        "spdlog", 
        "openssl", 
        "boost", 
        "rapidjson", 
        "robin-hood-hashing", 
        "xxhash",
        "cppcodec",
        "stb"
    )

    add_deps(
        "server-home-common",
        "server-home-database",
        "server-home-scripting",
        "server-home-main",
        "server-home-users"
    )