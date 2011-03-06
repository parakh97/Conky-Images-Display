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

static void 
cid_init (CidMainContainer *pCid) 
{    
    
    pCid->config->cVerbosity = NULL;
    
    pCid->config->bTesting = FALSE;
    
    pCid->runtime->dAngle = 0;
    
    pCid->runtime->iCurrentlyDrawing = 0;
    
    pCid->config->cConfFile = g_strdup_printf(CID_CONFIG_DIR,g_getenv("HOME"),CID_CONFIG_FILE);

#ifdef HAVE_E17    
    pCid->config->iHint = GDK_WINDOW_TYPE_HINT_DESKTOP; 
#elif HAVE_COMPIZ
    pCid->config->iHint = GDK_WINDOW_TYPE_HINT_DOCK;
#else
    pCid->config->iHint = GDK_WINDOW_TYPE_HINT_TOOLTIP;
#endif
    
    pCid->defaut->cDLPath = g_strdup_printf(CID_COVER_DIR,g_getenv("HOME"));
    
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
                cid_debug ("dbus connected");
                // On peut afficher directement le retour de cid_amarok_2_cover puisque
                // la chaine retournee sera liberee au prochain appel.
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
                cid_debug ("dbus connected");
                // On peut afficher directement le retour de cid_rhythmbox_cover puisque
                // la chaine retournee sera liberee au prochain appel.
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
            cid_exit (pCid, CID_PLAYER_ERROR,"ERROR: \"%d\" is not recognized as a supported player",cid->config->iPlayer);
    }
}

/* Methode initialisant les signaux à intercepter */
static void 
cid_set_signal_interception (struct sigaction *action) 
{
    (*action).sa_handler = cid_intercept_signal;
    sigfillset (&((*action).sa_mask));
    (*action).sa_flags = 0;
    CidDataTable *p_signals = cid_create_datatable(G_TYPE_INT,
                                                   SIGSEGV,
                                                   SIGFPE,
                                                   SIGILL,
                                                   SIGABRT,
                                                   SIGINT,
                                                   SIGTERM,
                                                   G_TYPE_INVALID);
    BEGIN_FOREACH_DT(p_signals)
        // p_temp est declare par BEGIN_FOREACH_DT
        if (sigaction (p_temp->content->iNumber, action, NULL) != 0)
        {
            cid_error ("Problem while catching signal %d (%s)",
                       p_temp->content->iNumber,
                       strsignal(p_temp->content->iNumber));
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

    struct sigaction action;
    
    cid = g_malloc0 (sizeof(*cid));
    cid->config = g_malloc0 (sizeof(*(cid->config)));
    cid->runtime = g_malloc0 (sizeof(*(cid->runtime)));
    cid->defaut = g_malloc0 (sizeof(*(cid->defaut)));
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    cid_log_set_level(0);
    
    cid_init(cid);
    
    // On internationalise l'appli.
    setlocale (LC_ALL,"");
    bindtextdomain (CID_GETTEXT_PACKAGE, CID_LOCALE_DIR);
    bind_textdomain_codeset (CID_GETTEXT_PACKAGE, "UTF-8");
    textdomain (CID_GETTEXT_PACKAGE);

    cid_read_parameters (&cid, &argc,&argv);
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
    cid_display_init (&cid,&argc,&argv);
    
    // Si on est ici, c'est qu'on a coupé la boucle GTK
    // Du coup, on en profite pour faire un peu de ménage
    // histoire de pas laisser la baraque dans un sale etat
    cid_key_file_free(&cid);
    cid_free_musicData();
    cid_free_main_structure (cid);

/*
    GError *error = NULL;
    GMatchInfo *match_info;
    GRegex *reg = g_regex_new ("%al",0,0,&error);
    gchar *init = "blah %al bloh";
    gchar *replacement = "blih";
    if (error != NULL)
    {
        fprintf (stderr,"g_regex_new error: %s\n",error->message);
        g_error_free (error);
        error = NULL;
    }
    g_regex_match (reg, init, 0, &match_info);
    while (g_match_info_matches (match_info))
    {
        gchar *word = g_match_info_fetch (match_info, 0);
        g_print ("Found: %s\n", word);
        g_free (word);
        g_match_info_next (match_info, NULL);
    }
    g_match_info_free (match_info);
    gchar *res = g_regex_replace (reg, init, -1, 0, replacement, 0, &error);
    if (error != NULL)
    {
        fprintf (stderr,"g_regex_replace error: %s\n",error->message);
        g_error_free (error);
        error = NULL;
    }
    g_print ("before: %s\nafter: %s\n",init,res);
    g_free (res);
    g_regex_unref (reg);
*/
/*
    gchar *test = g_strdup ("1 => 2");
    g_print ("(%d)> %s\n", strlen(test),test);
    cid_substitute_user_params (&test);
    g_print ("(%d)> %s\n", strlen(test),test);
*/
/*
    //g_type_init ();
    
    gchar *test = g_strdup("bonjour %user%, comment vas-tu ?\n%home%\nartist:%artist%");
    g_print ("avant: %s (%d)\n",test,strlen(test));
    //cid_str_replace_all (&test,"%user%",g_getenv("USER"));
    cid_substitute_user_params (&test);
    fprintf (stdout,"après: %s (%d)\n",test,strlen(test));
    
    g_free (test);
*/    
/*
    int i = 1;
    for (;i<argc;i++)
    {
        gchar *tmp = g_strdup (argv[i]);
        fprintf (stdout,"avant: %s\n",tmp);
        cid_parse_nl (&tmp);
        fprintf (stdout,"après: %s\n",tmp);
        g_free (tmp);
    }
*/
    
    fprintf (stdout,"Bye !\n");    

    return ret;
    
}

