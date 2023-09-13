target("server-database")
    set_kind("shared")
    set_basename("database")
    add_files("./**.cpp")
    set_pcxxheader("common.hpp")
    add_packages(
        "spdlog", 
        "boost", 
        "rapidjson", 
        "robin-hood-hashing", 
        "xxhash",
        "sqlite3",
        "cppcodec"
    )

    add_deps(
        "server-common"
    )