
SET(backends_SRC
    cid-amarok2.c   cid-amarok2.h
    cid-amarok.c    cid-amarok.h
    cid-exaile.c    cid-exaile.h
    cid-mpd.c       cid-mpd.h
    cid-rhythmbox.c cid-rhythmbox.h
    libmpdclient.c  libmpdclient.h)

add_library(backends STATIC ${backends_SRC})

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
