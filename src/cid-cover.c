/*
   *
   *                     cid-amazon.c
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/

//#include "cid-constantes.h"
#include "cid-cover.h"
#include "cid-struct.h"
#include "cid-messages.h"
#include "cid-utilities.h"
#include "cid-md5.h"

#include <curl/curl.h>
#include <fcntl.h>
#include <sys/stat.h>

int ret;
gchar *TAB_IMAGE_SIZES[] = {"large","extralarge"};

#ifdef LIBXML_READER_ENABLED

gboolean 
cid_get_xml_file (const gchar *artist, const gchar *album) 
{
    if (g_file_test (DEFAULT_DOWNLOADED_IMAGE_LOCATION, G_FILE_TEST_EXISTS))
    {
        cid_remove_file (DEFAULT_DOWNLOADED_IMAGE_LOCATION);
    }
    
    if (g_strcasecmp("Unknown",artist)==0 || g_strcasecmp("Unknown",album)==0 ||
        g_strcasecmp("Inconnu",artist)==0 || g_strcasecmp("Inconnu",album)==0)
    {
        return FALSE;
    }
    
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
    FILE * fp = fopen(DEFAULT_XML_LOCATION".tmp", "w"); 
    curl_easy_setopt(handle,  CURLOPT_WRITEDATA, fp); 
    curl_easy_setopt(handle,  CURLOPT_WRITEFUNCTION, fwrite);
    //curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);

    curl_easy_perform(handle);
    fclose(fp);
    curl_easy_cleanup(handle);
    
    if (rename (DEFAULT_XML_LOCATION".tmp",DEFAULT_XML_LOCATION) == -1)
    {
        cid_warning ("Cannot rename '%s' to '%s'", DEFAULT_XML_LOCATION".tmp",
                                                   DEFAULT_XML_LOCATION);
    }
    
    g_free (cTmpFilePath);
    g_free (cURLFull);
    
    return TRUE;
}

#else

gboolean 
cid_get_xml_file (const gchar *artist, const gchar *album) 
{
    return FALSE;
}

#endif

gboolean 
cid_download_missing_cover (const gchar *cURL/*, const gchar *cDestPath*/) 
{
    CURL *handle = curl_easy_init ();
    curl_easy_setopt(handle, CURLOPT_URL, cURL);
    FILE * fp = fopen(DEFAULT_DOWNLOADED_IMAGE_LOCATION".tmp", "w"); 
    curl_easy_setopt(handle,  CURLOPT_WRITEDATA, fp); 
    curl_easy_setopt(handle,  CURLOPT_WRITEFUNCTION, fwrite);
    //curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);

    curl_easy_perform(handle);
    fclose(fp);
    curl_easy_cleanup(handle);
    
    if (rename (DEFAULT_DOWNLOADED_IMAGE_LOCATION".tmp",DEFAULT_DOWNLOADED_IMAGE_LOCATION) == -1)
    {
        cid_warning ("Cannot rename '%s' to '%s'",DEFAULT_DOWNLOADED_IMAGE_LOCATION".tmp",
                                                  DEFAULT_DOWNLOADED_IMAGE_LOCATION);
    }
        
    return TRUE;
}

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
    
    cid_remove_file (filename);
    
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
        cid_warning ("Erreur lors de la création du contexte XPath");
        return;
    }
    
    // Evaluation de l'expression XPath
    //gchar *cXPath = g_strdup_printf (xpath,cid->config->iImageSize == MEDIUM_IMAGE ? "large" : "extralarge"/*TAB_IMAGE_SIZES[cid->config->iImageSize]*/);
    gchar *cXPath = g_strdup_vprintf (xpath, args);
    xmlXPathObjectPtr xpathRes = xmlXPathEvalExpression(cXPath, ctxt);
    g_free (cXPath);
    if (xpathRes == NULL) 
    {
        cid_warning ("Erreur sur l'expression XPath");
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

gchar * 
cid_db_store_cover (CidMainContainer **pCid,const gchar *cCoverPath,
                    const gchar *cArtist, const gchar *cAlbum)
{
    g_return_val_if_fail (cCoverPath != NULL 
                          && cArtist != NULL 
                          && cAlbum != NULL,
                          NULL);
    g_return_val_if_fail (g_strcasecmp (cArtist, "unknown") != 0
                          && g_strcasecmp (cAlbum, "unknown") != 0,
                          NULL);
    CidMainContainer *cid = *pCid;
    GKeyFile *pKeyFile;
    CURL *handle = curl_easy_init(); 
    gchar *cKey = g_strdup_printf ("%s_%s", curl_easy_escape (handle, cArtist, 0), 
                                            curl_easy_escape (handle, cAlbum, 0));
    curl_easy_cleanup (handle);
    gchar *cDBFile = g_strdup_printf ("%s/%s", cid->config->cDLPath, CID_COVER_DB);
    gchar *md5 = cid_md5sum (cCoverPath);
    GKeyFileFlags flags;
    GError *error = NULL;

    /* Create a new GKeyFile object and a bitwise list of flags. */
    pKeyFile = g_key_file_new ();
    flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

    if (!g_file_test (cid->config->cDLPath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)) 
    {
        cid_info ("Creating path %s", cid->config->cDLPath);
        g_mkdir_with_parents (cid->config->cDLPath, 7*8*8+7*8+5);
    }
    if (!g_file_test (cDBFile, G_FILE_TEST_EXISTS))
    {
        cid_info ("Creating file '%s'", cDBFile);
        int fd = open (cDBFile, O_CREAT, S_IRUSR | S_IWUSR);
        close (fd);
    }
    if (md5 == NULL)
    {
        cid_warning ("Unable to hash file");
        g_free (cKey);
        g_free (cDBFile);
        g_free (md5);
        g_key_file_free (pKeyFile);
        return NULL;
    }
    /* Load the GKeyFile or return. */
    if (!g_key_file_load_from_file (pKeyFile, cDBFile, flags, &error)) 
    {
        cid_warning ("%s",error->message);
        g_error_free (error);
    }
    gchar *cDestFile = g_strdup_printf ("%s/%s", cid->config->cDLPath, md5);
    cid_copy_file (cCoverPath, cDestFile);
    cid_remove_file (cCoverPath);
    g_key_file_set_value (pKeyFile, "DB", cKey, md5);
    cid_write_keys_to_file (pKeyFile, cDBFile);
    g_key_file_free (pKeyFile);
    g_free (cKey);
    g_free (cDBFile);
    g_free (md5);
    
    return cDestFile;
}

gchar *
cid_db_search_cover (CidMainContainer **pCid, const gchar *cArtist, const gchar *cAlbum)
{
    g_return_val_if_fail (cArtist != NULL && cAlbum != NULL, NULL);
    CidMainContainer *cid = *pCid;
    GKeyFile *pKeyFile;
    CURL *handle = curl_easy_init();
    gchar *cCoverPath = NULL;
    gchar *cKey = g_strdup_printf ("%s_%s", 
                                  curl_easy_escape (handle, cArtist, 0),
                                  curl_easy_escape (handle, cAlbum, 0));
    curl_easy_cleanup (handle);
    gchar *cDBFile = g_strdup_printf ("%s/%s", 
                                      cid->config->cDLPath, 
                                      CID_COVER_DB);
    GKeyFileFlags flags;
    GError *error = NULL;

    /* Create a new GKeyFile object and a bitwise list of flags. */
    pKeyFile = g_key_file_new ();
    flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

    if (!g_key_file_load_from_file (pKeyFile, cDBFile, flags, &error))
    {
        cid_warning ("%s",error->message);
        g_error_free (error);
        g_free (cKey);
        g_free (cDBFile);
        return NULL;
    }

    gchar *cVal = g_key_file_get_value (pKeyFile, "DB", cKey, &error);
    if (error != NULL)
    {
        cid_warning ("%s",error->message);
        g_error_free (error);
    }
    if (cVal != NULL)
    {
        cCoverPath =  g_strdup_printf ("%s/%s", cid->config->cDLPath, cVal);
        g_free (cVal);
    }

    g_free (cKey);
    g_free (cDBFile);
    g_key_file_free (pKeyFile);
        
    return cCoverPath;
}

gchar *
cid_cover_lookup (const gchar *cDir)
{
    return NULL;
}
