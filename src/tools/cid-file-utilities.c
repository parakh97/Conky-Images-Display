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
   *                cid-file-utilities.c
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/

#include "cid-file-utilities.h"
#include "cid-md5.h"
#include "cid-string-utilities.h"

#include "../cid-messages.h"
#include "../cid-constantes.h"
#include "../cid-config.h"

gboolean
cid_file_copy (const gchar *cSrc, const gchar *cDst)
{
    gboolean ret = TRUE;
    if (!cSrc || !cDst)
    {    
        cid_warning (
                "Unable to copy file due to missing field ! (%s,%s)",
                cSrc,
                cDst);
        return FALSE;
    }    
    FILE *src = fopen (cSrc,"rb");
    if (!src)
    {    
        cid_warning ("Unable to open file: %s",cSrc);
        return FALSE;
    }    
    FILE *dst = fopen (cDst,"wb");
    if (!dst)
    {    
        cid_warning ("Unable to open file: %s",cDst);
        fclose (src);
        return FALSE;
    }    
    
    char ch;
    while(!feof(src)) {
        ch = fgetc(src);
        if(ferror(src)) {
            cid_warning ("Error reading source file.");
            ret = FALSE;
            break;
        }    
        if(!feof(src)) fputc(ch, dst);
        if(ferror(dst)) {
            cid_warning ("Error writing destination file.");
            ret = FALSE;
            break;
        }    
    }    
    fclose (src);
    fclose (dst);
    gchar *sum1, *sum2;
    sum1 = cid_md5sum (cSrc);
    sum2 = cid_md5sum (cDst);
    if (g_strcmp0 (sum1, sum2) != 0)
    {
        cid_warning (
            "An unknown error occured while copying files... aborting");
        cid_file_remove (cDst);
        ret = FALSE;
    }
    g_free (sum1);
    g_free (sum2);

    return ret;
}

void
cid_file_remove (const gchar* cFilePath)
{
    if (!g_file_test (cFilePath, G_FILE_TEST_EXISTS))
    {    
        cid_warning ("The file '%s' does not exist", cFilePath);
        return;
    }    
    if (remove(cFilePath) == -1)
    {    
        cid_warning ("Error while removing %s",cFilePath);
    }    
}

gboolean 
cid_file_check_config_version (CidMainContainer **pCid, const gchar *f) 
{
    CidMainContainer *cid = *pCid;
    gchar *cCommand=NULL;
    gchar line_f1[80], line_f2[80];
    FILE *f1, *f2;

    if (cid->config->bDevMode)
        return TRUE;

    gchar *cOrigFile = 
            g_strdup_printf("%s/%s",CID_DATA_DIR, CID_CONFIG_FILE);

    f1 = fopen ((const char *)cOrigFile,"r");
    f2 = fopen ((const char *)f,"r");
    g_free (cOrigFile);

    if (!f1 || !f2)
        cid_exit (pCid, 3, "couldn't read conf file, try to delete it");
    
    if (!fgets(line_f1,80,f1) || !fgets(line_f2,80,f2))
        cid_exit (pCid, 3,"couldn't read conf file, try to delete it");
    
    fclose (f1);
    fclose (f2);
    
    cid_info ("line_f1 %s\nline_f2 %s",line_f1,line_f2);
        
    if (strcmp(line_f1,line_f2)!=0 || (*pCid)->config->bUnvalidKey) 
    {
        cid_warning ("bad file version, building a new one\n");
        cid_file_remove (f);
        gchar *cTmpPath = 
                g_strdup_printf("%s/%s",CID_DATA_DIR,CID_CONFIG_FILE);
        cid_file_copy(cTmpPath,f);
        g_free (cTmpPath);
        
        cid_save_data (pCid);
        cid_read_key_file (pCid, f);
        return FALSE;
    }
    return TRUE;
}

gboolean 
cid_file_move (const gchar *cSrc, const gchar *cDst)
{
    if (cid_file_copy (cSrc, cDst))
        cid_file_remove (cSrc);
    else
        return FALSE;
    
    return TRUE;
}

