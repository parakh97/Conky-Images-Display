/*
   *
   *                cid-file-utilities.c
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/

#include "cid-file-utilities.h"
#include "cid-messages.h"
#include "cid-md5.h"

gboolean
cid_file_copy (const gchar *cSrc, const gchar *cDst)
{
    gboolean ret = TRUE;
    if (!cSrc || !cDst)
    {    
        cid_warning ("Unable to copy file due to missing field ! (%s,%s)",cSrc,cDst);
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
        cid_warning ("An unknown error occured while copying files... aborting");
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

