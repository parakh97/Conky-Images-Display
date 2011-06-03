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
   *                  cid-utilities.h
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/
#ifndef __CID_UTILITIES__
#define  __CID_UTILITIES__

#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <ctype.h>
#include "../cid-struct.h"
#include "cid-X-utilities.h"

G_BEGIN_DECLS

#define _(string) gettext (string)

/**
 * Permet de lancer une commande en arriere plan
 * @param cCommand 'forme' de la commande
 * @param ... paramètres
 */
#define cid_launch_command(cCommand,...) \
    cid_launch_command_full(cCommand, NULL, ##__VA_ARGS__)

/**
 * Permet de lancer une commande
 * @param cCommandFormat La commande a executer
 * @param cWorkingDirectory Le repertoire de travail
 * @param ... 
 */
gboolean cid_launch_command_full (const gchar *cCommandFormat, 
                                  gchar *cWorkingDirectory, ...);
    
/**
 * Sort en retournant le code de retour donné en parametre.
 * @param pCid Structure de configuration.
 * @param code Code de retour.
 * @return code de retour
 */
int cid_sortie(CidMainContainer **pCid, int code);

/**
 * Lit les arguments du programme.
 * @param pCid Structure de configuration.
 * @param argc Nombre d'arguments.
 * @param argv Tableau de chaines contenant la liste des arguments.
 */
void cid_read_parameters (CidMainContainer **pCid, 
                          int *argc, 
                          char ***argv);

/**
 * Definit la niveau de verbosite.
 * @param cVerbosity Niveau de verbosite.
 */
void cid_set_verbosity (gchar *cVerbosity);

/**
 * Permet de deconnecter le monitoring du player.
 * @param pCid Structure de configuration.
 */
void cid_disconnect_player (CidMainContainer **pCid);

/**
 * Nettoyage des donnees musicales
 */
void cid_free_musicData(void);

/**
 * Nettoyage de notre structure principale.
 * @param pCid pointeur sur notre structure.
 */
void cid_free_main_structure (CidMainContainer *pCid);

/**
 * Permet de verifier que l'utilisateur entre des valeurs correctes.
 * @param pCid Structure de configuration.
 */
void cid_check_position (CidMainContainer **pCid);

/**
 * Permet de chiffrer une chaine de caractères.
 * @param cDecryptedString Chaîne à chiffrer.
 * @param cEncryptedString Chaîne chiffrée.
 */
void cid_encrypt_string (const gchar *cDecryptedString, 
                         gchar **cEncryptedString );

/**
 * Permet de déchiffrer une chaine de caractères.
 * @param cEncryptedString Chaîne à déchiffrer.
 * @param cDecryptedString Chaîne déchiffrée.
 */
void cid_decrypt_string (const gchar *cEncryptedString, 
                         gchar **cDecryptedString );

G_END_DECLS
#endif
