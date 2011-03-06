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
#include "cid-struct.h"
#include "cid-X-utilities.h"

G_BEGIN_DECLS

#define _(string) gettext (string)

/**
 * Permet de lancer une commande en arriere plan
 * @param cCommand 'forme' de la commande
 * @param ... paramètres
 */
#define cid_launch_command(cCommand,...) cid_launch_command_full(cCommand, NULL, ##__VA_ARGS__)

/**
 * Permet de lancer une commande
 * @param cCommandFormat La commande a executer
 * @param cWorkingDirectory Le repertoire de travail
 * @param ... 
 */
gboolean cid_launch_command_full (const gchar *cCommandFormat, gchar *cWorkingDirectory, ...);
    
/**
 * sort en retournant le code de retour donné en parametre
 * @param code de retour
 * @return code de retournant
 */
int cid_sortie(CidMainContainer **pCid, int code);

/**
 * lit les arguments du programme
 * @param nombre d'arguments
 * @param tableau de chaines contenant la liste des arguments
 */
void cid_read_parameters (CidMainContainer **pCid, int *argc, char ***argv);

/**
 * definit la niveau de verbosite
 * @param niveau de verbosite
 */
void cid_set_verbosity (gchar *cVerbosity);

/**
 * Permet de deconnecter le monitoring du player
 */
void cid_disconnect_player (CidMainContainer **pCid);

/**
 * Nettoyage des donnees musicales
 */
void cid_free_musicData(void);

/**
 * Nettoyage de notre structure principale
 * @param pCid pointeur sur notre structure
 */
void cid_free_main_structure (CidMainContainer *pCid);

/**
 * permet de verifier que l'utilisateur entre des valeurs
 * correctes
 */
void cid_check_position (CidMainContainer **pCid);

void cid_encrypt_string( const gchar *cDecryptedString,  gchar **cEncryptedString );

void cid_decrypt_string( const gchar *cDecryptedString,  gchar **cEncryptedString );

G_END_DECLS
#endif
