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
   *                             cid-config.h
   *                               -------
   *                         Conky Images Display
   *             --------------------------------------------
   *
*/
#ifndef __CID_CONFIG__
#define __CID_CONFIG__

#include "cid-struct.h"

G_BEGIN_DECLS

/**
 * Fonction permettant de recuperer une couleur dans un fichier de configuration.
 * @param pCid Structure de configuration.
 * @param pKeyFile Fichier de configuration.
 * @param cGroup Groupe auquel appartient la cle recherchee.
 * @param cKey Cle recherchee.
 * @param bAlpha Si on a un canal alpha.
 * @return La couleur correspondant aux criteres.
 */
CidColorContainer *cid_get_color_value_full (CidMainContainer **pCid, GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bAlpha);
#define CID_CONFIG_GET_COLOR(cGroup,cKey) cid_get_color_value_full (pCid,cid->pKeyFile,cGroup,cKey, FALSE)
#define CID_CONFIG_GET_COLOR_WITH_ALPHA(cGroup,cKey) cid_get_color_value_full (pCid,cid->pKeyFile,cGroup,cKey,TRUE)

/**
 * Fonction permettant de recuperer un boolean dans un fichier de configuration.
 * @param pCid Structure de configuration.
 * @param pKeyFile Fichier de configuration.
 * @param cGroup Groupe auquel appartient la cle recherchee.
 * @param cKey Cle recherchee.
 * @param bDefault Valeur par defaut.
 * @return Le booleen correspondant aux criteres.
 */
gboolean cid_get_boolean_value_full (CidMainContainer **pCid, GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault);
#define CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT(cGroup,cKey,bDefault) cid_get_boolean_value_full (pCid,cid->pKeyFile,cGroup,cKey,bDefault)
#define CID_CONFIG_GET_BOOLEAN(cGroup,cKey) cid_get_boolean_value_full (pCid,cid->pKeyFile,cGroup,cKey,FALSE)

/**
 * Fonction permettant de recuperer une chaine dans un fichier de configuration.
 * @param pCid Structure de configuration.
 * @param pKeyFile Fichier de configuration.
 * @param cGroup Groupe auquel appartient la cle recherchee.
 * @param cKey Cle recherchee.
 * @param bDefault Si on souhaite une valeur par defaut.
 * @param cDefault Valeur par defaut.
 * @param bFile Si on veut tester l'existance d'un fichier.
 * @param bDir Si on veut tester l'existance d'un dossier.
 * @param bForce Si on souhaite forcer le resultat meme si le fichier/repertoire n'existe pas.
 * @return La chaine correspondante aux criteres.
 */
gchar *cid_get_string_value_full (CidMainContainer **pCid, GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault, gchar *cDefault, gboolean bFile, gboolean bDir, gboolean bForce);
#define CID_CONFIG_GET_STRING(cGroup,cKey) cid_get_string_value_full (pCid,cid->pKeyFile,cGroup,cKey,FALSE,NULL,FALSE,FALSE,FALSE)
#define CID_CONFIG_GET_STRING_WITH_DEFAULT(cGroup,cKey,cDefault) cid_get_string_value_full (pCid,cid->pKeyFile,cGroup,cKey,FALSE,cDefault,FALSE,FALSE,FALSE)
#define CID_CONFIG_GET_FILE_PATH(cGroup,cKey,cDefault) cid_get_string_value_full (pCid,cid->pKeyFile,cGroup,cKey,TRUE,cDefault,TRUE,FALSE,FALSE)
#define CID_CONFIG_GET_DIR_PATH(cGroup,cKey,cDefault) cid_get_string_value_full (pCid,cid->pKeyFile,cGroup,cKey,FALSE,cDefault,FALSE,TRUE,FALSE)
#define CID_CONFIG_GET_DIR_PATH_FORCE(cGroup,cKey,cDefault) cid_get_string_value_full (pCid,cid->pKeyFile,cGroup,cKey,FALSE,cDefault,FALSE,TRUE,TRUE)

/**
 * Fonction permettant de recuperer un int dans un fichier de configuration.
 * @param pCid Structure de configuration.
 * @param pKeyFile Fichier de configuration.
 * @param cGroup Groupe auquel appartient la cle recherchee.
 * @param cKey Cle recherchee.
 * @param bDefault Si on souhaite une valeur par defaut.
 * @param iDefault Valeur par defaut.
 * @param bMax Si on veut une valeur maximale.
 * @param iMax Valeur maximale.
 * @return L'entier correspondant aux criteres.
 */
gint cid_get_int_value_full (CidMainContainer **pCid, GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault, gint iDefault, gboolean bMax, gint iMax);
#define CID_CONFIG_GET_INTEGER(cGroup,cKey) cid_get_int_value_full (pCid,cid->pKeyFile,cGroup,cKey,FALSE,0,FALSE,0)
#define CID_CONFIG_GET_INTEGER_WITH_DEFAULT(cGroup,cKey,iDefault) cid_get_int_value_full (pCid,cid->pKeyFile,cGroup,cKey,TRUE,iDefault,FALSE,0)
#define CID_CONFIG_GET_INTEGER_WITH_DEFAULT_AND_MAX(cGroup,cKey,iDefault,iMax) cid_get_int_value_full (pCid,cid->pKeyFile,cGroup,cKey,TRUE,iDefault,TRUE,iMax)

/**
 * Fonction qui charge la configuration .
 * @param f fichier de configuration à lire.
 */
int cid_read_config (CidMainContainer **pCid, const char *f);

/**
 * sauvegarde les données de cid
 */
void cid_save_data (CidMainContainer **pCid);

/**
 * sauvegarde les clés dans le fichier de configurations.
 * @param pKeyFile pointeur de KeyFile.
 * @param cConfFilePath chemin du fichier à sauvegarder.
 */
void cid_write_keys_to_file (GKeyFile *pKeyFile, const gchar *cConfFilePath);

/**
 * Permet de recharger la configuration lorsqu'on applique des changement via le GUI.
 */
void cid_read_config_after_update (CidMainContainer **pCid, const char *f);

/**
 * Lit le fichier de configuration donne en parametre.
 */
void cid_read_key_file (CidMainContainer **pCid, const gchar *f);

/**
 * Permet de libérer le fichier de clés
 */
void cid_key_file_free(CidMainContainer **pCid);


gboolean cid_load_key_file(CidMainContainer **pCid, GKeyFile **pKeyFile, const gchar *cFile);

G_END_DECLS
#endif
