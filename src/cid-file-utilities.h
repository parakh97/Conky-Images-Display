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

#include "cid-struct.h"

G_BEGIN_DECLS

/**
 * Permet de copier un fichier en vérifiant les sommes md5.
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

/**
 * Vérifie que le fichier de configuration existe.
 * @param f fichier de configuration à lire.
 * @return TRUE si il existe.
 */
gboolean cid_file_check (const char *f);

/**
 * Vérifie la version du fichier de configuration.
 * @param pCid Structure de configuration.
 * @param f Path du fichier à vérifier.
 * @return TRUE si les versions correspondent.
 */
gboolean cid_file_check_conf_version (CidMainContainer **pCid, const gchar *f);

/**
 * Déplace un fichier en vérifiant les sommes md5.
 * @param cSrc Le fichier source.
 * @param cDst Le fichier déstination.
 * @return TRUE si il n'y a pas d'erreurs.
 */
gboolean cid_file_move (const gchar *cSrc, const gchar *cDst);

/**
 * Retourne la liste de toutes les images d'un dossier.
 * @param pCid Structure de configuration.
 * @param cDirName Dossier dans lequel il faut chercher les images.
 * @return CidDataTable contenant l'ensemble des images.
 */
CidDataTable *cid_images_lookup (CidMainContainer **pCid, const gchar *cDirName);

G_END_DECLS
#endif
