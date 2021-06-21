cmake_minimum_required(VERSION 3.8)

set(lib_quick_mail_src
"quickmail.h"
"smtpsocket.h"
"quickmail.c"
"smtpsocket.c"
)

add_library(LIBQUICKMAIL STATIC ${lib_quick_mail_src})
set_target_properties(LIBQUICKMAIL PROPERTIES OUTPUT_NAME "quickmail")

# Windows
if (WIN32)
target_link_directories(LIBQUICKMAIL
    PUBLIC
    "${PROJECT_SOURCE_DIR}/../curl/windows"
)
target_link_libraries(LIBQUICKMAIL
    "libcurl.lib"
    "ws2_32.lib"
)
endif (WIN32)

# Linux
if (UNIX)
find_package( CURL REQUIRED )
target_link_libraries(LIBQUICKMAIL
    CURL::libcurl
)
endif (UNIX)