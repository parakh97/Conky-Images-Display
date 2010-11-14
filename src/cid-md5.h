/*
   *
   *                             cid-md5.h
   *                               -------
   *                         Conky Images Display
   *             --------------------------------------------
   *
*/
#ifndef __CID_MD5__
#define __CID_MD5__

#include "cid-struct.h"

G_BEGIN_DECLS

/**
 * Calcule la somme md5 du fichier passe
 * en argument
 * @param cFileName fichier dont on veut
 * connaitre la somme md5
 * @return somme md5 formatee
 */
gchar *cid_md5sum (const gchar *cFileName);

G_END_DECLS
#endif
