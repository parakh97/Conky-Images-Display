/*
   *
   *                  cid-utilities.c
   *                      -------
   *                Conky Images Display
   *             --------------------------
   *
*/

#include "cid-utilities.h"
#include "cid-struct.h"
#include "cid-callbacks.h"
#include "cid-messages.h"
#include "cid-constantes.h"
#include "cid-X-utilities.h"

#include <errno.h>
#ifdef HAVE_LIBCRYPT
/* libC crypt */
#include "crypt.h"

static char DES_crypt_key[64] =
{
    1,0,0,1,1,1,0,0, 1,0,1,1,1,0,1,1, 1,1,0,1,0,1,0,1, 1,1,0,0,0,0,0,1,
    0,0,0,1,0,1,1,0, 1,1,1,0,1,1,1,0, 1,1,1,0,0,1,0,0, 1,0,1,0,1,0,1,1
}; 
#endif

extern CidMainContainer *cid;
extern int ret;

/* Fonction de sortie en cas d'erreur, avec affichage d'un
   éventuel message d'erreur */
int 
cid_sortie(CidMainContainer **pCid, int code) 
{
    cid_disconnect_player (pCid);
    
    if (cid->runtime->bRunning)
        gtk_main_quit ();
    ret = code;
    return code;
}

void 
cid_disconnect_player (CidMainContainer **pCid) 
{
    cid_disconnect_from_amarok (pCid);
    rhythmbox_dbus_disconnect_from_bus ();
    cid_disconnect_from_exaile ();
    amarok_2_dbus_disconnect_from_bus ();
    cid_disconnect_from_mpd ();
}

void 
cid_free_musicData(void) 
{
    if (musicData.playing_uri!=NULL)
        g_free (musicData.playing_uri);
    if (musicData.playing_artist!=NULL)
        g_free (musicData.playing_artist);
    if (musicData.playing_album!=NULL)
        g_free (musicData.playing_album);
    if (musicData.playing_title!=NULL)
        g_free (musicData.playing_title);
    if (musicData.playing_cover!=NULL)
        g_free (musicData.playing_cover);
        
    musicData.playing_uri = NULL;
    musicData.playing_artist = NULL;
    musicData.playing_album = NULL;
    musicData.playing_title = NULL;
    musicData.playing_cover = NULL;

    musicData.playing_track = 0;
    musicData.playing_duration = 0;
    musicData.iSidCheckCover = 0;

    musicData.cover_exist = FALSE;
    musicData.playing = FALSE;
    musicData.opening = FALSE;
}

void 
cid_free_main_structure (CidMainContainer *pCid) 
{
    if (pCid->pWindow)
        gtk_widget_destroy(pCid->pWindow);
    if (pCid->p_cSurface)
        cairo_surface_destroy(pCid->p_cSurface);
    if (pCid->p_cPreviousSurface)
        cairo_surface_destroy(pCid->p_cPreviousSurface);
    if (pCid->p_cCross)
        cairo_surface_destroy(pCid->p_cCross);
    if (pCid->p_cPlay)
        cairo_surface_destroy(pCid->p_cPlay);
    if (pCid->p_cPause)
        cairo_surface_destroy(pCid->p_cPause);
    if (pCid->p_cPlay_big)
        cairo_surface_destroy(pCid->p_cPlay_big);
    if (pCid->p_cPause_big)
        cairo_surface_destroy(pCid->p_cPause_big);
    if (pCid->p_cNext)
        cairo_surface_destroy(pCid->p_cPrev);
    if (pCid->config->cConfFile)
        g_free(pCid->config->cConfFile);
    if (pCid->config->cVerbosity)
        g_free(pCid->config->cVerbosity);
        
    pCid->pWindow = NULL;
    pCid->p_cSurface = NULL;
    pCid->p_cPreviousSurface = NULL;
    pCid->p_cCross = NULL;
    pCid->p_cPlay = NULL;
    pCid->p_cPause = NULL;
    pCid->p_cPlay_big = NULL;
    pCid->p_cPause_big = NULL;
    pCid->p_cPrev = NULL;
    pCid->config->cConfFile = NULL;
    pCid->config->cVerbosity = NULL;
    
    g_free (pCid);
    pCid = NULL;
}
    
