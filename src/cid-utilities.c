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

extern int ret;

/* Fonction de sortie en cas d'erreur, avec affichage d'un
   éventuel message d'erreur */
int 
cid_sortie(CidMainContainer **pCid, int code) 
{
    cid_disconnect_player (pCid);
    
    if ((*pCid)->runtime->bRunning)
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
    
    cid_free_datatable (&pCid->runtime->pCoversList);
    cid_free_datatable (&pCid->runtime->pImagesList);
    
    if (pCid->runtime->pLookupDirectory)
        g_dir_close (pCid->runtime->pLookupDirectory);
    
    g_free (pCid);
    pCid = NULL;
}
    
void 
cid_read_parameters (CidMainContainer **pCid, int *argc, char ***argv) 
{
    CidMainContainer *cid = *pCid;
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
#ifdef HAVE_E17
        env = g_strdup ("e17 support");
#elif HAVE_COMPIZ
        env = g_strdup ("compiz support");
#else
        env = g_strdup ("none");
#endif
        fprintf (stdout,"Version: %s\n"
                        "Options: %s\n"
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

void 
cid_check_position (CidMainContainer **pCid) 
{
    CidMainContainer *cid = *pCid;
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

