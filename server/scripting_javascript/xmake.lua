target("server-scripting-javascript")
    set_kind("shared")
    set_basename("js-scripting")
    add_files("./**.cpp")
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
        "server-scripting"
    )