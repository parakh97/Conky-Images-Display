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

