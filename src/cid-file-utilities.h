/*
   *
   *                cid-file-utilities.h
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/
#ifndef __CID_FILE_UTILITIES__
#define  __CID_FILE_UTILITIES__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * Permet de copier un fichier.
 * @param cSrc Le fichier source.
 * @param cDst Le fichier destination.
 * @return TRUE si la copie s'est bien passée.
 */
gboolean cid_file_copy (const gchar *cSrc, const gchar *cDst);

/**
 * Permet de supprimer un fichier.
 * @param cSrc Le fichier a supprimer.
 */
void cid_file_remove (const gchar* cFilePath);

G_END_DECLS
#endif