void
cid_copy_file (const gchar *cSrc, const gchar *cDst)
{
    if (!cSrc || !cDst)
    {
        cid_warning ("Unable to copy file due to missing field ! (%s,%s)",cSrc,cDst);
        return;
    }
    FILE *src = fopen (cSrc,"rb");
    if (!src)
    {
        cid_warning ("Unable to open file: %s",cSrc);
        return;
    }
    FILE *dst = fopen (cDst,"wb");
    if (!dst)
    {
        cid_warning ("Unable to open file: %s",cDst);
        return;
    }
    
    char ch;
    while(!feof(src)) {
        ch = fgetc(src);
        if(ferror(src)) {
            cid_warning ("Error reading source file.");
            return;
        }
        if(!feof(src)) fputc(ch, dst);
        if(ferror(dst)) {
            cid_warning ("Error writing destination file.");
            return;
        }
    }
    fclose (src);
    fclose (dst);
}

void
cid_remove_file (const gchar* cFilePath)
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

void 
cid_read_parameters (int *argc, char ***argv) 
{
    
    GError *erreur;
    gboolean bPrintVersion = FALSE, bTestingMode = FALSE, bDebugMode = FALSE, 
    bSafeMode = FALSE, bCafe = FALSE, bConfigPanel = FALSE;
    gchar *cConfFile = NULL;
    
    int i=0;
    for (; i<*argc; i++) 
    {
        if (strcmp((*argv)[i], "coin-coin" ) == 0) 
        {
            fprintf (stdout,COIN_COIN);
            exit (CID_EXIT_SUCCESS);
        }
        if (strcmp((*argv)[i], "coin" ) == 0) 
        {
            fprintf (stdout,COIN);
            exit (CID_EXIT_SUCCESS);
        }
        if (strcmp((*argv)[i], "dev" ) == 0) 
        {
            fprintf (stdout,"/!\\ CAUTION /!\\\nDevelopment mode !\n");
            if (cid->config->cConfFile)
                g_free (cid->config->cConfFile);
            if (cid->config->cDefaultImage)
                g_free (cid->config->cDefaultImage);
            cid->config->cConfFile = g_strdup_printf ("%s/%s", TESTING_DIR, TESTING_FILE);
            cid->config->cDefaultImage = g_strdup_printf ("%s/%s", TESTING_DIR, TESTING_COVER);
            cid->config->bDevMode = TRUE;
        }
    }
    
    GOptionEntry entries[] =
    {
        {"log", 'l', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_STRING,
            &cid->config->cVerbosity,
            dgettext (CID_GETTEXT_PACKAGE, "log verbosity (debug,info,message,warning,error) default is warning."), NULL},
        {"config", 'c', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_FILENAME,
            &cConfFile,
            dgettext (CID_GETTEXT_PACKAGE, "load CID with this config file instead of ~/.config/cid/cid.conf."), NULL},
        {"testing", 'T', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bTestingMode,
            dgettext (CID_GETTEXT_PACKAGE, "runs CID in testing mode. (some unstable options might be running)"), NULL},
        {"debug", 'd', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bDebugMode,
            dgettext (CID_GETTEXT_PACKAGE, "runs CID in debug mode. (equivalent to '-l debug')"), NULL},
        {"edit", 'e', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bConfigPanel,
            dgettext (CID_GETTEXT_PACKAGE, "open CID's configuration panel."), NULL},
        {"safe", 's', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bSafeMode,
            dgettext (CID_GETTEXT_PACKAGE, "runs CID in safe mode."), NULL},
        {"cafe", 'C', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bCafe,
            dgettext (CID_GETTEXT_PACKAGE, "do you want a cup of coffee?"), NULL},
        {"version", 'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE,
            &bPrintVersion,
            dgettext (CID_GETTEXT_PACKAGE, "print version and quit."), NULL},
        { NULL, 0, 0, 0, NULL, NULL, NULL }
    };

    GOptionContext *context = g_option_context_new ("");
    g_option_context_set_summary (context,dgettext (CID_GETTEXT_PACKAGE, "Conky Images Display is a programm written in C.\n\
Its goal is to display the cover of the song which \
is currently playing in the player you chooseon the \
desktop like conky does with other informations.\n\
You can use it with the following options:\n"));
    g_option_context_add_main_entries (context, entries, CID_GETTEXT_PACKAGE);
    if (!g_option_context_parse (context, argc, argv, &erreur))
    {
        cid_exit (&cid, CID_ERROR_READING_ARGS, "ERROR : %s\n", erreur->message);
    }
    
    if (bSafeMode) 
    {
        cid->config->bSafeMode = TRUE;
        fprintf (stdout,"Safe Mode\n");
    }
    
    if (bConfigPanel)
    {
        cid->config->bConfigPanel = TRUE;
        cid->config->bSafeMode = TRUE;
    }
    
    if (bDebugMode) 
    {
        if (cid->config->cVerbosity)
            g_free (cid->config->cVerbosity);
        cid->config->cVerbosity = g_strdup ("debug");   
    }

    if (bPrintVersion) 
    {
        gchar *env;
        env = g_malloc (20*sizeof(*env));
#ifdef HAVE_E17
                        snprintf (env,20,"e17");
#else
                        snprintf (env,20,"gnome and/or kde");
#endif
        fprintf (stdout,"Version: %s\n"
                        "Compiled for: %s\n"
                        ,CID_VERSION
                        ,env);
        g_free (env);
        exit (CID_EXIT_SUCCESS);
    }
    
    if (bCafe) 
    {
        fprintf (stdout,"Please insert coin.\n");
        exit (CID_EXIT_SUCCESS);
    }

    if (bTestingMode) 
    {
        cid->config->bTesting = TRUE;
    }
    
    if (cConfFile != NULL)
    {
        if (cid->config->cConfFile)
            g_free (cid->config->cConfFile);
        cid->config->cConfFile = g_strdup (cConfFile);
        g_free (cConfFile);
    }
}
    
