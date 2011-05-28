/*
   *
   *                cid-string-utilities.h
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/
#ifndef __CID_STRING_UTILITIES__
#define  __CID_STRING_UTILITIES__

#include <gtk/gtk.h>

#include "cid-datatables.h"

G_BEGIN_DECLS

/**
 * converti une chaine en son equivalent MAJUSCULE
 * @param cSrc chaine source
 * @return la chaine 'uppee'
 */
gchar *cid_toupper (gchar *cSrc);

/**
 * Permet de substituer des 'joker' saisis par l'utilisateur
 * @param cPath la chaîne à substituer
 */
void cid_substitute_user_params (gchar **cPath);

/**
 * Permet de transformer une chaîne "\n" en caractère '\n'.
 * @param input Pointeur vers la chaîne à parser.
 */
void cid_parse_nl (gchar **input);

/**
 * Fonction utilisee pour le str_replace_all
 * @param pCase element courant de l'iteration
 * @param pData donnees a utiliser
 */
static void cid_datacase_replace (CidDataCase *pCase, gpointer *pData);

/**
 * Permet de remplacer tous les motifs d'une chaine donnee
 * @param string chaine que l'on souhaite modifier
 * @param sFrom motif que l'on souhaite remplacer
 * @param sTo motif avec lequel on remplace
 */
void cid_str_replace_all (gchar **string, const gchar *sFrom, const gchar *sTo);

void cid_str_replace_all_seq (gchar **string, gchar *seqFrom, gchar *seqTo);

/**
 * Permet de préparer une chaîne pour la recherche sur Last.fm.
 * @param cString chaîne à préparer.
 */
void cid_str_prepare (gchar **cString);

/**
 * Permet de tester une expression régulière.
 * @param cString la chaîne à tester.
 * @param cRegex expression régulière à passer sur la chaîne.
 * @return TRUE si on match, FALSE sinon.
 */
gboolean cid_str_match (const gchar *cString, const gchar *cRegex);

/**
 * Permet de splitter une chaine de caractères.
 * @param cString la chaîne à splitter.
 * @param cTokken le caractère à utiliser pour découper la chaine.
 * @return un tableau de chaines de caractères.
 */
CidDataTable *cid_str_split (const gchar *cString, const gchar cTokken);

G_END_DECLS
#endif
