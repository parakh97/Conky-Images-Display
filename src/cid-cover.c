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
   *                     cid-cover.c
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/

//#include "cid-constantes.h"
#include "gui/cid-callbacks.h"
#include "cid-cover.h"
#include "cid-struct.h"
#include "cid-messages.h"
#include "tools/cid-utilities.h"
#include "tools/cid-md5.h"
#include "tools/cid-file-utilities.h"

#include <curl/curl.h>
#include <fcntl.h>
#include <sys/stat.h>

int ret;
gchar *TAB_IMAGE_SIZES[] = {"large","extralarge"};

#ifdef LIBXML_READER_ENABLED

gboolean 
cid_get_xml_file (const gchar *artist, 
                  const gchar *album) 
{
    if (g_file_test (DEFAULT_DOWNLOADED_IMAGE_LOCATION, G_FILE_TEST_EXISTS))
    {
        cid_file_remove (DEFAULT_DOWNLOADED_IMAGE_LOCATION);
    }
    
    if (g_strcasecmp("Unknown",artist)==0 
        || g_strcasecmp("Unknown",album)==0 
        || g_strcasecmp("Inconnu",artist)==0 
        || g_strcasecmp("Inconnu",album)==0)
    {
        return FALSE;
    }
    
    gchar *cURLBegin = g_strdup_printf("%s%s&api_key=%s",
                                       LAST_API_URL,LAST_ALBUM,
                                       LAST_ID_KEY);
    gchar *cTmpFilePath = g_strdup (DEFAULT_XML_LOCATION);
    
    CURL *handle = curl_easy_init(); 
    gchar *cTmpArtist = g_strdup (artist);
    gchar *cTmpAlbum = g_strdup (album);
    cid_str_prepare (&cTmpArtist);
    cid_str_prepare (&cTmpAlbum);
    gchar *cArtistClean = curl_easy_escape (handle, cTmpArtist, 0);
    gchar *cAlbumClean = curl_easy_escape (handle, cTmpAlbum, 0);
    gchar *cURLArgs = g_strdup_printf ("&artist=%s&album=%s", 
                                       cArtistClean, 
                                       cAlbumClean);
    gchar *cURLFull = g_strdup_printf ("%s%s", cURLBegin, cURLArgs);
    g_free (cTmpArtist);
    g_free (cTmpAlbum);
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
        cid_warning ("Cannot rename '%s' to '%s'", 
                     DEFAULT_XML_LOCATION".tmp",
                     DEFAULT_XML_LOCATION);
    }
    
    g_free (cTmpFilePath);
    g_free (cURLFull);
    
    return TRUE;
}

#else

gboolean 
cid_get_xml_file (const gchar *artist, 
                  const gchar *album) 
{
    return FALSE;
}

#endif

gboolean 
cid_download_missing_cover (const gchar *cURL) 
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
        cid_warning ("Cannot rename '%s' to '%s'",
                     DEFAULT_DOWNLOADED_IMAGE_LOCATION".tmp",
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
cid_search_xml_xpath (const char *filename, 
                      gchar **cValue, 
                      const gchar*xpath, 
                      ...)
{
    xmlDocPtr doc;
    *cValue = NULL;
    va_list args;
    va_start(args,xpath);
    doc = xmlParseFile(filename);
    
    cid_file_remove (filename);
    
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
cid_db_store_cover (CidMainContainer **pCid,
                    const gchar *cCoverPath,
                    const gchar *cArtist, 
                    const gchar *cAlbum)
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
    gchar *cKey = g_strdup_printf ("%s_%s", curl_easy_escape (handle, 
                                                              cArtist, 
                                                              0), 
                                            curl_easy_escape (handle, 
                                                              cAlbum, 
                                                              0));
    curl_easy_cleanup (handle);
    gchar *cDBFile = g_strdup_printf ("%s/%s", 
                                      cid->config->cDLPath, 
                                      CID_COVER_DB);
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
    gchar *cDestFile = g_strdup_printf ("%s/%s", 
                                        cid->config->cDLPath, 
                                        md5);
    cid_file_copy (cCoverPath, cDestFile);
    cid_file_remove (cCoverPath);
    g_key_file_set_value (pKeyFile, DB_GROUP_DOWNLOAD, cKey, md5);
    cid_write_keys_to_file (pKeyFile, cDBFile);
    g_key_file_free (pKeyFile);
    g_free (cKey);
    g_free (cDBFile);
    g_free (md5);
    
    return cDestFile;
}

gchar *
cid_db_search_cover (CidMainContainer **pCid, 
                     const gchar *cArtist, 
                     const gchar *cAlbum)
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

    gchar *cVal = g_key_file_get_value (pKeyFile, DB_GROUP_DOWNLOAD, cKey, &error);
    if (error != NULL)
    {
        cid_warning ("%s",error->message);
        g_error_free (error);
    }
    if (cVal != NULL)
    {
        cCoverPath =  g_strdup_printf ("%s/%s", 
                                       cid->config->cDLPath, 
                                       cVal);
        g_free (cVal);
    }

    g_free (cKey);
    g_free (cDBFile);
    g_key_file_free (pKeyFile);
        
    return cCoverPath;
}

gchar *
cid_cover_lookup (CidMainContainer **pCid, 
                  const gchar *cArtist, 
                  const gchar *cAlbum, 
                  const gchar *cDir)
{
    CidMainContainer *cid = *pCid;
    gchar *cRes = NULL;
    
    cid_clear_datatable (&cid->runtime->pImagesList);
    
    BEGIN_FOREACH_DT (cid->runtime->pCoversList)
        g_free (cRes);
        gchar *file = g_strdup (p_temp->content->string);
        cid_substitute_user_params (&file);
        if (*file == '/')
            cRes = g_strdup (file);
        else
            cRes = g_strdup_printf ("%s/%s.jpg", cDir, file);
        cid_debug ("   test de %s\n", cRes);
        g_free (file);
        if (g_file_test (cRes, G_FILE_TEST_EXISTS))
        {
            return cRes;
        }
    END_FOREACH_DT_NF
    
    g_free (cRes);
    cRes = cid_db_search_cover (pCid, cArtist, cAlbum);
    
    if (cRes != NULL)
        return cRes;
    
    g_free (cRes);
    cRes = NULL;
    
    cid->runtime->pImagesList = cid_images_lookup (pCid, cDir);
    
    if (cid_datatable_length (cid->runtime->pImagesList) > 0)
    {
        CidDataCase *p_tmp = cid->runtime->pImagesList->head;
        cRes = g_strdup_printf ("%s/%s", cDir, p_tmp->content->string);
    }
    
    if (cRes != NULL)
        return cRes;
    
    cid->runtime->iCheckIter = 0;
    if (musicData.iSidCheckCover != 0) 
    {
        g_source_remove (musicData.iSidCheckCover);
        musicData.iSidCheckCover = 0;
    }
    cid_debug ("l'image n'existe pas encore => on boucle.\n");
    musicData.iSidCheckCover = 
                g_timeout_add (1000, 
                               (GSourceFunc) _check_cover_is_present, 
                               pCid);

    return NULL;
}
