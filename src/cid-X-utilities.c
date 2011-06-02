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
   *                  cid-X-utilities.c
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/

#include "cid-messages.h"
#include "cid-struct.h"

#include <X11/Xlib.h>

extern CidMainContainer *cid;

static Display *s_XDisplay = NULL;

static int 
_cid_xerror_handler (Display * pDisplay, XErrorEvent *pXError) 
{
    cid_debug ("Erreur (%d, %d, %d) lors d'une requete X sur %d", pXError->error_code, pXError->request_code, pXError->minor_code, pXError->resourceid);
    return 0;
}

void 
cid_get_X_infos (void) 
{
    s_XDisplay = XOpenDisplay (0);
    
    XSetErrorHandler (_cid_xerror_handler);
    
    Screen *XScreen = XDefaultScreenOfDisplay (s_XDisplay);
    cid->XScreenWidth  = WidthOfScreen (XScreen);
    cid->XScreenHeight = HeightOfScreen (XScreen);
    
    //g_print ("%dx%d\n",XScreenWidth,XScreenHeight);
}
