/*
   *
   *                            cid-callbacks.c
   *                                -------
   *                          Conky Images Display
   *                    --------------------------------
   *
*/

//#include "cid.h"
#include "cid-callbacks.h"
#include "cid-gui-callback.h"
#include "cid-messages.h"
#include "cid-struct.h"
#include "cid-gui-factory.h"
#include "cid-cover.h"
#include "cid-config.h"
#include "cid-utilities.h"
#include "cid-asynchrone.h"
#include "cid-constantes.h"
#include "cid-draw.h"
#include "cid-menu-factory.h"

extern CidMainContainer *cid;
//extern gboolean bCurrentlyDownloading, bCurrentlyDownloadingXML, bCurrentlyFocused;

gboolean bFlyingButton;

//static gchar *cImageURL = NULL;
//static gint cpt = 0;
//static CidMeasure *pMeasureTimer = NULL;
static CidTask *pMeasureDownload = NULL;

void 
cid_interrupt (CidMainContainer **pCid) 
{
    _cid_quit(NULL,(gpointer *)pCid);
}

void 
_cid_quit (GtkWidget *p_widget, gpointer *user_data) 
{
    cid_save_data ((CidMainContainer **)user_data);

    cid_file_remove (DEFAULT_DOWNLOADED_IMAGE_LOCATION);
    
    cid_sortie ((CidMainContainer **)user_data, CID_EXIT_SUCCESS);

    /* Parametres inutilises */
    (void)p_widget;
}

gpointer 
_cid_launch_threaded (gchar *cCommand) 
{    
    if (!system (cCommand)) return NULL;
    g_free (cCommand);
    return NULL;
}

void 
on_clic (GtkWidget *p_widget, GdkEventButton* pButton) 
{

    // si on a un clic gauche, que le clic est maintenu, et qu'on se trouve dans la zone permettant le deplacement
    if (pButton->button == 1 && pButton->type == GDK_BUTTON_PRESS 
        && pButton->x > cid->config->iWidth-cid->config->iExtraSize && pButton->y < cid->config->iExtraSize) { // clic gauche
        // si on ne verouille pas la position
        if (!cid->config->bLockPosition) 
        {
            if (cid->config->bShowAbove)
                gtk_window_set_keep_below (GTK_WINDOW (cid->pWindow), TRUE);
            gtk_window_begin_move_drag (GTK_WINDOW (gtk_widget_get_toplevel (p_widget)),
                                        pButton->button,
                                        pButton->x_root,
                                        pButton->y_root,
                                        pButton->time); // alors on se deplace
        }
    } 
    else if (pButton->button == 1 && pButton->type == GDK_BUTTON_RELEASE) 
    {
        // on relache un clic gauche, donc on lance la fonction 'play/pause'/'next'/'previous'
        if (cid->config->bMonitorPlayer && cid->config->iPlayer != PLAYER_NONE) 
        {
            if (cid->config->bDisplayControl) 
            {
                if (pButton->x < cid->config->iPrevNextSize &&
                    pButton->y < (cid->config->iHeight + cid->config->iPrevNextSize)/2 &&
                    pButton->y > (cid->config->iHeight - cid->config->iPrevNextSize)/2) {
                    
                    cid->runtime->pMonitorList->p_fPrevious(&cid);
                    
                } 
                else if (pButton->x < (cid->config->iWidth + cid->config->iPlayPauseSize)/2 &&
                         pButton->x > (cid->config->iWidth - cid->config->iPlayPauseSize)/2 &&
                         pButton->y < (cid->config->iHeight + cid->config->iPlayPauseSize)/2 &&
                         pButton->y > (cid->config->iHeight - cid->config->iPlayPauseSize)/2) 
                {
                
                    cid->runtime->pMonitorList->p_fPlayPause(&cid);
                
                } 
                else if (pButton->x > cid->config->iWidth - cid->config->iPrevNextSize &&
                         pButton->y < (cid->config->iHeight + cid->config->iPrevNextSize)/2 &&
                         pButton->y > (cid->config->iHeight - cid->config->iPrevNextSize)/2) 
                {
                
                    cid->runtime->pMonitorList->p_fNext(&cid);
                
                }
            } 
            else 
            {
                cid->runtime->pMonitorList->p_fPlayPause(&cid);
            }
            if (pButton->x < cid->config->iExtraSize &&
                     pButton->y > (cid->config->iHeight - cid->config->iExtraSize) &&
                     cid->config->iPlayer == PLAYER_MPD)
            {
                if (cid->runtime->bConnected)
                    cid->p_fDisconnectHandler();
                else
                    cid->p_fConnectHandler(cid->config->iInter);
                CID_REDRAW;
            }
        }
    } 
    else if (pButton->button == 2 && pButton->type == GDK_BUTTON_RELEASE) 
    { // clic milieu
        // on relache un clic du milieu, donc on lance la fonction 'Next'
        if (cid->config->bMonitorPlayer && cid->config->iPlayer != PLAYER_NONE && !cid->config->bDisplayControl)
            cid->runtime->pMonitorList->p_fNext(&cid);
    } 
    else if (pButton->button == 3 && pButton->type == GDK_BUTTON_RELEASE)
    { //clic droit
        // On relache un clic droit, donc on affiche le menu contextuel
        /*
        if (cid->pMenu)
            gtk_widget_destroy (cid->pMenu);
        cid->pMenu = cid_build_menu ();  

        gtk_widget_show_all (cid->pMenu);

        gtk_menu_popup (GTK_MENU (cid->pMenu),
                NULL,
                NULL,
                NULL,
                NULL,
                1,
                gtk_get_current_event_time ());
        */
        cid_build_menu(&cid);        
    }
    
    (void)p_widget;
}

