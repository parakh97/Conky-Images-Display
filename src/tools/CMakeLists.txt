
SET(tools_SRC
    cid-console-call.c      cid-console-call.h
    cid-datatables.c        cid-datatables.h
    cid-dbus.c              cid-dbus.h
    cid-file-utilities.c    cid-file-utilities.h
    cid-md5.c               cid-md5.h
    cid-string-utilities.c  cid-string-utilities.h
    cid-utilities.c         cid-utilities.h
    cid-X-utilities.c       cid-X-utilities.h)

add_library(tools STATIC ${tools_SRC})

# Make sure the linker can find the libraries.
link_directories(
    ${PACKAGE_LIBRARY_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/backends
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/tools
)

include_directories(
    ${PACKAGE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/backends
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/tools
)

