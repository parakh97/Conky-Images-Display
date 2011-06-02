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
   *                              amarok.c
   *                               -------
   *                         Conky Images Display
   *             --------------------------------------------
   *
*/

//#include "cid.h"
#include "cid-amarok.h"
#include "cid-struct.h"
#include "cid-messages.h"
#include "cid-utilities.h"
#include "cid-callbacks.h"
#include "cid-console-call.h"
#include "cid-constantes.h"
#include "cid-asynchrone.h"
#include "cid-cover.h"
#include "cid-draw.h"
#include "cid-animation.h"

#include <string.h>

//extern CidMainContainer *cid;

static gboolean cont;
gboolean run = FALSE;
//static CidMeasure *pMeasureTimer = NULL;
//extern gboolean bCurrentlyDownloading, bCurrentlyDownloadingXML;

///dcop amarok playlist addMediaList 
///[ "file:///home/benjamin/Music/aboutagirl.mp3" ]

gboolean 
get_amarock_musicData (CidMainContainer **pCid) 
{
    CidMainContainer *cid = *pCid;
    CIDError *error = NULL;
    gint state = cid_console_get_int_with_error_full(
                                "dcop amarok player status",-1,&error);
    
    musicData.opening = TRUE;
    
    if (error) 
    {
        cid_warning ("%s",error->message);
        
        musicData.opening = FALSE;
        musicData.playing = FALSE;
    
        if (run) 
        {
            cid_display_image(cid->config->cDefaultImage);
            cid_set_state_icon();
            run = FALSE;
        }
        
        cid_free_error (error);
        return FALSE;
    }
    
    gboolean bOldState = musicData.playing;
    
    if (state == 1)
        musicData.playing = FALSE;
    else if (state == 2)
        musicData.playing = TRUE;
        
    if (bOldState!=musicData.playing)
        cid_set_state_icon();
    
    
    gchar *gArtist, *gAlbum, *gTitle, *gPlayingURI, *gCoverURI;
    
    
    gArtist = cid_console_get_string ("dcop amarok player artist");
    gAlbum = cid_console_get_string ("dcop amarok player album");
    gTitle = cid_console_get_string ("dcop amarok player title");
    gPlayingURI = cid_console_get_string (
                                    "dcop amarok player encodedURL");
    gCoverURI = cid_console_get_string (
                                    "dcop amarok player coverImage");
    
    g_free (musicData.playing_cover);
    if (gCoverURI == NULL)
        musicData.playing_cover = g_strdup (cid->config->cDefaultImage);
    else 
        musicData.playing_cover = g_strdup (gCoverURI);
    
    if (musicData.playing_uri != NULL && 
        strcmp (musicData.playing_uri,gPlayingURI)==0)
    {
        g_free (gArtist);
        g_free (gAlbum);
        g_free (gTitle);
        g_free (gPlayingURI);
        g_free (gCoverURI);
        return FALSE;
    }
        
    g_free (musicData.playing_uri);
    musicData.playing_uri = g_strdup(gPlayingURI);
    g_free (musicData.playing_title);
    musicData.playing_title = g_strdup (gTitle);
    g_free (musicData.playing_album);
    musicData.playing_album = g_strdup (gAlbum);
    g_free (musicData.playing_artist);
    musicData.playing_artist = g_strdup (gArtist);
    
    
    run = TRUE;
    cid_info ("\nartist : %s\n\
album : %s\n\
title : %s\n\
song uri : %s\n\
cover uri : %s",
gArtist,gAlbum,gTitle,gPlayingURI,gCoverURI);
    g_free (gArtist);
    g_free (gAlbum);
    g_free (gTitle);
    g_free (gPlayingURI);
    g_free (gCoverURI);
    return TRUE;
}

gchar *
cid_check_amarok_cover_exists (CidMainContainer **pCid, gchar *cURI) 
{
    CidMainContainer *cid = *pCid;
    gchar *cRet = g_strdup(cURI);
    gchar **cCleanURI = g_strsplit (cRet,"@",0);
    gchar **cSplitedURI = g_strsplit (cCleanURI[1],".",0);
    if (g_strcasecmp(cSplitedURI[0],"nocover")==0) 
    {
        g_free (cRet);
        cRet = cid_db_search_cover (pCid, 
                                    musicData.playing_artist, 
                                    musicData.playing_album);
        if (cRet == NULL)
        {
            cid->runtime->iCheckIter = 0;
            g_timeout_add (1000, 
                           (GSourceFunc) _check_cover_is_present, 
                           pCid);
            cRet = g_strdup(cid->config->cDefaultImage);
        }
    }
    g_strfreev (cCleanURI);
    g_strfreev (cSplitedURI);
    return cRet;
}

gboolean 
cid_amarok_cover(CidMainContainer **pCid) 
{
    CidMainContainer *cid = *pCid;
    /* On vérifie l'état d'Amarok */
    if (get_amarock_musicData(pCid)) 
    {
        /* Si Amarok ne joue pas, on affiche l'image par défaut. */
        if (!musicData.opening) 
        {
            cid_debug ("Amarok isn't playing");
            cid_display_image (cid->config->cDefaultImage);
        } 
        else 
        {
            if (musicData.playing && musicData.playing_cover != NULL)
            {
                gchar *cCover = cid_check_amarok_cover_exists(
                                            pCid, 
                                            musicData.playing_cover);
                cid_display_image (cCover);
                g_free (cCover);
            }
            else
                cid_display_image (cid->config->cDefaultImage);
            cid_animation(cid->config->iAnimationType);
        } 
    }
        
    return cont;
}

void 
cid_amarok_pipe (CidMainContainer **pCid, gint iInter) 
{
    CidMainContainer *cid = *pCid;
    cid->runtime->iPipe = g_timeout_add_full (G_PRIORITY_HIGH, 
                                            iInter,
                                            (gpointer) cid_amarok_cover, 
                                            pCid, 
                                            NULL);
}

void 
cid_disconnect_from_amarok (CidMainContainer **pCid) 
{
    CidMainContainer *cid = *pCid;
    cont = FALSE;
    if (cid->runtime->bPipeRunning)
        g_source_remove (cid->runtime->iPipe);
    cid->runtime->bPipeRunning = FALSE;
}

void 
cid_connect_to_amarok(CidMainContainer **pCid, gint iInter) 
{
    CidMainContainer *cid = *pCid;
    cont = TRUE;
    cid->runtime->bPipeRunning = TRUE;
    cid_display_image(cid->config->cDefaultImage);
    cid_amarok_pipe (pCid, iInter);   
}

void 
_playPause_amarok (CidMainContainer **pCid) 
{
    cid_launch_command ("dcop amarok player playPause >/dev/null 2>&1");
    cid_amarok_cover (pCid);
}

void 
_next_amarok (CidMainContainer **pCid) 
{
    cid_launch_command ("dcop amarok player next >/dev/null 2>&1");
    cid_amarok_cover (pCid);
}

void 
_previous_amarok (CidMainContainer **pCid) 
{
    cid_launch_command ("dcop amarok player prev >/dev/null 2>&1");
    cid_amarok_cover (pCid);
}

void 
cid_build_amarok_menu (CidMainContainer **pCid) 
{
    CidMainContainer *cid = *pCid;
    cid->runtime->pMonitorList->p_fPlayPause = _playPause_amarok;
    cid->runtime->pMonitorList->p_fNext = _next_amarok;
    cid->runtime->pMonitorList->p_fPrevious = _previous_amarok;
    cid->runtime->pMonitorList->p_fAddToQueue = NULL;
}
