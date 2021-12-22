cmake_minimum_required(VERSION 3.8)

set(lib_quick_mail_src
"quickmail.h"
"smtpsocket.h"
"quickmail.c"
"smtpsocket.c"
)

add_library(libquickmail STATIC ${lib_quick_mail_src})
set_target_properties(libquickmail PROPERTIES OUTPUT_NAME "quickmail")

find_package( CURL REQUIRED )
target_link_libraries(libquickmail
    CURL::libcurl
)