void 
cid_set_verbosity (gchar *cVerbosity)
{
    if (!cVerbosity)
        cid_log_set_level(G_LOG_LEVEL_WARNING);
    else if (!strcmp(cVerbosity, "debug"))
        cid_log_set_level(G_LOG_LEVEL_DEBUG);
    else if (!strcmp(cVerbosity, "info"))
        cid_log_set_level(G_LOG_LEVEL_INFO);
    else if (!strcmp(cVerbosity, "message"))
        cid_log_set_level(G_LOG_LEVEL_MESSAGE);
    else if (!strcmp(cVerbosity, "warning"))
        cid_log_set_level(G_LOG_LEVEL_WARNING);
    else if (!strcmp(cVerbosity, "error"))
        cid_log_set_level(G_LOG_LEVEL_ERROR);
    else {
        cid_log_set_level(G_LOG_LEVEL_WARNING);
        cid_warning("bad verbosity option: default to warning");
    }
}

gboolean 
cid_launch_command_full (const gchar *cCommandFormat, gchar *cWorkingDirectory, ...) 
{
    g_return_val_if_fail (cCommandFormat != NULL, FALSE);
    
    va_list args;
    va_start (args, cWorkingDirectory);
    gchar *cCommand = g_strdup_vprintf (cCommandFormat, args);
    va_end (args);
    cid_debug ("%s (%s , %s)", __func__, cCommand, cWorkingDirectory);
    
    gchar *cBGCommand;
    if (cCommand[strlen (cCommand)-1] != '&') 
    {
        cBGCommand = g_strconcat (cCommand, " &", NULL);
        g_free (cCommand);
    }
    else
        cBGCommand = cCommand;
    GError *erreur = NULL;
    GThread* pThread = g_thread_create ((GThreadFunc) _cid_launch_threaded, cBGCommand, FALSE, &erreur);
    if (erreur != NULL) 
    {
        cid_warning ("couldn't launch this command (%s)", erreur->message);
        g_error_free (erreur);
        g_free (cBGCommand);
        return FALSE;
    }
    return TRUE;
}

gchar *
cid_toupper (gchar *cSrc) 
{
    register int t;
    gchar *cRes = g_malloc (sizeof(gchar)*(strlen(cSrc)+1));

    for(t=0; cSrc[t]; t++)  
    {
        cRes[t] = toupper(cSrc[t]);
    }
    return cRes;
}