void 
on_dragNdrop_data_received (GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time,
                        gpointer userdata) 
{
    gchar *cReceivedData = (gchar *) seldata->data;
    g_return_if_fail (cReceivedData != NULL);
    int length = strlen (cReceivedData);
    if (cReceivedData[length-1] == '\n')
        cReceivedData[--length] = '\0';  // on vire le retour chariot final.
    if (cReceivedData[length-1] == '\r')
        cReceivedData[--length] = '\0';
        
    fprintf (stdout, "before: %s\n", cReceivedData);
        
    gchar **cReceivedDataList = g_strsplit(cReceivedData,"\n",0);
    //g_free (cReceivedData);
    
    
    while (*cReceivedDataList != NULL) 
    {
        gboolean isImage = g_str_has_suffix(*cReceivedDataList,"JPG")
                        || g_str_has_suffix(*cReceivedDataList,"jpg")
                        || g_str_has_suffix(*cReceivedDataList,"JPEG")
                        || g_str_has_suffix(*cReceivedDataList,"jpeg")
                        || g_str_has_suffix(*cReceivedDataList,"PNG")
                        || g_str_has_suffix(*cReceivedDataList,"png")
                        || g_str_has_suffix(*cReceivedDataList,"SVG")
                        || g_str_has_suffix(*cReceivedDataList,"svg");
        
        fprintf (stdout, "POUET: '%s' (%s)\n", *cReceivedDataList, isImage ? "TRUE" : "FALSE");
        if(isImage) 
        {
            if(musicData.playing_artist != NULL && musicData.playing_album != NULL) 
            {
                cid_debug("Le fichier est une image");
                GString *command = g_string_new ("");
                if(strncmp(*cReceivedDataList, "http://", 7) == 0) 
                {
                    cid_debug("Le fichier est distant");
                    g_string_printf (command, "wget -O /tmp/\"%s - %s.jpg\" %s",
                        musicData.playing_artist,
                        musicData.playing_album,
                        *cReceivedDataList);
                } 
                else 
                {
                    cid_debug("Le fichier est local");
                    gchar *cFileSrc = (**cReceivedDataList == '/' ? g_strdup(*cReceivedDataList) : g_filename_from_uri (*cReceivedDataList, NULL, NULL));
                    gchar *cFileDst = g_strdup_printf ("/tmp/\"%s - %s.jpg\"",musicData.playing_artist,musicData.playing_album);
                    cid_file_copy (cFileSrc,cFileDst);
                    g_free (cFileSrc);
                    g_free (cFileDst);
                }
                cid_launch_command (command->str);                
                g_string_free (command, TRUE);
                gchar *cTmpImagePath = g_strdup_printf("/tmp/%s - %s.jpg",musicData.playing_artist,musicData.playing_album);
                cid_display_image(cTmpImagePath);
                cid_animation(cid->config->iAnimationType);
                g_free (cTmpImagePath);
            } 
            else 
            {
                gchar *cTmpImagePath = (**cReceivedDataList == '/' ? g_strdup(*cReceivedDataList) : g_filename_from_uri (*cReceivedDataList, NULL, NULL));
                cid_display_image(cTmpImagePath);
                cid_animation(cid->config->iAnimationType);
                g_free (cTmpImagePath);
            }
            gtk_main_quit ();
            gtk_main ();
        } 
        else 
        {
            cid_debug("On ajoute à la playlist");
            if (cid->runtime->pMonitorList->p_fAddToQueue!=NULL)
                cid->runtime->pMonitorList->p_fAddToQueue(*cReceivedDataList);
        }
        g_free (*cReceivedDataList);
        cReceivedDataList++;
    }
        
    //g_print("d&d >>> %s\n",cReceivedData);
    //system (g_strdup_printf("nautilus %s &",cReceivedData));
    //g_strfreev (cReceivedDataList);
    g_free (cReceivedData);
}


