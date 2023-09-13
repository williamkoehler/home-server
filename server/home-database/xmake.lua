target("server-home-database")
    set_kind("shared")
    set_basename("home-database")
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
        "server-home-common"
    )