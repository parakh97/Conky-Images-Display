
########### install files ###############

install (FILES
    animation.png
    connect.png
    cross.png
    default.svg
    disconnect.png
    options.png
    DESTINATION ${pkgdatadir})

install (FILES
	${CMAKE_CURRENT_BINARY_DIR}/cid.conf
	DESTINATION ${pkgdatadir})

# appli
install (FILES
	cid.svg
	DESTINATION ${datadir}/pixmaps)

install (FILES
	cid.desktop
	DESTINATION ${datadir}/applications)
