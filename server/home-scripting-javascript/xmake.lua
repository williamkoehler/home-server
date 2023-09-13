target("server-home-scripting-javascript")
    set_kind("shared")
    set_basename("home-js-scripting")
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
        "server-home-database",
        "server-home-scripting"
    )