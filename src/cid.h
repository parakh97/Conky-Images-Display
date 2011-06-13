/***********************************************************************
*
* Program:
*   Conky Images Display
*
* License :
*  This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License, version 2.
*   If you don't know what that means take a look at:
*      http://www.gnu.org/licenses/licenses.html#GPL
*
* Original idea :
*   Charlie MERLAND, July 2008.
*
***********************************************************************/
/*
   *
   *                              cid.h
   *                             -------
   *                       Conky Images Display
   *             --------------------------------------------
   *
*/

#ifndef __CID__
#define  __CID__

#include <signal.h>
#include <pthread.h>
#include <curl/curl.h>

#include "cid-animation.h"
#include "cid-applet-canvas.h"
#include "cid-asynchrone.h"
#include "cid-config.h"
#include "cid-constantes.h"
#include "cid-cover.h"
#include "cid-draw.h"
#include "cid-messages.h"
#include "cid-modules.h"
#include "cid-struct.h"

/* backends */
#include "backends/cid-amarok.h"
#include "backends/cid-amarok2.h"
#include "backends/cid-exaile.h"
#include "backends/cid-mpd.h"
#include "backends/cid-rhythmbox.h"
#include "backends/libmpdclient.h"

/* gui */
#include "cid-callbacks.h"
#include "gui/cid-gui-callback.h"
#include "gui/cid-gui-factory.h"
#include "gui/cid-menu-factory.h"

/* tools */
#include "tools/cid-console-call.h"
#include "tools/cid-datatables.h"
#include "tools/cid-dbus.h"
#include "tools/cid-file-utilities.h"
#include "tools/cid-md5.h"
#include "tools/cid-string-utilities.h"
#include "tools/cid-utilities.h"

#include "config.h"

#endif