gboolean 
cid_file_check (const gchar *f) 
{
    gchar *cFileTest;
    gboolean ret = TRUE;
    if (!g_file_test (f, G_FILE_TEST_EXISTS))
    {
        gchar *cDirectory = g_path_get_dirname (f);
        if (! g_file_test (cDirectory, 
                    G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)) 
        {
            cid_info ("Creating path '%s'", cDirectory);
            g_mkdir_with_parents (cDirectory, 7*8*8+7*8+5);
        }
        g_free (cDirectory);
        
        /// retro-compatibilité: on déplace les anciens fichiers de conf
        /// si existants.
        cFileTest = g_strdup_printf("%s/%s",
                                    g_getenv("HOME"),
                                    OLD_CONFIG_FILE);
        if (g_file_test (cFileTest, G_FILE_TEST_EXISTS))
        {
            gchar *cSrc = g_strdup_printf("%s/%s",
                                          g_getenv("HOME"),
                                          OLD_CONFIG_FILE);
            gchar *cDst = g_strdup_printf("%s/.config/cid/%s",
                                          g_getenv("HOME"),
                                          CID_CONFIG_FILE);
            cid_debug ("Moving file from %s to %s",cSrc,cDst);
            ret = cid_file_move (cSrc,cDst);
            g_free (cSrc);
            g_free (cDst);
            g_free (cFileTest);
            return ret;
        }
        g_free (cFileTest);
        cFileTest = g_strdup_printf("%s/.config/%s",
                                    g_getenv("HOME"),
                                    OLD_CONFIG_FILE);
        if (g_file_test (cFileTest, G_FILE_TEST_EXISTS))
        {
            gchar *cSrc = g_strdup_printf("%s/.config/%s",
                                          g_getenv("HOME"),
                                          OLD_CONFIG_FILE);
            gchar *cDst = g_strdup_printf("%s/.config/cid/%s",
                                          g_getenv("HOME"),
                                          CID_CONFIG_FILE);
            cid_debug ("Moving file from %s to %s",cSrc,cDst);
            ret = cid_file_move (cSrc,cDst);
            g_free (cSrc);
            g_free (cDst);
            g_free (cFileTest);
            return ret;
        }
        g_free (cFileTest);
        cFileTest = g_strdup_printf("%s/.config/%s",
                                    g_getenv("HOME"),
                                    CID_CONFIG_FILE);
        if (g_file_test (cFileTest, G_FILE_TEST_EXISTS))
        {
            gchar *cSrc = g_strdup_printf("%s/.config/%s",
                                          g_getenv("HOME"),
                                          CID_CONFIG_FILE);
            gchar *cDst = g_strdup_printf("%s/.config/cid/%s",
                                          g_getenv("HOME"),
                                          CID_CONFIG_FILE);
            cid_debug ("Moving file from %s to %s",cSrc,cDst);
            ret = cid_file_move (cSrc,cDst);
            g_free (cSrc);
            g_free (cDst);
            g_free (cFileTest);
            return ret;
        }
        g_free (cFileTest);
        
        /// Si on a pas trouvé d'ancienne conf, on recopie la conf par
        /// défaut.
        gchar *cSrc = g_strdup_printf("%s/%s",
                                      CID_DATA_DIR,
                                      CID_CONFIG_FILE);
        cid_debug ("Copying file from %s to %s",cSrc,f);
        ret = cid_file_copy (cSrc,f);
        g_free (cSrc);
    }
    return ret;
}

CidDataTable *
cid_images_lookup (CidMainContainer **pCid, const gchar *cDirName)
{
    g_return_val_if_fail (cDirName != NULL, NULL);
    
    CidMainContainer *cid = *pCid;
    CidDataTable *ret = cid_datatable_new ();
    GError *error = NULL;
    if (ret != NULL)
    {
        cid->runtime->pLookupDirectory = g_dir_open (cDirName,0,&error);
        if (error != NULL)
        {
            cid_warning ("%s", error->message);
            g_clear_error (&error);
            return NULL;
        }
        const gchar *cEntry = NULL;
        while ((cEntry = 
                g_dir_read_name (cid->runtime->pLookupDirectory))!=NULL)
        {
            //fprintf(stdout,"Testing file: %s ",cEntry);
            if (cid_str_match (cEntry,"\\.(jpg|png|gif|svg)$"))
            {
                cid_datatable_append (&ret, 
                                cid_datacontent_new_string (cEntry));
                //fprintf(stdout,"[OK]\n");
            }
            //else
            //{
            //    fprintf(stdout,"[KO]\n");
            //}
        }
        g_dir_close (cid->runtime->pLookupDirectory);
    }
    return ret;
}
