
SET(cid_SRCS
    cid-amarok2.c
    cid-amarok2.h
    cid-amarok.c
    cid-amarok.h
    cid-animation.c
    cid-animation.h
    cid-animation-thread.c
    cid-animation-thread.h
    cid-applet-canvas.h
    cid-asynchrone.c
    cid-asynchrone.h
    cid.c
    cid-callbacks.c
    cid-callbacks.h
    cid-config.c
    cid-config.h
    cid-console-call.c
    cid-console-call.h
    cid-constantes.h
    cid-cover.c
    cid-cover.h
    cid-datatables.c
    cid-datatables.h
    cid-dbus.c
    cid-dbus.h
    cid-draw.c
    cid-draw.h
    cid-exaile.c
    cid-exaile.h
    cid-file-utilities.c
    cid-file-utilities.h
    cid-gui-callback.c
    cid-gui-callback.h
    cid-gui-factory.c
    cid-gui-factory.h
    cid.h
    cid-md5.c
    cid-md5.h
    cid-menu-factory.c
    cid-menu-factory.h
    cid-messages.c
    cid-messages.h
    cid-modules.c
    cid-modules.h
    cid-mpd.c
    cid-mpd.h
    cid-rhythmbox.c
    cid-rhythmbox.h
    cid-string-utilities.c
    cid-string-utilities.h
    cid-struct.h
    cid-utilities.c
    cid-utilities.h
    cid-X-utilities.c
    cid-X-utilities.h
    libmpdclient.c
    libmpdclient.h
)


include_directories(
	${PACKAGE_INCLUDE_DIRS}
	${XEXTEND_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src
)

# Make sure the linker can find the libraries.
link_directories(
	${PACKAGE_LIBRARY_DIRS}
	${XEXTEND_LIBRARY_DIRS}
)

# Add executable that is built from the source files.
add_executable (${PROJECT_NAME}
	${cid_SRCS}
	cid.c)

# Link the executable to the librairies.
target_link_libraries (${PROJECT_NAME}
	${PACKAGE_LIBRARIES}
	${XEXTEND_LIBRARIES}
	${LIBCRYPT_LIBS}
)

# install the program once it is built.
install(
	TARGETS ${PACKAGE}
	DESTINATION bin)