CidDataTable *
cid_datatable_new (void)
{
    CidDataTable *res = g_new0(CidDataTable,1);
    if (res != NULL)
    {
        res->length = 0;
        res->head = NULL;
        res->tail = NULL;
    }
    return res;
}

CidDataCase *
cid_datacase_new (void)
{
    CidDataCase *ret = g_new0(CidDataCase,1);
    if (ret != NULL) 
    {
        ret->content = NULL;
        ret->next = NULL;
        ret->prev = NULL;
    }
    return ret;
}

CidDataContent *
cid_datacontent_new (GType iType, gpointer value)
{
    CidDataContent *ret = g_new0(CidDataContent,1);
    if (ret != NULL)
    {
        ret->type = iType;
        switch (iType) 
        {
            case G_TYPE_STRING:
                ret->string = NULL;
                int iLength = (strlen((gchar *) value)+1)*sizeof(gchar);
                ret->string = g_malloc0(iLength);
                strncpy(ret->string, (gchar *) value, iLength);
                break;
            case G_TYPE_INT:
                ret->iNumber = (gint)(long) value;
                break;
            case G_TYPE_BOOLEAN:
                ret->booleen = (gboolean)(long) value;
                break;
            case CID_TYPE_SUBSTITUTE:
                ret->sub = (CidSubstitute *) value;
                break;
            default:
                g_free(ret);
                return NULL;
        }
    }
    return ret;
}

gboolean
cid_datacontent_equals (CidDataContent *d1, CidDataContent *d2)
{
    if (d1 == NULL || d2 == NULL)
        return FALSE;
    if (d1->type != d2->type)
        return FALSE;
    switch (d1->type) 
    {
        case G_TYPE_STRING:
            return g_strcmp0(d1->string,d2->string) == 0;
        case G_TYPE_INT:
            return d1->iNumber == d2->iNumber;
        case G_TYPE_BOOLEAN:
            return d1->booleen == d2->booleen;
        case CID_TYPE_SUBSTITUTE:
            return g_strcmp0(d1->sub->regex,d2->sub->regex) == 0
                   && g_strcmp0(d1->sub->replacement,d2->sub->replacement) == 0;
    }
}

void
cid_datatable_append(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL) 
    {
        CidDataCase *p_new = cid_datacase_new(); 
        if (p_new != NULL) 
        {
            p_new->content = data; 
            p_new->next = NULL; 
            if ((*p_list)->tail == NULL)
            {
                p_new->prev = NULL; 
                (*p_list)->head = p_new;
                (*p_list)->tail = p_new;
            }
            else
            {
                (*p_list)->tail->next = p_new;
                p_new->prev = (*p_list)->tail;
                (*p_list)->tail = p_new;
            }
            (*p_list)->length++;
        }
    }
}

void
cid_datatable_prepend(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_new = cid_datacase_new();
        if (p_new != NULL)
        {
            p_new->content = data;
            p_new->prev = NULL;
            if ((*p_list)->tail == NULL)
            {
                p_new->next = NULL;
                (*p_list)->head = p_new;
                (*p_list)->tail = p_new;
            }
            else
            {
                (*p_list)->head->prev = p_new;
                p_new->next = (*p_list)->head;
                (*p_list)->head = p_new;
            }
            (*p_list)->length++;
       }
    }
}

void
cid_datatable_insert(CidDataTable **p_list, CidDataContent *data, gint position)
{
    if (*p_list != NULL)
    {
        if (position < 0)
        {
            cid_datatable_prepend(p_list,data);
            return;
        }
        else 
        if (position > (gint) cid_datatable_length(*p_list))
        {
            cid_datatable_append(p_list,data);
            return;
        }
        CidDataCase *p_temp = (*p_list)->head;
        int i = 1;
        while (p_temp != NULL && i <= position)
        {
            if (position == i)
            {
                if (p_temp->next == NULL)
                {
                    cid_datatable_append(p_list, data);
                }
                else if (p_temp->prev == NULL)
                {
                    cid_datatable_prepend(p_list, data);
                }
                else
                {
                    CidDataCase *p_new = cid_datacase_new();
                    if (p_new != NULL)
                    {
                        p_new->content = data;
                        p_temp->next->prev = p_new;
                        p_temp->prev->next = p_new;
                        p_new->prev = p_temp->prev;
                        p_new->next = p_temp;
                        (*p_list)->length++;
                    }
                }
            }
            else
            {
                p_temp = p_temp->next;
            }
            i++;
        }
    }
}

