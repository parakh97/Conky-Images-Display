/*
   *
   *                cid-string-utilities.c
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/

#include "cid-string-utilities.h"
#include "cid-struct.h"

gchar *
cid_toupper (gchar *cSrc) 
{
    register int t;
    gchar *cRes = g_malloc (sizeof(gchar)*(strlen(cSrc)+1));

    for(t=0; cSrc[t]; t++)  
    {
        cRes[t] = toupper(cSrc[t]);
    }
    cRes[t] = '\0';
    return cRes;
}

static void
cid_datacase_replace (CidDataCase *pCase, gpointer *pData)
{
    if (pCase != NULL)
    {
        gchar **c_tmp = pData[2];
        if (GPOINTER_TO_INT(pData[0]) < GPOINTER_TO_INT(pData[1]))
        {
            gchar *tmp = g_strdup(*c_tmp);
            g_free (*c_tmp);
            *c_tmp = NULL;
            *c_tmp = g_strdup_printf ("%s%s%s",tmp,pCase->content->string,(gchar *)pData[3]);
            g_free (tmp);
        } 
        else
        {
            gchar *tmp = g_strdup(*c_tmp);
            g_free (*c_tmp);
            *c_tmp = NULL;
            *c_tmp = g_strdup_printf ("%s%s",tmp,pCase->content->string);
            g_free (tmp);
        }
    }
}

void
cid_str_replace_all (gchar **string, const gchar *sFrom, const gchar *sTo)
{
    g_return_if_fail (*string != NULL && sFrom != NULL && sTo != NULL);
    
    gchar **tmp = g_strsplit(*string,sFrom,0);
    CidDataTable *t_temp = cid_datatable_new();
    while (*tmp != NULL)
    {
        cid_datatable_append(&t_temp,cid_datacontent_new_string(*tmp));
        g_free (*tmp);
        tmp++;
    }
    size_t size = cid_datatable_length(t_temp);
    if (size < 2)
    {
        cid_clear_datatable(&t_temp);
        return;
    }
    int length = (strlen(*string)+((strlen(sTo)-strlen(sFrom))*size))*sizeof(gchar)+1;
    g_free (*string);
    *string = NULL;
    *string = g_malloc0(length);
    gpointer *pData = g_new(gpointer,5);
    pData[0] = GINT_TO_POINTER(0);
    pData[1] = GINT_TO_POINTER(size);
    pData[2] = string;
    pData[3] = (gchar *)g_strdup(sTo);
    cid_datatable_foreach(t_temp,(CidDataAction)cid_datacase_replace,pData);
    cid_clear_datatable(&t_temp);
    g_free (pData[3]);
    g_free (pData);
}

void
cid_str_replace_all_seq (gchar **string, gchar *seqFrom, gchar *seqTo)
{
//    if (strlen(seqFrom) != strlen(seqTo))
//        return;
    /*
    while (*seqFrom != '\0' && *seqTo != '\0')
    {
        gchar *from = g_malloc0(2*sizeof(gchar)), *to = g_malloc0(2*sizeof(gchar));
        g_sprintf(from,"%c",*seqFrom);
        g_sprintf(to,"%c",*seqTo);
        g_print("from: %s, to: %s\n",from,to);
        cid_str_replace_all (string,from,to);
        g_free(from), from = NULL;
        g_free(to), to = NULL;
        seqFrom++;
        seqTo++;
    }
    */
    for(;*seqFrom != '\0';seqFrom++)
    {
        fprintf (stdout,"%c\n",*seqFrom);
    }
}

void
cid_parse_nl (gchar **input)
{
    gchar *in = *input;
    gint length = strlen (*input);
    gint ind = 0, cpt = 0;
    gboolean found = FALSE;
    gchar *output = g_malloc (sizeof(gchar)*(length+1));
    while (ind<length)
    {
        if (in[ind] == '\\' && ind < length-1 && in[ind+1] == 'n')
        {
            output[cpt] = '\n';
            ind++;
            found = TRUE;
        }
        else
        {
            output[cpt] = in[ind];
        }
        ind++,cpt++;
    }
    output[cpt] = '\0';
    if (found)
    {
        g_free (*input);
        *input = NULL;
        *input = g_strdup (output);
    }
    g_free (output);
}

static void
cid_proceed_substitute (CidDataCase *pCase, gpointer *pData)
{
    cid_str_replace_all (pData[1],pCase->content->sub->regex,pCase->content->sub->replacement);
}

