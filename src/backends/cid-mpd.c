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
   *                               cid-mpd.c
   *                                -------
   *                          Conky Images Display
   *             --------------------------------------------
   *
*/

#include "cid-mpd.h"

#include "../cid-struct.h"
#include "../cid-asynchrone.h"
#include "../cid-constantes.h"
#include "../cid-messages.h"
#include "../cid-cover.h"

#include "../tools/cid-utilities.h"

#include "../cid-callbacks.h"

extern CidMainContainer *cid;

static gboolean cont, first, paused = FALSE;
static mpd_Connection *conn = NULL;

gboolean
cid_mpd_cover ()
{
    mpd_Status *status;
    mpd_InfoEntity *entity;
    // On ouvre une connexion s'il n'en existe pas deja une
    if (!conn)
    {
        cid_debug ("Connecting to MPD");
        conn = mpd_newConnection (cid->mpd_host,cid->mpd_port, 10);
        
        if (cid->mpd_pass != NULL && strcmp (cid->mpd_pass, "") != 0)
        {
            mpd_sendPasswordCommand (conn,cid->mpd_pass);
            mpd_finishCommand (conn); 
        }
    }
    
    // On verifie qu'il n'y ait pas d'erreurs
    if (conn->error)
    {
        cid_warning ("MPD error: %s",conn->errorStr);
        mpd_closeConnection (conn);
        conn = NULL;
        musicData.opening = FALSE;
        musicData.playing = FALSE;
        cid_set_state_icon ();
        cid_display_image (cid->config->cDefaultImage);
        return cont;
    }
    
    mpd_sendStatusCommand (conn);
    
    if ((status = mpd_getStatus (conn)) == NULL)
    {
        cid_warning ("MPD error: %s",conn->errorStr);
        mpd_closeConnection(conn);
        conn = NULL;
        musicData.opening = FALSE;
        musicData.playing = FALSE;
        cid_set_state_icon ();
        cid_display_image (cid->config->cDefaultImage);
        return cont;
    }
    
    mpd_finishCommand (conn);
    
    if (conn->error)
    {
        mpd_closeConnection (conn);
        conn = NULL;
        mpd_freeStatus(status);
        return cont;
    }
    
    musicData.opening = TRUE;
    
    gboolean bOldState = musicData.playing;
    musicData.playing = (status->state == MPD_STATUS_STATE_PLAY);
    
    if (status->state == MPD_STATUS_STATE_STOP || 
        status->state == MPD_STATUS_STATE_UNKNOWN)
    {
        if (bOldState!=musicData.playing || first)
        {
            paused = bOldState!=musicData.playing;
            musicData.opening = FALSE;
            first = FALSE;
            mpd_freeStatus(status);
            cid_set_state_icon();
            cid_display_image (cid->config->cDefaultImage);
        }
        return cont;
    }
    
    first = FALSE;
    
    mpd_freeStatus(status);
    
    if (bOldState!=musicData.playing)
        cid_set_state_icon();
    
    if (conn->error)
    {
        mpd_closeConnection (conn);
        conn = NULL;
        return cont;
    }
    
    mpd_sendCurrentSongCommand (conn);
    while ((entity = mpd_getNextInfoEntity(conn)))
    {
        mpd_Song *song = entity->info.song;

        if (entity->type != MPD_INFO_ENTITY_TYPE_SONG) {
            mpd_freeInfoEntity(entity);
            continue;
        }
        
        if (musicData.playing_uri != NULL && 
            strcmp (musicData.playing_uri,song->file) == 0 && !paused) 
        {
            if (bOldState!=musicData.playing)
                cid_display_image (musicData.playing_cover);
            return cont;
        }
        
        g_free (musicData.playing_artist);
        if (song->artist)
            musicData.playing_artist = g_strdup (song->artist);
        else
            musicData.playing_artist = NULL;
        
        g_free (musicData.playing_album);
        if (song->album)
            musicData.playing_album = g_strdup (song->album);
        else
            musicData.playing_album = NULL;
        
        g_free (musicData.playing_title);
        if (song->title)
            musicData.playing_title = g_strdup (song->title);
        else
            musicData.playing_title = NULL;
        
        g_free (musicData.playing_uri);
        if (song->file)
            musicData.playing_uri = g_strdup (song->file);
        else
            musicData.playing_uri = NULL;
        
        if (entity != NULL) 
        {
            mpd_freeInfoEntity(entity);
            entity = NULL;
        }
    }
    if (entity != NULL) 
    {
        mpd_freeInfoEntity (entity);
        entity = NULL;
    }
    mpd_finishCommand (conn);
    
    gchar *cSongPath = NULL;
    
    if (cid->mpd_dir != NULL)
    {
        gchar *tmp = 
            g_strdup_printf ("%s%s/%s", 
                             strncmp (cid->mpd_dir, "file://", 7) == 0 ? 
                                "" : 
                                "file://", 
                             cid->mpd_dir, 
                             musicData.playing_uri);
        cSongPath = g_filename_from_uri (tmp, NULL, NULL);
        g_free (tmp);
    }
    
    if (cSongPath != NULL)
    {
        gchar *cSongDir = g_path_get_dirname (cSongPath);
        g_free (cSongPath);
        g_free (musicData.playing_cover);
        musicData.playing_cover = cid_cover_lookup (&cid, 
                                              musicData.playing_artist, 
                                              musicData.playing_album,
                                              cSongDir);
        g_free (cSongDir);
    } else {
        cid_debug ("l'image n'existe pas => on va la chercher.");
        g_free (musicData.playing_cover);
        musicData.playing_cover = cid_cover_lookup (&cid, 
                                              musicData.playing_artist, 
                                              musicData.playing_album,
                                              NULL);
    }
    
    cid_info (
"\nartist : %s\nalbum : %s\ntitle : %s\nsong uri : %s\ncover uri : %s",
              musicData.playing_artist,
              musicData.playing_album,
              musicData.playing_title,
              musicData.playing_uri,
              musicData.playing_cover);
    
    cid_display_image (musicData.playing_cover);
    cid_animation(cid->config->iAnimationType);
    
    return cont;
}

