
SET(gui_SRC
    cid-callbacks.c     cid-callbacks.h
    cid-gui-callback.c  cid-gui-callback.h
    cid-gui-factory.c   cid-gui-factory.h
    cid-menu-factory.c  cid-menu-factory.h)

add_library(gui STATIC ${gui_SRC})

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
