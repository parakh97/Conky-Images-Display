/*
   *
   *                     cid-amazon.c
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/
//#include "cid.h"
#include "cid-cover.h"
#include "cid-struct.h"
#include "cid-messages.h"
#include "cid-utilities.h"

#include <curl/curl.h>

extern CidMainContainer *cid;

gboolean bCurrentlyDownloading = FALSE;
gboolean bCurrentlyDownloadingXML = FALSE;

#ifdef LIBXML_READER_ENABLED

int ret;
gchar *TAB_IMAGE_SIZES[] = {"large","extralarge"};

/**
 * Parse le fichier XML passé en argument
 * à la recherche de l'URL de la pochette
 * @param filename URI du fichier à lire
 * @param imageSize Taille de l'image que l'on souhaite
 */
void
cid_search_xml_xpath (const char *filename, gchar **cValue, const gchar*xpath, ...)
{
    xmlDocPtr doc;
    *cValue = NULL;
    va_list args;
    va_start(args,xpath);
    doc = xmlParseFile(filename);
    if (doc == NULL) 
    {
        cid_warning ("Document XML invalide");
        return;
    }
    // Initialisation de l'environnement XPath
    xmlXPathInit();
    // Création du contexte
    xmlXPathContextPtr ctxt = xmlXPathNewContext(doc);
    if (ctxt == NULL) 
    {
        cid_warning ( "Erreur lors de la création du contexte XPath");
        return;
    }
    
    // Evaluation de l'expression XPath
    //gchar *cXPath = g_strdup_printf (xpath,cid->config->iImageSize == MEDIUM_IMAGE ? "large" : "extralarge"/*TAB_IMAGE_SIZES[cid->config->iImageSize]*/);
    gchar *cXPath = g_strdup_vprintf (xpath, args);
    xmlXPathObjectPtr xpathRes = xmlXPathEvalExpression(cXPath, ctxt);
    g_free (cXPath);
    if (xpathRes == NULL) 
    {
        cid_warning ( "Erreur sur l'expression XPath");
        return;
    }
    
    // Manipulation du résultat
    if (xpathRes->type == XPATH_NODESET) 
    {
        int i;
        if (xpathRes->nodesetval != NULL)
        {
            for (i = 0; i < xpathRes->nodesetval->nodeNr; i++) 
            {
                xmlNodePtr n = xpathRes->nodesetval->nodeTab[i];
                if (n->type == XML_TEXT_NODE || n->type == XML_CDATA_SECTION_NODE) 
                {
                    *cValue = g_strdup (n->content);
                }
            }
        }
    }
    
    // Libération de la mémoire
    va_end (args);
    xmlXPathFreeObject(xpathRes);
    xmlXPathFreeContext(ctxt);
    xmlFreeDoc(doc);
}

gboolean 
cid_get_xml_file (const gchar *artist, const gchar *album) 
{
    cid_remove_file (DEFAULT_DOWNLOADED_IMAGE_LOCATION);
    
    if (g_strcasecmp("Unknown",artist)==0 || g_strcasecmp("Unknown",album)==0 ||
        g_strcasecmp("Inconnu",artist)==0 || g_strcasecmp("Inconnu",album)==0)
        return FALSE;
    
    gchar *cURLBegin = g_strdup_printf("%s%s&api_key=%s",LAST_API_URL,LAST_ALBUM,LAST_ID_KEY);
    gchar *cTmpFilePath = g_strdup (DEFAULT_XML_LOCATION);
    
    CURL *handle = curl_easy_init(); 
    gchar *cArtistClean = curl_easy_escape (handle, artist, 0);
    gchar *cAlbumClean = curl_easy_escape (handle, album, 0);
    gchar *cURLArgs = g_strdup_printf ("&artist=%s&album=%s", cArtistClean, cAlbumClean);
    gchar *cURLFull = g_strdup_printf ("%s%s", cURLBegin, cURLArgs);
    g_free (cURLArgs);
    g_free (cArtistClean);
    g_free (cAlbumClean);
    g_free (cURLBegin);
    curl_easy_setopt(handle, CURLOPT_URL, cURLFull);
    FILE * fp = fopen(DEFAULT_XML_LOCATION, "w"); 
    curl_easy_setopt(handle,  CURLOPT_WRITEDATA, fp); 
    curl_easy_setopt(handle,  CURLOPT_WRITEFUNCTION, fwrite);
    //curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);

    curl_easy_perform(handle);
    fclose(fp);
    curl_easy_cleanup(handle);

    bCurrentlyDownloadingXML = TRUE;
    
    g_free (cTmpFilePath);
    g_free (cURLFull);
    
    return TRUE;
}

gboolean 
cid_download_missing_cover (const gchar *cURL, const gchar *cDestPath) 
{
    bCurrentlyDownloading = TRUE;
    CURL *handle = curl_easy_init ();
    curl_easy_setopt(handle, CURLOPT_URL, cURL);
    FILE * fp = fopen(cDestPath, "w"); 
    curl_easy_setopt(handle,  CURLOPT_WRITEDATA, fp); 
    curl_easy_setopt(handle,  CURLOPT_WRITEFUNCTION, fwrite);
    //curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);

    curl_easy_perform(handle);
    fclose(fp);
    curl_easy_cleanup(handle);
    
    cid_remove_file (DEFAULT_XML_LOCATION);
    return TRUE;
}

#else

gboolean 
cid_get_xml_file (const gchar *artist, const gchar *album) 
{
    return FALSE;
}

#endif