void 
cid_substitute_user_params (gchar **cPath)
{
    CidDataTable *table = cid_create_datatable (CID_TYPE_SUBSTITUTE, 
                                                cid_new_substitute ("%artist%",musicData.playing_artist ? 
                                                                               musicData.playing_artist :
                                                                               ""),
                                                cid_new_substitute ("%album%",musicData.playing_album ?
                                                                              musicData.playing_album :
                                                                              ""),
                                                cid_new_substitute ("%home%",g_getenv ("HOME")),
                                                cid_new_substitute ("%user%",g_getenv ("USER")),
                                                G_TYPE_INVALID);

    gpointer *pData = g_new0(gpointer, 2);
    pData[0] = GINT_TO_POINTER(0);
    pData[1] = cPath;
    cid_datatable_foreach (table, (CidDataAction) cid_proceed_substitute, pData);
    cid_clear_datatable (&table);
    g_free (pData);
}

CidSubstitute *
cid_new_substitute (const gchar *regex, const gchar *replacement)
{
    CidSubstitute *ret = g_new0 (CidSubstitute, 1);
    if (ret != NULL)
    {
        ret->regex = g_strdup (regex);
        ret->replacement = g_strdup (replacement);
    }
    return ret;
}

void
cid_free_substitute (CidSubstitute *pSub)
{
    if (pSub == NULL)
        return;
    g_free (pSub->regex);
    g_free (pSub->replacement);
    g_free (pSub);
}

static void
cid_proceed_regex (gchar **cString, const gchar *regex, const gchar *replacement)
{
    g_return_if_fail (*cString != NULL);
    
    GError *error = NULL;
    GRegex *reg = g_regex_new (regex,0,0,&error);
    if (error != NULL)
    {
        fprintf (stderr,"g_regex_new error: %s\n",error->message);
        g_error_free (error);
        error = NULL;
        g_regex_unref (reg);
        return;
    }
    gchar *res = g_regex_replace (reg, *cString, -1, 0, replacement, 0, &error);
    if (error != NULL)
    {
        fprintf (stderr,"g_regex_replace error: %s\n",error->message);
        g_error_free (error);
        error = NULL;
        g_regex_unref (reg);
        return;
    }
    g_free (*cString);
    *cString = g_strdup (res);
    g_free (res);
    g_regex_unref (reg);
}

static void
cid_foreach_proceed_regex (CidDataCase *pCase, gpointer *pData)
{
    cid_proceed_regex (pData[1],pCase->content->sub->regex,pCase->content->sub->replacement);
}

void 
cid_str_prepare (gchar **cString)
{
    CidDataTable *table = cid_create_datatable (CID_TYPE_SUBSTITUTE, 
                                                cid_new_substitute ("\\(.*\\)",""),
                                                cid_new_substitute ("\\[.*\\]",""),
                                                cid_new_substitute ("_"," "),
                                                cid_new_substitute (" +"," "),
                                                cid_new_substitute (" $",""),
                                                G_TYPE_INVALID);

    gpointer *pData = g_new0(gpointer, 2);
    pData[0] = GINT_TO_POINTER(0);
    pData[1] = cString;
    cid_datatable_foreach (table, (CidDataAction) cid_foreach_proceed_regex, pData);
    cid_clear_datatable (&table);
    g_free (pData);
}

gboolean
cid_str_match (const gchar *cString, const gchar *cRegex)
{
    return g_regex_match_simple (cRegex, cString, G_REGEX_CASELESS, 0);
}

CidDataTable *
cid_str_split (const gchar *cString, const gchar cTokken)
{
    CidDataTable *ret = cid_datatable_new ();
    gchar *cTmp = g_malloc (256 * sizeof(gchar));
    gint cpt = 0, i = 0, nb = 2;
    for (; *cString != 0; cString++,cpt++,i++)
    {
        if (cpt > 10)
        {
            cTmp = g_realloc (cTmp, nb * 256 * sizeof(gchar));
            if (cTmp == NULL)
            {
                g_free (cTmp);
                cid_clear_datatable (&ret);
                return NULL;
            }
            nb++;
            cpt = 0;
        }
        if (*cString == cTokken)
        {
            if (i > 0)
            {
                cTmp[i] = '\0';
                gchar *cAdd = g_strndup (cTmp, i - 1);
                cid_datatable_append (&ret, cid_datacontent_new_string (cAdd));
                g_free (cTmp);
                g_free (cAdd);
                cTmp = g_malloc (256 * sizeof(gchar));
                i = -1;
                nb = 2;
                cpt = -1;
            }
            continue;
        }
        cTmp[i] = *cString;
    }
    if (i > 0)
    {
        if (cpt > 10)
        {
            cTmp = g_realloc (cTmp, nb * 256 * sizeof(gchar));
            if (cTmp == NULL)
            {
                g_free (cTmp);
                cid_clear_datatable (&ret);
                return NULL;
            }
        }
        cTmp[i] = '\0';
        gchar *cAdd = g_strndup (cTmp, i - 1);
        cid_datatable_append (&ret, cid_datacontent_new_string (cAdd));
        g_free (cTmp);
        g_free (cAdd);
    }
    
    return ret;
}