void
cid_free_datacase_full (CidDataCase *pCase, gpointer *pData)
{
    if (pCase != NULL)
    {
        cid_free_datacontent(pCase->content);
        g_free(pCase);
    }
}

void
cid_free_datacontent_full (CidDataContent *pContent, gpointer *pData)
{
    if (pContent != NULL)
    {
        if (pContent->type == G_TYPE_STRING && pContent->string != NULL)
            g_free(pContent->string);
        if (pContent->type == CID_TYPE_SUBSTITUTE)
            cid_free_substitute (pContent->sub);
        g_free(pContent);
    }
}

void
cid_free_datatable (CidDataTable **p_list)
{
    if (*p_list != NULL)
    {
        cid_datatable_foreach(*p_list,(CidDataAction) cid_free_datacase_full, NULL);
        g_free(*p_list), *p_list = NULL;
    }
}

void
cid_datatable_foreach (CidDataTable *p_list, CidDataAction func, gpointer *pData)
{
    if (p_list != NULL)
    {
        CidDataCase *p_temp = p_list->head;
        gboolean bCreateData = FALSE;
        if (pData == NULL)
        {
            pData = g_new(gpointer,1);
            bCreateData = TRUE;
        }
        gint cpt = 1;
        while (p_temp != NULL)
        {
            pData[0] = GINT_TO_POINTER(cpt);
            CidDataCase *p_del = g_malloc0(sizeof(*p_temp));
            memcpy(p_del,p_temp,sizeof(*p_temp));
            func (p_temp, pData);
            p_temp = p_del->next;
            g_free(p_del);
            cpt++;
        }
        if (bCreateData)
        {
            g_free(pData);
        }
    }
}

void
cid_datacase_print (CidDataCase *pCase, gpointer *pData)
{
    if (pCase != NULL && pCase->content != NULL)
    {
        switch (pCase->content->type) 
        {
            case G_TYPE_STRING:
                fprintf (stdout,"%s\n",pCase->content->string);
                break;
            case G_TYPE_INT:
                fprintf (stdout,"%d\n",pCase->content->iNumber);
                break;
            case G_TYPE_BOOLEAN:
                fprintf (stdout,"%s\n",pCase->content->booleen ? "TRUE" : "FALSE");
                break;
            case CID_TYPE_SUBSTITUTE:
                fprintf (stdout,"%s>%s\n",pCase->content->sub->regex,
                                          pCase->content->sub->replacement);
                break;
        }
    }
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
            *c_tmp = g_strdup_printf ("%s%s%s",tmp,pCase->content->string,pData[3]);
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
cid_datatable_remove(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_temp = (*p_list)->head;
        gboolean found = FALSE;
        while (p_temp != NULL && !found)
        {
            if (cid_datacontent_equals(p_temp->content,data))
            {
                if (p_temp->next == NULL)
                {
                    cid_free_datacase((*p_list)->tail);
                    (*p_list)->tail = p_temp->prev;
                    cid_free_datacase((*p_list)->tail->next);
                    (*p_list)->tail->next = NULL;
                }
                else if (p_temp->prev == NULL)
                {
                    cid_free_datacase((*p_list)->head);
                    (*p_list)->head = p_temp->next;
                    cid_free_datacase((*p_list)->tail->prev);
                    (*p_list)->head->prev = NULL;
                }
                else
                {
                    p_temp->next->prev = p_temp->prev;
                    p_temp->prev->next = p_temp->next;
                }
                cid_free_datacase(p_temp);
                (*p_list)->length--;
                found = TRUE;
            }
            else
            {
                p_temp = p_temp->next;
            }
        }
    }
}