void 
cid_mpd_pipe (gint iInter) 
{
    cid->runtime->iPipe = 
            g_timeout_add_full (G_PRIORITY_HIGH, 
                                iInter,
                                (gpointer) cid_mpd_cover, 
                                NULL, 
                                NULL);
}

void 
cid_connect_to_mpd (gint iInter)
{
    cont = TRUE;
    first = TRUE;
    cid->runtime->bPipeRunning = TRUE;
    cid_mpd_cover ();
    cid->runtime->bConnected = TRUE;
    cid_mpd_pipe (iInter);   
}

void 
cid_disconnect_from_mpd ()
{
    cont = FALSE;
    cid->runtime->bConnected = FALSE;
    if (cid->runtime->bPipeRunning)
        g_source_remove (cid->runtime->iPipe);
    cid->runtime->bPipeRunning = FALSE;
    musicData.opening = FALSE;
    musicData.playing = FALSE;
    cid_display_image (NULL);
}

static void
cid_reconnect_mpd (gint iInter)
{
    cid_connect_to_mpd (iInter);
    cid_display_image (musicData.playing_cover);
}

void 
_playPause_mpd (CidMainContainer **pCid) 
{
    if (conn)
    {
        mpd_sendPauseCommand (conn,musicData.playing);
        mpd_finishCommand (conn);
        cid_mpd_cover ();
    }
}

void 
_next_mpd (CidMainContainer **pCid) 
{
    if (conn)
    {
        mpd_sendNextCommand (conn);
        mpd_finishCommand (conn);
        cid_mpd_cover ();
    }
}

void 
_previous_mpd (CidMainContainer **pCid) 
{
    if (conn)
    {
        mpd_sendPrevCommand (conn);
        mpd_finishCommand (conn);
        cid_mpd_cover ();
    }
}

void 
cid_build_mpd_menu (void)
{
    cid->runtime->pMonitorList->p_fPlayPause = _playPause_mpd;
    cid->runtime->pMonitorList->p_fNext = _next_mpd;
    cid->runtime->pMonitorList->p_fPrevious = _previous_mpd;
    cid->runtime->pMonitorList->p_fAddToQueue = NULL;
    cid->p_fConnectHandler = cid_reconnect_mpd;
    cid->p_fDisconnectHandler = cid_disconnect_from_mpd;
}
