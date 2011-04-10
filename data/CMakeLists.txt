
########### install files ###############

install (FILES
    animation.png
    connect.png
    cross.png
    default.svg
    disconnect.png
    next-red.png
    next-white.png
    next-yellow.png
    options.png
    pause-red.png
    pause-white.png
    pause-yellow.png
    play-red.png
    play-white.png
    play-yellow.png
    prev-red.png
    prev-white.png
    prev-yellow.png
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
