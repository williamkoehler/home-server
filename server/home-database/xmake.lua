target("server-home-database")
    set_kind("shared")
    add_files("./**.cpp")
    add_packages(
        "spdlog", 
        "openssl", 
        "boost", 
        "rapidjson", 
        "robin-hood-hashing", 
        "xxhash",
        "sqlite3",
        "cppcodec"
    )

    add_deps(
        "server-home-common"
    )