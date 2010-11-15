/*****************************************************************************************************
**
** Program:
**    Conky Images Display
**
** License :
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License, version 2.
**    If you don't know what that means take a look at:
**       http://www.gnu.org/licenses/licenses.html#GPL
**
** Original idea :
**    Charlie MERLAND, July 2008.
**
*************************************************************
** Authors:
**    Charlie MERLAND
**    Benjamin SANS <sans_ben@yahoo.fr>
**
** Notes :
**    Initially developed to use it with conky to display amarok's cover 
**    on desktop.
**    The project was re-written for rhythmbox.
**    In the end, we deceided to merge our two programs with a DBus
**    support to add amarok2 and other players in the future...
**
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details. 
**
*******************************************************************************/
/*
   *
   *                                 cid.c
   *                                -------
   *                          Conky Images Display
   *             --------------------------------------------
   *
*/

#include "cid.h"

CidMainContainer *cid;
int ret = CID_EXIT_SUCCESS;

static gchar *cLaunchCommand = NULL;

static void 
cid_init (CidMainContainer *pCid) 
{    
    
    pCid->config->cVerbosity = NULL;
    
    pCid->config->bTesting = FALSE;
    
    pCid->runtime->dAngle = 0;
    
    pCid->runtime->iCurrentlyDrawing = 0;
    
    pCid->config->cConfFile = g_strdup_printf("%s/.config/cid/%s",g_getenv("HOME"),CID_CONFIG_FILE);

#ifdef HAVE_E17    
    pCid->config->iHint = GDK_WINDOW_TYPE_HINT_DESKTOP; 
#else
//    pCid->config->iHint = GDK_WINDOW_TYPE_HINT_DOCK;
    pCid->config->iHint = GDK_WINDOW_TYPE_HINT_TOOLTIP;
#endif
    
    pCid->defaut->cDLPath = g_strdup_printf("%s/.cache/cid",g_getenv("HOME"));
    
    pCid->pKeyFile = NULL;
}

/* Methode appelée pour relancer cid en cas de plantage */
static void 
cid_intercept_signal (int number) 
{
    if (number == SIGINT || number == SIGTERM)
    {
        cid_interrupt ();
        return;
    }
    cid_warning ("Attention : cid has crashed (%s).", strsignal(number));
    //execl ("/bin/sh", "/bin/sh", "-c", cLaunchCommand, NULL);  // on ne revient pas de cette fonction.
    //cid_warning ("Sorry, couldn't restart cid");
    fflush (stdout);
    signal (number, SIG_DFL);
    raise (number);
}

void 
cid_run_with_player (CidMainContainer **pCid) 
{
    CidMainContainer *cid = *pCid;
    if (cid->config->iPlayer != PLAYER_NONE)
        cid->runtime->pMonitorList = g_new0 (CidControlFunctionsList,1);
    /* On lance telle ou telle fonction selon le lecteur selectionne */
    switch (cid->config->iPlayer) 
    {
        /* Amarok 1.4 */
        case PLAYER_AMAROK_1:
            cid_build_amarok_menu (pCid);
            cid_connect_to_amarok(pCid, cid->config->iInter);
            break;
        /* Amarok 2 */
        case PLAYER_AMAROK_2:
            cid_build_amarok_2_menu ();
            if (amarok_2_dbus_connect_to_bus()) 
            {
                cid_debug ("\ndbus connected\n");
                cid_display_image((gchar *)cid_amarok_2_cover());
            } 
            else 
            {
                cid_exit (pCid, CID_EXIT_ERROR,"\nFailed to connect dbus...\n");
            }
            break;
        /* Rhythmbox */
        case PLAYER_RHYTHMBOX:
            cid_build_rhythmbox_menu ();
            /* Initialisation de DBus */
            if (rhythmbox_dbus_connect_to_bus()) 
            {
                cid_debug ("\ndbus connected\n");
                cid_display_image((gchar *)cid_rhythmbox_cover());
            } 
            else 
            {
                cid_exit (pCid, CID_EXIT_ERROR,"\nFailed to connect dbus...\n");
            }
            break;
        /* Exaile */
        case PLAYER_EXAILE:
            cid_build_exaile_menu ();
            cid_connect_to_exaile(cid->config->iInter);
            break;
        /* MPD */
        case PLAYER_MPD:
            cid_build_mpd_menu ();
            cid_connect_to_mpd (cid->config->iInter);
            break;
        /* None */
        case PLAYER_NONE:
            cid_display_image(NULL);
            break;
        /* Sinon, on a un lecteur inconnu */
        default:
            cid_exit (pCid, CID_PLAYER_ERROR,"ERROR: \"%d\" is not recognize as a supported player\n",cid->config->iPlayer);
    }
}

