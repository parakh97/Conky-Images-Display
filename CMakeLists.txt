########### requirements ###############

cmake_minimum_required (VERSION 2.6)
find_package (PkgConfig)
include (CheckLibraryExists)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckSymbolExists)

############ sources tarball ############

#set (CPACK_SOURCE_GENERATOR "TGZ")
#set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${VERSION}")
#set (CPACK_SOURCE_IGNORE_FILES 
#	"/build/;/.bzr/;bzrignore$;/config.h$;/gldi-config.h$;/scripts/;/doc/;/misc/;~$;${CPACK_SOURCE_IGNORE_FILES}")
#include (CPack)

#add_custom_target( dist
#	COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
#add_custom_target(dist-bzr
#	COMMAND bzr export ${CMAKE_PROJECT_NAME}-${VERSION}.tar.gz
#	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

########### global variables ###############

set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE ${PACKAGE})
set (prefix ${CMAKE_INSTALL_PREFIX})
set (exec_prefix ${prefix})
set (datadir ${prefix}/share)
set (pkgdatadir ${datadir}/cid)
set (mandir ${datadir}/man)

if( CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT "${FORCE_LIB64}" STREQUAL "")  # 64bits and force install in lib64
	set (libdir ${prefix}/lib64)
else()
	set (libdir ${prefix}/lib${LIB_SUFFIX})
endif()

set (includedir ${prefix}/include)
set (bindir ${prefix}/bin)

########### project ###############

project ("cid")
set (VERSION "1.1.6-dev")

add_definitions (-W -Werror -fno-builtin -g) #-Wunreachable-code -Wno-unused-parameter -Wall 
if (NOT ${CMAKE_BUILD_TYPE})
	add_definitions (-O3)
endif()
add_definitions (-DCID_DEFAULT_IMAGE="${pkgdatadir}/default.svg")
add_definitions (-DCID_CONFIG_FILE="cid.conf")
add_definitions (-DCID_MODULES_DIR="${libdir}/${PACKAGE}")
if (${HAVE_COMPIZ})
    add_definitions (-DHAVE_COMPIZ=1)
    set (USE_COMPIZ 1)
endif()
add_custom_target (uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

########### dependencies ###############

pkg_check_modules ("PACKAGE" REQUIRED "gtk+-2.0" "dbus-1" "dbus-glib-1" "libxml-2.0" "gthread-2.0" "libcurl" "x11" "cairo" "libcrypto")

#pkg_check_modules ("XEXTEND" "xtst" "xcomposite" "xinerama")
#if (${XEXTEND_FOUND})
#	set (HAVE_XEXTEND 1)
#	set (xextend_required "xtst xcomposite xinerama")  # pour le .pc
#endif()

# We use crypt(3) which may be in libc, or in libcrypt (eg FreeBSD)
#check_library_exists (crypt encrypt "" HAVE_LIBCRYPT)
#if (${HAVE_LIBCRYPT})
#	set (LIBCRYPT_LIBS "-lcrypt")
#endif()

check_include_files ("math.h" HAVE_MATH_H)
check_symbol_exists (LC_MESSAGES "locale.h" HAVE_LC_MESSAGES)
CHECK_LIBRARY_EXISTS (m sin "" HAVE_LIBM)

MESSAGE (STATUS)
MESSAGE (STATUS "C.I.D. will be compiled with the following options :")
if (${USE_COMPIZ})
    MESSAGE (STATUS " * Use Compiz support : yes")
else()
    MESSAGE (STATUS " * Use Compiz support : no")
endif()
#if (${XEXTEND_FOUND})
#	MESSAGE (STATUS " * Use X Extensions : yes")
#else()
#	MESSAGE (STATUS " * Use X Extensions : no")
#endif()
#if (${HAVE_LIBCRYPT})
#	MESSAGE (STATUS " * crypt passwords : yes")
#else()
#	MESSAGE (STATUS " * crypt passwords : no")
#endif()
MESSAGE (STATUS)

########### next steps ###############

add_subdirectory (src)
add_subdirectory (data)
add_subdirectory (po)

########### variables defined at compil time ###############

set (CID_DATA_DIR ${pkgdatadir})
set (CID_LOCALE_DIR ${datadir}/locale)
set (CID_GETTEXT_PACKAGE ${GETTEXT_PACKAGE})

########### file generation ###############

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/data/cid.conf.in ${CMAKE_CURRENT_BINARY_DIR}/data/cid.conf)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cid.pc.in ${CMAKE_CURRENT_BINARY_DIR}/cid.pc)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY)

########### install files ###############

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/cid.pc DESTINATION ${libdir}/pkgconfig)
