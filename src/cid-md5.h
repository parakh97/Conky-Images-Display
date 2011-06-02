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
