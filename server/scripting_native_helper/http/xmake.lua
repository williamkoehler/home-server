target("server-home-scripting-native")
    set_kind("static")
    add_packages(
        "spdlog", 
        "openssl", 
        "boost", 
    )

    add_deps(
        "server-common",
    )