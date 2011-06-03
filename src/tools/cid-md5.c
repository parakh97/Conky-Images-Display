/* Copyright (c) 2010 the authors listed at the following URL, and/or
the authors of referenced articles or incorporated external code:
http://en.literateprograms.org/MD5_sum_(C,_OpenSSL)?
action=history&offset=20090615063756

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from: 
    http://en.literateprograms.org/MD5_sum_(C,_OpenSSL)?oldid=16463
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "cid-md5.h"
#include "cid-messages.h"

#include <openssl/evp.h>


static gchar *
format_as_hex (const unsigned char *digest, int len) 
{
    gchar *ret = malloc (len * 2 * sizeof(gchar) + 1);
    if (ret == NULL) 
    {
        cid_warning ("Cannot allocate memory");
        return NULL;
    }
    int i,j;
    for(i = 0, j = 0; j < len; i+=2, j++)
    {
        gchar *tmp = g_strdup_printf ("%02x",digest[j]);
        ret[i] = tmp[0];
        ret[i+1] = tmp[1];
        g_free (tmp);
    }
    ret[i] = '\0';
    return ret;
}


static gchar * 
calculate_md5_of (const void *content, ssize_t len)
{
    EVP_MD_CTX mdctx;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
  
    EVP_DigestInit(&mdctx, EVP_md5());
    EVP_DigestUpdate(&mdctx, content, (size_t) len);
    EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);
    
    return format_as_hex (md_value, md_len);
}

gchar * 
cid_md5sum (const gchar *cFileName) 
{
    struct stat sStatBuf;
    int iRval = 0;
    int iFd = -1;
    off_t iSizeOfFile;
    ssize_t iReadBytes;
    void *pFileContent;
    gchar *ret;

    iRval = stat(cFileName, &sStatBuf);
    iSizeOfFile = sStatBuf.st_size;
    pFileContent = malloc(iSizeOfFile);
    if (NULL == pFileContent){
        cid_warning ("Can't allocate memory");
        return NULL;
    }
    iFd = open(cFileName, 0, O_RDONLY);
    if (iFd < 0 ){
        cid_warning ("Unable to open '%s'",cFileName);
        if (pFileContent) free(pFileContent);
        return NULL;
    }
    /* slurp in all from the file at once */
    iReadBytes = read(iFd, pFileContent, iSizeOfFile);
    if ( iReadBytes < 0 ) {
        cid_warning ("Error while reading '%s'",cFileName);
        if (pFileContent) free(pFileContent);
        if (iFd > 0) close(iFd);
        return NULL;
    }
    close(iFd);
    ret = calculate_md5_of (pFileContent, iSizeOfFile);
    free(pFileContent);
    
    return ret;
}

