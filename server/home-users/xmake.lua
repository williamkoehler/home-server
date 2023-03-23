target("server-home-users")
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
        "jwt-cpp"
    )

    add_deps(
        "server-home-common",
        "server-home-database"
    )