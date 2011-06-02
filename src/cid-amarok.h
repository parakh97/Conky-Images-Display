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
   *                     cid-amarok.h
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
*/

#ifndef __CID_AMAROK__
#define  __CID_AMAROK__

#include <gtk/gtk.h>

#include "cid-struct.h"

G_BEGIN_DECLS

/**
 * connexion à amarok
 * @param iInter durée entre 2 interrogations
 */
void cid_connect_to_amarok(CidMainContainer **pCid, gint iInter);

/**
 * deconnexion d'amarok
 */
void cid_disconnect_from_amarok (CidMainContainer **pCid);

/**
 * Ajoute les options de monitoring de amarok
 */
void cid_build_amarok_menu (CidMainContainer **pCid);

G_END_DECLS
#endif