void 
on_motion (GtkWidget *widget, GdkEventMotion *event) 
{
    
    cid->runtime->iCursorX = event->x;
    cid->runtime->iCursorY = event->y;
    
    
    if ((cid->runtime->iCursorX < cid->config->iPrevNextSize &&
        cid->runtime->iCursorY < (cid->config->iHeight + cid->config->iPrevNextSize)/2 &&
        cid->runtime->iCursorY > (cid->config->iHeight - cid->config->iPrevNextSize)/2)
    ||
        (cid->runtime->iCursorX > cid->config->iWidth - cid->config->iPrevNextSize &&
        cid->runtime->iCursorY < (cid->config->iHeight + cid->config->iPrevNextSize)/2 &&
        cid->runtime->iCursorY > (cid->config->iHeight - cid->config->iPrevNextSize)/2)
    ||
        (cid->runtime->iCursorX < (cid->config->iWidth + cid->config->iPlayPauseSize)/2 &&
        cid->runtime->iCursorX > (cid->config->iWidth - cid->config->iPlayPauseSize)/2 &&
        cid->runtime->iCursorY < (cid->config->iHeight + cid->config->iPlayPauseSize)/2 &&
        cid->runtime->iCursorY > (cid->config->iHeight - cid->config->iPlayPauseSize)/2)
    ||
        (cid->runtime->iCursorX < cid->config->iExtraSize &&
        cid->runtime->iCursorY > (cid->config->iHeight - cid->config->iExtraSize))) 
    {
        
        CID_REDRAW;
        bFlyingButton = TRUE;
    } 
    else if (bFlyingButton) 
    {
        bFlyingButton = FALSE;
        CID_REDRAW;
    }
}

void 
_cid_about (GtkMenuItem *pMenuItem, gpointer *data) 
{
    
    GtkWidget *pDialog = gtk_message_dialog_new (GTK_WINDOW (cid->pWindow),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_CLOSE,
        "\nConky Images Display (2008-2011)\n version %s",CID_VERSION);
    
#if GTK_MINOR_VERSION >= 12
    GtkWidget *pLink = gtk_link_button_new_with_label (CID_WEBSITE, "Conky Images Display (2008-2011)\n version "CID_VERSION);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), pLink);
#endif
    
    gchar *cImagePath = g_strdup (CID_DEFAULT_IMAGE);
    GtkWidget *pImage = gtk_image_new_from_file (cImagePath);
    g_free (cImagePath);
#if GTK_MINOR_VERSION >= 12
    gtk_message_dialog_set_image (GTK_MESSAGE_DIALOG (pDialog), pImage);
#endif
    GtkWidget *pNoteBook = gtk_notebook_new ();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (pNoteBook), TRUE);
    gtk_notebook_popup_enable (GTK_NOTEBOOK (pNoteBook));
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(pDialog)->vbox), pNoteBook);
    
    _cid_add_about_page (pNoteBook,
        "Development",
        _("<b>Main developers :</b>\n  Benjamin SANS\n  Charlie MERLAND\n\
<b>Original idea/first development :</b>\n  Charlie MERLAND\n\
<b>Translations :</b>\n  Benjamin SANS\n\
<b>Source references :</b>\n  some part of the code is inspired by Cairo-Dock\n\
<b>Special Thanks :</b>\n  Cairo-Dock's team\n"));
    
    _cid_add_about_page (pNoteBook,
        "Support",
        _("<b>Installation script :</b>\n  Benjamin SANS\n\
<b>Site ("CID_WEBSITE") :</b>\n  Charlie MERLAND\n\
<b>Any suggestion? Leave it on :</b>\n  "CID_WEBSITE"\n"));
    
    gtk_widget_show_all (pDialog);
    gtk_window_set_position (GTK_WINDOW (pDialog), GTK_WIN_POS_CENTER_ALWAYS);
    gtk_window_set_keep_above (GTK_WINDOW (pDialog), TRUE);
    gtk_dialog_run (GTK_DIALOG (pDialog));
    gtk_widget_destroy (pDialog);
}