void
cid_datatable_remove_all(CidDataTable **p_list, CidDataContent *data)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_temp = (*p_list)->head;
        while (p_temp != NULL)
        {
            if (cid_datacontent_equals(p_temp->content,data))
            {
                CidDataCase *p_del = p_temp;
                p_temp = p_temp->next;
                if (p_del->next == NULL)
                {
                    (*p_list)->tail = p_del->prev;
                    (*p_list)->tail->next = NULL;
                }
                else if (p_del->prev == NULL)
                {
                    (*p_list)->head = p_del->next;
                    (*p_list)->head->prev = NULL;
                }
                else
                {
                    p_del->next->prev = p_del->prev;
                    p_del->prev->next = p_del->next;
                }
                cid_free_datacase(p_del);
                (*p_list)->length--;
            }
            else
            {
                p_temp = p_temp->next;
            }
        }
    }
}

void
cid_datatable_remove_id(CidDataTable **p_list, gint position)
{
    if (*p_list != NULL)
    {
        CidDataCase *p_temp = (*p_list)->head;
        int i = 1;
        while (p_temp != NULL && i <= position)
        {
            if (position == i)
            {
                if (p_temp->next == NULL)
                {
                    (*p_list)->tail = p_temp->prev;
                    (*p_list)->tail->next = NULL;
                }
                else if (p_temp->prev == NULL)
                {
                    (*p_list)->head = p_temp->next;
                    (*p_list)->head->prev = NULL;
                }
                else
                {
                    p_temp->next->prev = p_temp->prev;
                    p_temp->prev->next = p_temp->next;
                }
                cid_free_datacase(p_temp);
                (*p_list)->length--;
            }
            else
            {
                p_temp = p_temp->next;
            }
            i++;
        }
    }
}

size_t
cid_datatable_length(CidDataTable *p_list)
{
    size_t ret = 0;
    if (p_list != NULL)
    {
        ret = p_list->length;
    }
    return ret;
}

CidDataTable *
cid_create_datatable (GType iDataType, ...)
{
    CidDataTable *res = cid_datatable_new();
    GType iCurrType = iDataType;
    va_list args;
    va_start(args,iDataType);
    void *current;
    while ((GType) (current = va_arg(args,gpointer)) != G_TYPE_INVALID) {
        CidDataContent *tmp = NULL;
        if ((GType) current == G_TYPE_BOOLEAN || (GType) current == G_TYPE_INT 
            || (GType) current == G_TYPE_STRING || (GType) current == CID_TYPE_SUBSTITUTE)
        {
            iCurrType = (GType) current;
            continue;
        }
        switch (iCurrType) 
        {
            case G_TYPE_BOOLEAN:
                tmp = cid_datacontent_new_boolean(current);
                break;
            case G_TYPE_STRING:
                tmp = cid_datacontent_new_string(current);
                break;
            case G_TYPE_INT:
                tmp = cid_datacontent_new_int(current);
                break;
            case CID_TYPE_SUBSTITUTE:
                tmp = cid_datacontent_new_substitute(current);
                break;
            default:
                iCurrType = (GType) current;
        }
        cid_datatable_append(&res,tmp);
    }
    va_end(args);
    return res;
}

CidDataTable *
cid_create_sized_datatable_with_default_full (size_t iSize, GType iType, void *value)
{
    size_t cpt = 0;
    CidDataTable *res = cid_datatable_new();
    for (;cpt<iSize;cpt++)
    {
        CidDataContent *tmp = cid_datacontent_new(iType, value);
        cid_datatable_append(&res,tmp);
    }
    return res;
}