/* Methode initialisant les signaux à intercepter */
static void 
cid_set_signal_interception (struct sigaction *action) 
{
    /*
    signal (SIGSEGV, cid_intercept_signal);  // Segmentation violation
    signal (SIGFPE, cid_intercept_signal);  // Floating-point exception
    signal (SIGILL, cid_intercept_signal);  // Illegal instruction
    signal (SIGABRT, cid_intercept_signal);  // Abort
    */
    (*action).sa_handler = cid_intercept_signal;
    sigfillset (&((*action).sa_mask));
    (*action).sa_flags = 0;
    CidDataTable *p_signals = cid_create_datatable(G_TYPE_INT,SIGSEGV,SIGFPE,SIGILL,SIGABRT,SIGINT,SIGTERM,G_TYPE_INVALID);
    BEGIN_FOREACH_DT(p_signals)
        if (sigaction (p_temp->content->iNumber, action, NULL) != 0)
        {
            cid_error ("Problem while catching signal %d",p_temp->content->iNumber);
        }
    END_FOREACH_DT
}

static void 
cid_display_init(CidMainContainer **pCid, int *argc, char ***argv) 
{
    CidMainContainer *cid = *pCid;
    
    /* Initialisation de Gtk */
    if (!cid->runtime->bRunning)
        cid->runtime->bRunning = gtk_init_check(argc, argv);
    if (!cid->runtime->bRunning)
        cid_exit (pCid, CID_GTK_ERROR,"Unable to load gtk context");
    
    /* On intercepte les signaux */
//    signal (SIGINT, cid_interrupt); // ctrl+c
//    signal (SIGTERM, cid_interrupt);

    if (cid->config->bSafeMode) 
    {
        _cid_conf_panel(NULL,NULL);
    }
    
    if (cid->config->bConfigPanel)
    {
        exit (CID_EXIT_SUCCESS);
    }
    
    /* On créé la fenêtre */
    cid_create_main_window();
    
    /* On lance le monitoring du player */
    cid_run_with_player(&cid);  
        
    /* Enfin on lance la boucle Gtk */
    gtk_main();
}

/* Fonction principale */

int 
main ( int argc, char **argv ) 
{        

/// TODO: debug
/*
    int argcBis = argc, a=0;
    char **argvBis = calloc(argc,sizeof(char));
    if (argvBis==NULL)
        cid_exit(CID_EXIT_ERROR,"Error while copying args");
    for (;a<argc;a++) 
    {
        argvBis[a] = realloc (argvBis[a],strlen(argv[a])*sizeof(char));
        if (argvBis[a]!=NULL)
            strcpy(argvBis[a],argv[a]);
        else
            cid_exit(CID_EXIT_ERROR,"Error while copying args");
    }
*/
    //char **argvBis = malloc(sizeof(argv));
    //memcpy(argvBis,argv,sizeof(argv));
    struct sigaction action;

    cid = g_malloc0 (sizeof(*cid));
    cid->config = g_malloc0 (sizeof(*(cid->config)));
    cid->runtime = g_malloc0 (sizeof(*(cid->runtime)));
    cid->defaut = g_malloc0 (sizeof(*(cid->defaut)));
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    int i;
    GString *sCommandString = g_string_new (argv[0]);
    for (i = 1; i < argc; i ++) 
    {
        g_string_append_printf (sCommandString, " %s", argv[i]);
    }
    g_string_append_printf (sCommandString, " -s");
    cLaunchCommand = sCommandString->str;
    g_string_free (sCommandString, FALSE);
    
    cid_log_set_level(0);
    
    cid_init(cid);

    // On internationalise l'appli.
    setlocale (LC_ALL,"");
    bindtextdomain (CID_GETTEXT_PACKAGE, CID_LOCALE_DIR);
    bind_textdomain_codeset (CID_GETTEXT_PACKAGE, "UTF-8");
    textdomain (CID_GETTEXT_PACKAGE);

    cid_read_parameters (&argc,&argv);
    cid_set_verbosity (cid->config->cVerbosity);
    
    cid_read_config (&cid, cid->config->cConfFile);
    cid->config->bChangedTestingConf = cid->config->bTesting && cid->config->bUnstable;
    
    cid_set_signal_interception (&action);
    
    if (!g_thread_supported ())
    { 
        g_thread_init(NULL); 
    }
    gdk_threads_init();

    // La on lance la boucle GTK
    //cid_display_init (&argc,&argvBis);
    cid_display_init (&cid,0,NULL);
    //free (argvBis);
    
    // Si on est ici, c'est qu'on a coupé la boucle GTK
    // Du coup, on en profite pour faire un peu de ménage
    // histoire de pas laisser la baraque dans un sale etat
    cid_key_file_free(&cid);
    cid_free_musicData();
    cid_free_main_structure (cid);

    fprintf (stdout,"Bye !\n");    

    return ret;
    
}

