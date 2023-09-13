target("server-home-server")
    set_kind("binary")
    set_basename("home-server")
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
        "server-home-main",
        "server-home-scripting",
        "server-home-scripting-javascript",
        "server-home-scripting-native",
        "server-home-api"
    )