void
cid_str_replace_all (gchar **string, const gchar *sFrom, const gchar *sTo)
{
    if (*string == NULL || sFrom == NULL || sTo == NULL)
        return;
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
        cid_free_datatable(&t_temp);
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
    cid_free_datatable(&t_temp);
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
cid_check_position (void) 
{
    cid_get_X_infos();
    if (cid->config->iPosX > (cid->XScreenWidth - cid->config->iWidth)) 
        cid->config->iPosX = (cid->XScreenWidth - cid->config->iWidth);
    if (cid->config->iPosY > (cid->XScreenHeight - cid->config->iHeight)) 
        cid->config->iPosY = (cid->XScreenHeight - cid->config->iHeight);
    if (cid->config->iPosX < 0) 
        cid->config->iPosX = 0;
    if (cid->config->iPosY < 0) 
        cid->config->iPosY = 0;
}

void 
cid_decrypt_string( const gchar *cEncryptedString,  gchar **cDecryptedString )
{
    if( !cEncryptedString || *cEncryptedString == '\0' )
    {
        *cDecryptedString = g_strdup( "" );
        return;
    }
#ifdef HAVE_LIBCRYPT
    gchar *input = g_strdup(cEncryptedString);
    gchar *shifted_input = input;
    gchar **output = cDecryptedString; 

    gchar *current_output = NULL;
    if( output && input && strlen(input)>0 )
    {
        *output = g_malloc( (strlen(input)+1)/3+1 );
        current_output = *output;
    }
    else
    {
        if( input )
        {
            g_free(input);
        }
        return;
    }

    gchar *last_char_in_input = input + strlen(input);
//  g_print( "Password (before decrypt): %s\n", input );

    for( ; shifted_input < last_char_in_input; shifted_input += 16+8, current_output += 8 )
    {
        guint block[8];
        gchar txt[64];
        gint i = 0, j = 0;
        gchar current_letter = 0;
    
        memset( txt, 0, 64 );

        shifted_input[16+8-1] = 0; // cut the string

        sscanf( shifted_input, "%X-%X-%X-%X-%X-%X-%X-%X",
        &block[0], &block[1], &block[2], &block[3], &block[4], &block[5], &block[6], &block[7] );

        // process the eight first characters of "input"
        for( i = 0; i < 8 ; i++ )
            for ( j = 0; j < 8; j++ )
                txt[i*8+j] = block[i] >> j & 1;
    
        setkey( DES_crypt_key );
        encrypt( txt, 1 );  // decrypt

        for ( i = 0; i < 8; i++ )
        {
            current_output[i] = 0;
            for ( j = 0; j < 8; j++ )
            {
                current_output[i] |= txt[i*8+j] << j;
            }
        }
    }

    *current_output = 0;

//  g_print( "Password (after decrypt): %s\n", *output );

    g_free( input );

#else
    *cDecryptedString = g_strdup( cEncryptedString );
#endif
}

void 
cid_encrypt_string( const gchar *cDecryptedString,  gchar **cEncryptedString )
{
    if( !cDecryptedString || strlen(cDecryptedString) == 0 )
    {
        *cEncryptedString = g_strdup( "" );
        return;
    }
    
#ifdef HAVE_LIBCRYPT
    const gchar *input = cDecryptedString;
    gchar **output = cEncryptedString;
    guint input_length = 0;

    gchar *current_output = NULL;
    if( output && input && strlen(input)>0 )
    {
        // for each block of 8 characters, we need 24 bytes.
        guint nbBlocks = strlen(input)/8+1;
    
        *output = g_malloc( nbBlocks*24+1 );
        current_output = *output;
    }
    else
    {
        return;
    }

    const gchar *last_char_in_input = input + strlen(input);

//  g_print( "Password (before encrypt): %s\n", input );

    for( ; input < last_char_in_input; input += 8, current_output += 16+8 )
    {
        gchar txt[64];
        gint i = 0, j = 0;
        gchar current_letter = 0;
    
        memset( txt, 0, 64 );
    
        // process the eight first characters of "input"
        for( i = 0; i < strlen(input) && i < 8 ; i++ )
            for ( j = 0; j < 8; j++ )
                txt[i*8+j] = input[i] >> j & 1;
    
        setkey( DES_crypt_key );
        encrypt( txt, 0 );  // encrypt

        for ( i = 0; i < 8; i++ )
        {
            current_letter = 0;
            for ( j = 0; j < 8; j++ )
            {
                current_letter |= txt[i*8+j] << j;
            }
            snprintf( current_output + i*3, 4, "%02X-", current_letter );
        }
    }

    *(current_output-1) = 0;

//  g_print( "Password (after encrypt): %s\n", *output );
#else
    *cEncryptedString = g_strdup( cDecryptedString );
#endif
}

static gboolean
eval_cb (const GMatchInfo *info,
         GString          *res,
         gpointer          data)
{
    gchar *match;
    gchar *r;

    match = g_match_info_fetch (info, 0);
    r = g_hash_table_lookup ((GHashTable *)data, match);
    g_string_append (res, r);
    g_free (match);

    return FALSE;
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
    cid_free_datatable (&table);
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
    
