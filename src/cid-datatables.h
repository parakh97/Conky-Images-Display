/*
   *
   *                  cid-datatables.h
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/
#ifndef __CID_DATATABLES__
#define  __CID_DATATABLES__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _CidDataTable CidDataTable;
typedef struct _CidDataContent CidDataContent;
typedef struct _CidDataCase CidDataCase;

typedef void (* CidDataAction) (CidDataCase *pCase, gpointer *pData);

/// Structure représentant un couple regex/replacement
typedef struct _CidSubstitute
{
    /// la regex recherchée
    gchar *regex;
    /// par quoi on remplace
    gchar *replacement;
} CidSubstitute;

/// Structure de données représentant un tableau
struct _CidDataTable {
    /// taille de la table
    size_t length;
    /// premier élément de la table
    CidDataCase *head;
    /// dernier élément de la table
    CidDataCase *tail;
};

/// Structure représentant le contenu d'une case
struct _CidDataContent {
    /// structure anonyme servant à encapsuler un seul type de donnée dans une case
    union {
        /// type chaine de caractère
        gchar *string;
        /// type entier
        gint iNumber;
        /// type booléen
        gboolean booleen;
        /// type #CidSubstitute
        CidSubstitute *sub;
    };
    /// type contenu dans la case
    GType type;
};

/// Structure représentant une case
struct _CidDataCase {
    /// contenu de la case de type #CidDataContent
    CidDataContent *content;
    /// case suivante
    CidDataCase *next;
    /// case précédante
    CidDataCase *prev;
};

/// Définition d'un nouveau type 'SUBSTITUTE'
#define CID_TYPE_SUBSTITUTE G_TYPE_MAKE_FUNDAMENTAL (49)


/**
 * Fonction permettant de creer un tableau dynamique
 * @param iDataType le type des donnees qui suivent
 * @param ... les données, pouvant etre des chaines, des booleens, des 
 * entiers, ou des GType. Il faut obligatoirement terminer par un
 * G_TYPE_INVALID
 * @return pointeur vers notre structure
 */
CidDataTable *cid_create_datatable (GType iDataType, ...);

/**
 * Fonction permettant de creer une tableau dynamique d'une taille donnee
 * avec une valeur par defaut donnee
 * @param iSize taille du tableau
 * @param iType type des donnees
 * @param value valeur par defaut
 * @return notre tableau
 */
CidDataTable *cid_create_sized_datatable_with_default_full (size_t iSize, GType iType, gpointer value);

#define cid_create_sized_datatable_with_default(iSize,iType,value) cid_create_sized_datatable_with_default_full(iSize,iType,(gpointer)value)
#define cid_create_sized_datatable(iSize) cid_create_sized_datatable_with_default(iSize,G_TYPE_INVALID,NULL)

/**
 * Fonction permettant de liberer notre liste
 * @param pointeur vers notre liste
 */
void cid_free_datatable(CidDataTable **p_list);

/**
 * Permet de supprimer le premier element 'data' de la liste 'p_list'
 * @param p_list liste de depart
 * @param data element a supprimer
 * @return nouvelle liste 
 */
void cid_datatable_remove(CidDataTable **p_list, CidDataContent *data);

/**
 * Permet de supprimer tous les elements 'data' de la liste 'p_list'
 * @param p_list liste de depart
 * @param data element a supprimer
 * @return nouvelle liste
 */
void cid_datatable_remove_all(CidDataTable **p_list, CidDataContent *data);

/**
 * Supprime de la liste 'p_list' l'element situe a l'indice 'position'
 * @param p_list liste de depart
 * @param position indice de l'element a supprimer
 * @return nouvelle liste
 */
void cid_datatable_remove_id(CidDataTable **p_list, gint position);

/**
 * Permet de connaitre la taille d'une liste
 * @param p_list liste dont on souhaite connaitre la taille
 * @return taille de la liste
 */
size_t cid_datatable_length(CidDataTable *p_list);

/**
 * Permet d'inserer un element a l'indice souhaite
 * @param p_list liste dans laquelle on souhaite ajouter l'element
 * @param data element a ajouter
 * @param position indice ou l'on souhaite ajouter l'element
 * @return nouvelle liste
 */
void cid_datatable_insert(CidDataTable **p_list, CidDataContent *data, gint position);

/**
 * Permet d'ajouter un element en debut de liste
 * @param p_list liste a laquelle on souhaite ajouter un element
 * @param data element a ajouter
 * @return nouvelle liste
 */
void cid_datatable_prepend(CidDataTable **p_list, CidDataContent *data);

/**
 * Permet d'ajouter un element en fin de liste
 * @param p_list liste a laquelle on souhaite ajouter un element
 * @param data element a ajouter
 * @return nouvelle liste
 */
