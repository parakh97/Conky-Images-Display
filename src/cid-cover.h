/*
   *
   *                     cid-cover.h
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/
#ifndef __CID_COVER__
#define  __CID_COVER__

#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <gtk/gtk.h>
#include "cid-struct.h"

G_BEGIN_DECLS

#define LICENCE_KEY "0C3430YZ2MVJKQ4JEKG2"
#define AMAZON_API_URL_1 "http://ecs.amazonaws.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="
#define AMAZON_API_URL_2 "&AssociateTag=webservices-20&ResponseGroup=Images,ItemAttributes&Operation=ItemSearch&ItemSearch.Shared.SearchIndex=Music"
#define DEFAULT_XML_LOCATION "/tmp/cid_amazon.xml"
#define DEFAULT_DOWNLOADED_IMAGE_LOCATION "/tmp/default.jpg"

#define LAST_ID_KEY "941340dc17f3e2704c5b06ebf845e98e"
#define SECRET_ID_KEY "4f46fa5c4a8d4eaf794cdb6ff7bff2ea"
#define LAST_API_URL "http://ws.audioscrobbler.com/2.0/?method="
#define LAST_ARTIST "artist.getinfo"
#define LAST_ALBUM "album.getinfo"

#define LAST_XPATH "//image[@size='%s']/text()"

#define TEST_XML "../data/test.xml"

#define CID_COVER_DB "cover.db"


/**
 * Parse le fichier XML passé en argument
 * à la recherche de l'URL de la pochette
 * @param filename URI du fichier à lire
 * @param cValue buffer dans lequel placer l'URL
 * @param xpath expression a rechercher
 */
void cid_search_xml_xpath (const char *filename, gchar **cValue, const gchar *xpath, ...);

#define cid_get_cover_url(filename,cValue) cid_search_xml_xpath(filename,cValue,LAST_XPATH,cid->config->iImageSize==MEDIUM_IMAGE?"large":"extralarge")

/**
 * Recupere le fichier xml sur amazon.
 * @param artist Nom de l'artiste.
 * @param album Nom de l'album.
 * @return succes du telechargement.
 */
gboolean cid_get_xml_file (const gchar *artist, const gchar *album);

/**
 * Recupere la pochette.
 * @param cURL URL de la pochette.
 * @param cDestPath Ou en enregistre la pochette telechargee.
 * @return succes du telechargement.
 */
gboolean cid_download_missing_cover (const gchar *cURL/*, const gchar *cDestPath*/);

/**
 * On sauvegarde la pochette dans notre 'magazin'.
 * @param pCid Structure de controle de cid.
 * @param cCoverPath URI de la pochette a stocker.
 * @return URI de la pochette apres stockage dans notre
 * 'magazin', NULL si stockage impossible.
 */
gchar *cid_store_cover (CidMainContainer **pCid,const gchar *cCoverPath,
                        const gchar *cArtist, const gchar *cAlbum);

/**
 * Recherche une pochette dans notre 'magazin'.
 * @param pCid Structure de controle de cid.
 * @param cArtist nom de l'artiste.
 * @param cAlbum nom de l'album.
 * @return URI complet de l'image, NULL si non toruve.
 */
gchar *cid_search_cover (CidMainContainer **pCid, const gchar *cArtist,
                         const gchar *cAlbum);

G_END_DECLS
#endif