static void 
_cid_add_about_page (GtkWidget *pNoteBook, const gchar *cPageLabel, const gchar *cAboutText) 
{
    GtkWidget *pVBox, *pScrolledWindow;
    GtkWidget *pPageLabel, *pAboutLabel;
    
    pPageLabel = gtk_label_new (cPageLabel);
    pVBox = gtk_vbox_new (FALSE, 0);
    pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pVBox);
    gtk_notebook_append_page (GTK_NOTEBOOK (pNoteBook), pScrolledWindow, pPageLabel);
    
    pAboutLabel = gtk_label_new (NULL);
    gtk_label_set_use_markup (GTK_LABEL (pAboutLabel), TRUE);
    gtk_box_pack_start (GTK_BOX (pVBox),
        pAboutLabel,
        FALSE,
        FALSE,
        0);
    gtk_label_set_markup (GTK_LABEL (pAboutLabel), cAboutText);
}

static gboolean 
_cid_check_and_display (gpointer *pSharedMemory)
{
    gchar *cArtist = pSharedMemory[0];
    gchar *cAlbum = pSharedMemory[1];
    CidMainContainer **pCid = pSharedMemory[2];
    
    // Quand on a la pochette, on l'affiche et on stoppe la boucle
    if (g_file_test (DEFAULT_DOWNLOADED_IMAGE_LOCATION, G_FILE_TEST_EXISTS)) 
    {
        cid_display_image(DEFAULT_DOWNLOADED_IMAGE_LOCATION);
        
        gchar *tmp;
        tmp = cid_db_store_cover (pCid, 
                         DEFAULT_DOWNLOADED_IMAGE_LOCATION,
                         cArtist,
                         cAlbum);
                         
        g_free (tmp);
    } 
    return FALSE;
}

static void 
_cid_proceed_download_cover (gpointer *pSharedMemory) 
{
    gchar *cArtist = pSharedMemory[0];
    gchar *cAlbum = pSharedMemory[1];
    gchar *cImageURL = NULL;
    CidMainContainer **pCid = pSharedMemory[2];
    
    // Avant tout, on dl le xml
    cid_get_xml_file (cArtist,cAlbum);

    // Quand on a le xml, on dl la pochette
    if (g_file_test (DEFAULT_XML_LOCATION, G_FILE_TEST_EXISTS)) 
    {
        cid_get_cover_url (DEFAULT_XML_LOCATION,&cImageURL);
        cid_debug ("URL : %s",cImageURL);
        if (cImageURL != NULL) 
        {
            cid_download_missing_cover (cImageURL);
            g_free (cImageURL);
        } 
        else 
        {
            cid_debug ("Téléchargement impossible");
        }
    }
}

static void 
_free_dl (gpointer *pSharedMemory)
{
    g_free (pSharedMemory[0]);
    g_free (pSharedMemory[1]);
    g_free (pSharedMemory);
}

gboolean 
_check_cover_is_present (CidMainContainer **pCid) 
{
    CidMainContainer *cid = *pCid;
    cid->runtime->iCheckIter++;
    if (g_file_test (musicData.playing_cover, G_FILE_TEST_EXISTS)) 
    {
        cid_display_image(musicData.playing_cover);
        musicData.cover_exist = TRUE;
        return FALSE;
    } 
    else if (cid->runtime->iCheckIter > cid->config->iTimeToWait) 
    {
        if (cid->config->bDownload)
        {
            if (pMeasureDownload != NULL)
            {
                cid_free_task (pMeasureDownload);
            }
            gpointer *pSharedMemory = g_new0 (gpointer, 3);
            pSharedMemory[0] = g_strdup (musicData.playing_artist);
            pSharedMemory[1] = g_strdup (musicData.playing_album);
            pSharedMemory[2] = pCid;
            pMeasureDownload = cid_new_task_full (1, 
                                                  (CidGetDataAsyncFunc)_cid_proceed_download_cover,
                                                  (CidUpdateSyncFunc)_cid_check_and_display, 
                                                  (GFreeFunc) _free_dl,
                                                  pSharedMemory);
            cid_launch_task (pMeasureDownload);
        }
        
        return FALSE;
    } 
    else 
    {
        return TRUE;
    }
}

void 
_cid_conf_panel (GtkMenuItem *pItemMenu, gpointer *data) 
{
    if (!cid->runtime->bConfFilePanel) 
    {
        cid_save_data(&cid);
        
        if (cid->pConfigPanel)
            gtk_widget_destroy (cid->pConfigPanel);
        
        cid_edit_conf_file_with_panel (NULL, 
                                       cid->config->cConfFile, 
                                       cid->config->bSafeMode && !cid->config->bConfigPanel ? 
                                            _(" < Maintenance Mode > ") : 
                                            _("CID Configuration Panel"), 
                                       CONFIG_WIDTH, 
                                       CONFIG_HEIGHT, 
                                       '\0', 
                                       NULL, 
                                       cid->config->bSafeMode ? 
                                            (CidReadConfigFunc) cid_read_config : 
                                            (CidReadConfigFunc) cid_read_config_after_update, 
                                       CID_GETTEXT_PACKAGE);
    }
}