void cid_datatable_append(CidDataTable **p_list, CidDataContent *data);

/**
 * Permet de creer un nouvel element pouvant etre insere dans une liste
 * @param iType type de l'element
 * @param value valeur de l'element
 * @return nouvel element
 */
CidDataContent *cid_datacontent_new (GType iType, gpointer value);

/**
 * Permet de comparer deux elements
 * @param d1 element 1
 * @param d2 element 2
 * @return TRUE si les elements sont egaux (en terme de valeur), sinon FALSE 
 */
gboolean cid_datacontent_equals (CidDataContent *d1, CidDataContent *d2);

/**
 * Permet de parcourir tous les elements d'une liste donnee, et d'effectue le traitement voulu
 * @param p_list liste que l'on souhaite parcourir
 * @param func pointeur vers la fonction que l'on souhaite effectuer sur chaque element de la liste
 * @param pData donnees a transmettre a la fonction func
 * /!\ pData[0] contiendra toujours l'indice de l'element courant
 */
void cid_datatable_foreach (CidDataTable *p_list, CidDataAction func, gpointer *pData);

/**
 * Permet d'afficher la valeur d'un element
 * @param pCase element dont on souhaite afficher la valeur
 * @param pData donnees provenant eventuellement de cid_datatable_foreach
 */
void cid_datacase_print (CidDataCase *pCase, gpointer *pData);

/**
 * Permet de supprimer un element
 * @param pCase element a supprimer
 * @param pData donnees provenant du foreach
 */
void cid_free_datacase_full (CidDataCase *pCase, gpointer *pData);

/**
 * Permet de supprimer un conteneur
 * @param pContent contenu a supprimer
 * @param pData donnees provenant du foreach
 */
void cid_free_datacontent_full (CidDataContent *pContent, gpointer *pData);

#define cid_free_datacase(val) cid_free_datacase_full(val, NULL)

#define cid_free_datacontent(val) cid_free_datacontent_full(val, NULL)

#define cid_datacontent_new_string(val) cid_datacontent_new(G_TYPE_STRING, (gpointer)val)
#define cid_datacontent_new_int(val) cid_datacontent_new(G_TYPE_INT, (gpointer)val)
#define cid_datacontent_new_boolean(val) cid_datacontent_new(G_TYPE_BOOLEAN, (gpointer)val)
#define cid_datacontent_new_substitute(val) cid_datacontent_new(CID_TYPE_SUBSTITUTE, (gpointer)val)

#define cid_datatable_remove_first(list) cid_datatable_remove_id(list, 1)
#define cid_datatable_remove_last(list) cid_datatable_remove_id(list, cid_datatable_length(list))

/**
 * Permet de convertir un tableau de chaînes classic en DataTable.
 * @param table tableau de départ.
 * @param iSize nombre d'élément à copier (-1 pour tout le tableau).
 * @return un DataTable contenant une copie de l'ensemble des élements
 *         du tableau passé en paramètre.
 */
CidDataTable *cid_char_table_to_datatable (gchar **table, gint iSize);

/**
 * Permet de convertir un DataTable en tableau de chaînes.
 * @param pTable table à convertir.
 * @param iSize pointeur retournant le nombre d'éléments.
 * @return un tableau de chaînes de caractères contenant une copie de 
 *         l'ensemble des chaînes de la table.
 */
gchar **cid_datatable_to_char_table (CidDataTable *pTable, gint *iSize);

/**
 * Permet de cloner un datatable.
 * @param pSource datatable à cloner.
 * @return un clone du datatable source.
 */
CidDataTable *cid_clone_datatable (CidDataTable *pSource);

/**
 * Crée une nouvelle structure CidSubstitute.
 * @param regex La regex du substitute.
 * @param replacement Ce par quoi on remplace la regex.
 * @return La nouvelle structure CidSubstitute alouée.
 */
 CidSubstitute *cid_new_substitute (const gchar *regex, const gchar *replacement);
 
/**
 * Libère une structure CidSubstitute.
 * @param pSub La structure à libérer.
 */
void cid_free_substitute (CidSubstitute *pSub);

CidDataTable *cid_datatable_new (void);

#define BEGIN_FOREACH_DT(dt) \
CidDataTable *p_dt=dt; \
CidDataCase *p_temp=p_dt->head; \
while(p_temp!=NULL) \
{

#define END_FOREACH_DT \
p_temp=p_temp->next; \
} \
cid_free_datatable(&p_dt);

#define END_FOREACH_DT_NF \
p_temp=p_temp->next; \
} 

G_END_DECLS
#endif
