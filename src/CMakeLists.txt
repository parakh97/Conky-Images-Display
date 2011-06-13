SET (BIN_NAME cid)

add_subdirectory (backends)
add_subdirectory (gui)
add_subdirectory (tools)

SET(cid_SRCS
    cid.c                   cid.h
    cid-animation.c         cid-animation.h
    cid-animation-thread.c  cid-animation-thread.h
    cid-asynchrone.c        cid-asynchrone.h
    cid-callbacks.c         cid-callbacks.h
    cid-config.c            cid-config.h
    cid-cover.c             cid-cover.h
    cid-draw.c              cid-draw.h
    cid-messages.c          cid-messages.h
    cid-modules.c           cid-modules.h
    cid-applet-canvas.h
    cid-constantes.h
    cid-struct.h
)


include_directories(
    ${PACKAGE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/backends
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/tools
)

# Make sure the linker can find the libraries.
link_directories(
    ${PACKAGE_LIBRARY_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/backends
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/tools
)

# Add executable that is built from the source files.
add_executable (${PROJECT_NAME}
    ${cid_SRCS})

# Link the executable to the librairies.
target_link_libraries (${PROJECT_NAME}
    ${PACKAGE_LIBRARIES}
    ${LIBCRYPT_LIBS}
    backends
    gui
    tools
)

# install the program once it is built.
install(
    TARGETS ${BIN_NAME}
    DESTINATION bin)


