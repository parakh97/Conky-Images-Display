/*
   *
   *                             cid-config.c
   *                               -------
   *                         Conky Images Display
   *             --------------------------------------------
   *
*/

#include <math.h>
#include <sys/stat.h>

#include "cid-config.h"
#include "cid-messages.h"
#include "cid-utilities.h"
#include "cid-gui-factory.h"
#include "cid-callbacks.h"
#include "cid-gui-callback.h"
#include "cid-constantes.h"
#include "cid-draw.h"

static gint iNbRead = 0;
static gboolean bChangedDesktop;
//static gboolean bUnvalidKey;
static gboolean bReloaded = FALSE;
static PlayerIndice iPlayerChanged;
static SymbolColor iSymbolChanged;
static gint iOldWidth, iOldHeight;

void 
cid_read_config_after_update (CidMainContainer **pCid, const char *f) 
{
    CidMainContainer *cid = *pCid;
    cid_read_config (pCid, f);
    
    // Si on active les fonctions 'instables', on détruit puis recree la fenetre
    if ((cid->config->bChangedTestingConf != (cid->config->bUnstable && cid->config->bTesting))) 
    {
        cid->config->bChangedTestingConf = cid->config->bTesting && cid->config->bUnstable;
        gtk_widget_destroy(cid->pWindow);
        cid_create_main_window();
    }
    
    // Si on change de lecteur
    if (iPlayerChanged != cid->config->iPlayer) 
    {
        cid_disconnect_player (pCid);
    
        cid_free_musicData();
    
        if (cid->runtime->pMonitorList) 
        {
            //g_slice_free (CidControlFunctionsList,cid->runtime->pMonitorList);
            cid->runtime->pMonitorList = NULL;
        }
    
    
        cid_run_with_player(pCid);
    }
    
    // Si la couleur des controles change, on les recharge
    if (iSymbolChanged != cid->config->iSymbolColor || iPlayerChanged != cid->config->iPlayer)
        cid_load_symbols();
    
    cid_check_position(pCid);
    
    gtk_window_move (GTK_WINDOW(cid->pWindow), cid->config->iPosX, cid->config->iPosY);
    gtk_window_resize (GTK_WINDOW (cid->pWindow), cid->config->iWidth, cid->config->iHeight);
    
    // Si on change l'affichage
    if (bChangedDesktop != cid->config->bAllDesktop) 
    {
        if (!cid->config->bAllDesktop)
            gtk_window_unstick(GTK_WINDOW(cid->pWindow));
        else
            gtk_window_stick(GTK_WINDOW(cid->pWindow));
    }
    
    // Enfin, si on redimenssionne, on recharge les images
    if (cid->config->iWidth != iOldWidth || cid->config->iHeight != iOldHeight) 
    {
        cid_display_image (musicData.playing_cover);
        cid_load_symbols();
    }
    
    CID_REDRAW;
    
}

gboolean 
cid_load_key_file(CidMainContainer **pCid, GKeyFile **pKeyFile, const gchar *cFile) 
{
    CidMainContainer *cid = *pCid;
    GKeyFileFlags flags;
    GError *error = NULL;

    /* Create a new GKeyFile object and a bitwise list of flags. */
    *pKeyFile = g_key_file_new ();
    flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

    /* Load the GKeyFile or return. */
    if (!g_key_file_load_from_file (*pKeyFile, cFile, flags, &error)) 
    {
        cid_warning ("%s",error->message);
        g_error_free (error);
        return FALSE;
    }
    return TRUE;
}

void 
cid_key_file_free(CidMainContainer **pCid) 
{
    g_key_file_free ((*pCid)->pKeyFile);
}

gboolean 
cid_get_boolean_value_full (CidMainContainer **pCid, GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault) 
{
    GError *error = NULL;
    gboolean bGet = g_key_file_get_boolean (pKeyFile, cGroup, cKey, &error);
    if (error != NULL) 
    {
        (*pCid)->config->bUnvalidKey = TRUE;
        cid_warning("key '%s' in group '%s'\n=> %s",cKey,cGroup,error->message);
        g_error_free(error);
        error = NULL;
        bGet = bDefault;
    }
    cid_debug ("%s:%s=%s",cGroup,cKey,bGet?"TRUE":"FALSE");
    return bGet;
}

gchar *
cid_get_string_value_full (CidMainContainer **pCid, GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault, gchar *cDefault, gboolean bFile, gboolean bDir, gboolean bForce) 
{
    GError *error = NULL;
    gchar *cGet = g_key_file_get_string (pKeyFile, cGroup, cKey, &error);
    if (error != NULL && bDefault) 
    {
        (*pCid)->config->bUnvalidKey = TRUE;
        cid_warning("key '%s' in group '%s'\n=> %s",cKey,cGroup,error->message);
        g_error_free(error);
        error = NULL;
        g_free (cGet);
        cGet = g_strdup(cDefault);
    }
    if ((bFile || bDir) && cDefault != NULL && !g_file_test (cGet, bDir ? G_FILE_TEST_IS_DIR : G_FILE_TEST_EXISTS)) 
    {
        g_free (cGet);
        if (g_file_test (cDefault, bDir ? G_FILE_TEST_IS_DIR : G_FILE_TEST_EXISTS) || bForce)
        {
            cid_debug ("%s:%s=%s",cGroup,cKey,cDefault);
            return cDefault;
        }
        else 
        {
            cid_debug ("%s:%s=(NULL)",cGroup,cKey);
            return NULL;
        }
    }
    cid_debug ("%s:%s=%s",cGroup,cKey,cGet);
    return cGet;
}

gint 
cid_get_int_value_full (CidMainContainer **pCid, GKeyFile *pKeyFile, gchar *cGroup, gchar *cKey, gboolean bDefault, gint iDefault, gboolean bMax, gint iMax) 
{
    GError *error = NULL;
    gint iGet = g_key_file_get_integer (pKeyFile, cGroup, cKey, &error);
    if (error != NULL && bDefault) 
    {
        (*pCid)->config->bUnvalidKey = TRUE;
        cid_warning("key '%s' in group '%s'\n=> %s",cKey,cGroup,error->message);
        g_error_free(error);
        error = NULL;
        iGet = iDefault;
    }
    cid_debug ("%s:%s=%d",cGroup,cKey,(bMax && iGet > iMax)?iMax:iGet);
    if (bMax && iGet > iMax)
        return iMax;
    return iGet;
}

static gboolean 
cid_free_and_debug_error (CidMainContainer **pCid, GError **error) 
{
    if (*error != NULL) 
    {
        (*pCid)->config->bUnvalidKey = TRUE;
        cid_warning("\n=> %s",(*error)->message);
        g_error_free(*error);
        *error = NULL;
        return TRUE;
    }
    return FALSE;
}

void 
cid_read_key_file (CidMainContainer **pCid, const gchar *f) 
{   
    CidMainContainer *cid = *pCid;
    if (!cid_load_key_file(pCid, &(cid->pKeyFile), f))
        cid_exit(pCid, CID_ERROR_READING_FILE,"Key File error");

    bChangedDesktop = cid->config->bAllDesktop;
    iPlayerChanged  = cid->config->iPlayer;
    iSymbolChanged  = cid->config->iSymbolColor;
    iOldWidth       = cid->config->iWidth;
    iOldHeight      = cid->config->iHeight;
    
    gint *pSize;
    gsize iReadSize;

    GError *error = NULL;
    cid->config->bUnvalidKey = FALSE;
    
    // [System] configuration
    cid->config->iPlayer         = CID_CONFIG_GET_INTEGER ("System", "PLAYER");
    cid->config->iInter          = CID_CONFIG_GET_INTEGER_WITH_DEFAULT ("System", "INTER", DEFAULT_TIMERS) SECONDES;
    cid->config->bMonitorPlayer  = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("System", "MONITOR", TRUE);
    cid->config->bPlayerState    = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("System", "STATE", TRUE);
    cid->config->bDisplayTitle   = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("System", "TITLE", TRUE);
    cid->config->iSymbolColor    = CID_CONFIG_GET_INTEGER ("System", "SYMBOL_COLOR");
    cid->config->bDisplayControl = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("System", "CONTROLS", TRUE);
    cid->config->dPoliceSize     = g_key_file_get_double  (cid->pKeyFile, "System", "POLICE_SIZE", &error);
    cid_free_and_debug_error(pCid, &error);
    cid->config->dPoliceColor    = g_key_file_get_double_list (cid->pKeyFile, "System", "POLICE_COLOR", &cid->config->iPlainTextSize, &error);
    cid_free_and_debug_error(pCid, &error);
    cid->config->dOutlineTextColor = g_key_file_get_double_list (cid->pKeyFile, "System", "OUTLINE_COLOR", &cid->config->iOutlineTextSize, &error);
    cid_free_and_debug_error(pCid, &error);
    cid->config->t_cCoverPatternList = g_key_file_get_string_list (cid->pKeyFile, "System", "FILES_LIST", &cid->config->iNbPatterns, &error);
    cid_free_and_debug_error(pCid, &error);
    cid->runtime->pCoversList = cid_char_table_to_datatable (cid->config->t_cCoverPatternList, cid->config->iNbPatterns);

    // [Options] configuration
    cid->config->bHide           = CID_CONFIG_GET_BOOLEAN ("Options", "HIDE");
    cid->config->cDefaultImage   = CID_CONFIG_GET_FILE_PATH  ("Options", "IMAGE", 
        cid->config->bDevMode ? TESTING_DIR"/"TESTING_COVER : CID_DEFAULT_IMAGE);
    cid->config->bRunAnimation   = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Options", "ANIMATION", TRUE);
    cid->config->iAnimationType  = CID_CONFIG_GET_INTEGER ("Options", "ANIMATION_TYPE");
    cid->config->iAnimationSpeed = CID_CONFIG_GET_INTEGER ("Options", "ANIMATION_SPEED");
    cid->config->bThreaded       = CID_CONFIG_GET_BOOLEAN ("Options", "THREAD");
    cid->config->bUnstable       = cid->config->bTesting && CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Options",
                                                            "B_UNSTABLE", TRUE);
    
    // [Downloads] configuration
    cid->config->bDownload       = CID_CONFIG_GET_BOOLEAN ("Downloads", "DOWNLOAD");
    cid->config->cDLPath         = CID_CONFIG_GET_DIR_PATH_FORCE ("Downloads", "DL_PATH", cid->defaut->cDLPath);
    cid->config->iImageSize      = CID_CONFIG_GET_INTEGER ("Downloads", "D_SIZE");
    cid->config->iTimeToWait     = CID_CONFIG_GET_INTEGER_WITH_DEFAULT ("Downloads", "DELAY", DEFAULT_TIMERS);
    
    // [Behaviour] configuration
    cid->config->iPosX          = CID_CONFIG_GET_INTEGER ("Behaviour", "GAP_X");
    cid->config->iPosY          = CID_CONFIG_GET_INTEGER ("Behaviour", "GAP_Y");
    pSize               = g_key_file_get_integer_list (cid->pKeyFile, "Behaviour", "SIZE", &iReadSize, &error);
    if (cid_free_and_debug_error(pCid, &error) || iReadSize != 2)
    {
        pSize = g_realloc (pSize, 2 * sizeof(int));
        if (pSize != NULL)
        {
            pSize[0] = DEFAULT_SIZE;
            pSize[1] = DEFAULT_SIZE;
        }
        else
        {
            cid_exit (pCid, CID_ERROR_READING_FILE, "cannot allocate memory");
        }
    }
    cid->config->dRotate        = g_key_file_get_double  (cid->pKeyFile, "Behaviour", "ROTATION", &error);
    cid_free_and_debug_error(pCid, &error);
    cid->config->dColor         = g_key_file_get_double_list (cid->pKeyFile, "Behaviour", "COLOR", &cid->config->iColorSize, &error);
    cid_free_and_debug_error(pCid, &error);
    cid->config->dFlyingColor   = g_key_file_get_double_list (cid->pKeyFile, "Behaviour", "FLYING_COLOR", &cid->config->iFlyiniColorSize, &error);
    cid_free_and_debug_error(pCid, &error);
    cid->config->bKeepCorners   = CID_CONFIG_GET_BOOLEAN ("Behaviour", "KEEP_CORNERS");
    cid->config->bAllDesktop    = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Behaviour", "ALL_DESKTOP", TRUE);
    cid->config->bLockPosition  = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Behaviour", "LOCK", TRUE);
    cid->config->bMask          = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Behaviour", "MASK", TRUE);
    cid->config->bShowAbove     = CID_CONFIG_GET_BOOLEAN_WITH_DEFAULT ("Behaviour", "SWITCH_ABOVE", TRUE);
    
    // MPD configurations
    cid->mpd_dir   = CID_CONFIG_GET_DIR_PATH ("MPD", "MPD_DIR", g_strdup_printf ("%s/Music",g_getenv ("HOME")));
    cid->mpd_host  = CID_CONFIG_GET_STRING ("MPD", "MPD_HOST");
    if (cid->mpd_host != NULL && strcmp (cid->mpd_host, "") == 0)
    {
        g_free (cid->mpd_host);
        cid->mpd_host = g_strdup ("localhost");
    }
    gchar *cEncrypted = NULL;
    cEncrypted     = CID_CONFIG_GET_STRING ("MPD", "MPD_PASS");
    //cid_decrypt_string (cEncrypted, &cid->mpd_pass);
    cid->mpd_pass = g_strdup (cEncrypted);
    g_free (cEncrypted);
    cid->mpd_port  = CID_CONFIG_GET_INTEGER_WITH_DEFAULT ("MPD", "MPD_PORT", 6600);
    
    cid->config->iWidth = pSize[0] <= MAX_SIZE ? pSize[0] : MAX_SIZE;
    cid->config->iHeight = pSize[1] <= MAX_SIZE ? pSize[1] : MAX_SIZE;
    
    if (!cid->config->bUnvalidKey) 
    {
        cid->config->dRed            = cid->config->dColor[0];
        cid->config->dGreen          = cid->config->dColor[1];
        cid->config->dBlue           = cid->config->dColor[2];
        cid->config->dAlpha          = cid->config->dColor[3];
        cid->runtime->dFocusVariation = cid->config->dFlyingColor[3]>cid->config->dAlpha ? +1 : -1;
        cid->config->iExtraSize      = (cid->config->iHeight + cid->config->iWidth)/20;
        cid->config->iPrevNextSize   = cid->config->iExtraSize * 2;
        cid->config->iPlayPauseSize  = cid->config->iExtraSize * 3;
    }
    
    cid_key_file_free(pCid);

    if (cid->config->bUnvalidKey && !bReloaded)
    {
        cid_save_data (pCid);
        cid_read_key_file (pCid, f);
        bReloaded = TRUE;
    }
}

int 
cid_read_config (CidMainContainer **pCid, const char *f) 
{
    CidMainContainer *cid = *pCid;
    
    cid_info ("Reading file : %s",f);
    
    if (!cid->config->bDevMode) 
    {
        if (!cid_file_check (f))
            cid_exit (pCid, CID_ERROR_READING_FILE, "Unable to find configuration file");
    }
    
    cid_read_key_file (pCid, f);
    
    if (!cid->config->bDevMode) 
    {
        if (!cid_file_check_config_version (pCid, f))
            cid_exit (pCid, CID_ERROR_READING_FILE, "Unable to replace configuration file");
    }
    
    iNbRead++;

    return 0;
}

void 
cid_get_data (CidMainContainer **pCid) 
{
    CidMainContainer *cid = *pCid;
    /* On récupère la position de cid */
    gtk_window_get_position(GTK_WINDOW (cid->pWindow), &cid->config->iPosX, &cid->config->iPosY);
    
    /* On récupère la taille de cid */
    gtk_window_get_size(GTK_WINDOW (cid->pWindow), &cid->config->iWidth, &cid->config->iHeight);
}

void 
cid_save_data (CidMainContainer **pCid) 
{
    CidMainContainer *cid = *pCid;
    if (!cid_load_key_file(pCid, &(cid->pKeyFile), cid->config->cConfFile))
        cid_exit(pCid, CID_ERROR_READING_FILE,"Key File error");
    
    if (cid->pWindow!=NULL)
        cid_get_data(pCid);
    
    // [System] configuration
    g_key_file_set_integer (cid->pKeyFile, "System", "PLAYER", cid->config->iPlayer);
    g_key_file_set_integer (cid->pKeyFile, "System", "INTER", cid->config->iInter/1000);
    g_key_file_set_integer (cid->pKeyFile, "System", "SYMBOL_COLOR", cid->config->iSymbolColor);
    g_key_file_set_boolean (cid->pKeyFile, "System", "MONITOR", cid->config->bMonitorPlayer);
    g_key_file_set_boolean (cid->pKeyFile, "System", "STATE", cid->config->bPlayerState);
    g_key_file_set_boolean (cid->pKeyFile, "System", "TITLE", cid->config->bDisplayTitle);
    g_key_file_set_boolean (cid->pKeyFile, "System", "CONTROLS", cid->config->bDisplayControl);
    g_key_file_set_double (cid->pKeyFile, "System", "POLICE_SIZE",(cid->config->dPoliceSize));
    g_key_file_set_double_list (cid->pKeyFile, "System", "POLICE_COLOR", (cid->config->dPoliceColor), cid->config->iPlainTextSize);
    g_key_file_set_double_list (cid->pKeyFile, "System", "OUTLINE_COLOR", (cid->config->dOutlineTextColor), cid->config->iOutlineTextSize);

    // [Options] configuration
    g_key_file_set_boolean (cid->pKeyFile, "Options", "ANIMATION", cid->config->bRunAnimation);
    g_key_file_set_boolean (cid->pKeyFile, "Options", "HIDE", cid->config->bHide);
    if (strcmp(cid->config->cDefaultImage,TESTING_DIR"/"TESTING_COVER)!=0 && strcmp(cid->config->cDefaultImage,CID_DEFAULT_IMAGE)!=0)
        g_key_file_set_string  (cid->pKeyFile, "Options", "IMAGE", cid->config->cDefaultImage);
    else
        g_key_file_set_string  (cid->pKeyFile, "Options", "IMAGE", "");
    g_key_file_set_boolean (cid->pKeyFile, "Options", "THREAD", cid->config->bThreaded);
    g_key_file_set_integer (cid->pKeyFile, "Options", "ANIMATION_TYPE", cid->config->iAnimationType);
    g_key_file_set_integer (cid->pKeyFile, "Options", "ANIMATION_SPEED", cid->config->iAnimationSpeed);
    g_key_file_set_boolean (cid->pKeyFile, "Options", "B_UNSTABLE", cid->config->bUnstable);
    
    // [Downloads] configuration
    g_key_file_set_boolean (cid->pKeyFile, "Downloads", "DOWNLOAD", cid->config->bDownload);
    g_key_file_set_integer (cid->pKeyFile, "Downloads", "DELAY", cid->config->iTimeToWait);
    g_key_file_set_integer (cid->pKeyFile, "Downloads", "D_SIZE", cid->config->iImageSize);
    if (strcmp(cid->config->cDLPath,cid->defaut->cDLPath) != 0)
        g_key_file_set_string (cid->pKeyFile, "Downloads", "DL_PATH", cid->config->cDLPath);
    else
        g_key_file_set_string (cid->pKeyFile, "Downloads", "DL_PATH", "");
    
    // [Behaviour] configuration
    gint pSize[2] = {cid->config->iWidth,cid->config->iHeight};
    gsize iReadSize = sizeof (pSize) / sizeof (*pSize);
    g_key_file_set_integer_list (cid->pKeyFile, "Behaviour", "SIZE", pSize, iReadSize);
    g_key_file_set_integer (cid->pKeyFile, "Behaviour", "GAP_X",cid->config->iPosX);
    g_key_file_set_integer (cid->pKeyFile, "Behaviour", "GAP_Y",cid->config->iPosY);
    
    g_key_file_set_double (cid->pKeyFile, "Behaviour", "ROTATION",(cid->config->dRotate));
    g_key_file_set_double_list (cid->pKeyFile, "Behaviour", "COLOR", (cid->config->dColor), cid->config->iColorSize);
    g_key_file_set_double_list (cid->pKeyFile, "Behaviour", "FLYING_COLOR", (cid->config->dFlyingColor), cid->config->iFlyiniColorSize);
    g_key_file_set_boolean (cid->pKeyFile, "Behaviour", "KEEP_CORNERS", cid->config->bKeepCorners);
    g_key_file_set_boolean (cid->pKeyFile, "Behaviour", "ALL_DESKTOP", cid->config->bAllDesktop);
    g_key_file_set_boolean (cid->pKeyFile, "Behaviour", "LOCK", cid->config->bLockPosition);
    g_key_file_set_boolean (cid->pKeyFile, "Behaviour", "MASK", cid->config->bMask);
    g_key_file_set_boolean (cid->pKeyFile, "Behaviour", "SWITCH_ABOVE", cid->config->bShowAbove);

    // [MPD] configuration
    gchar *cDefaultDir = g_strdup_printf ("%s/Music", g_getenv ("HOME"));
    if (cid->mpd_dir != NULL && strcmp (cid->mpd_dir, cDefaultDir) != 0)
        g_key_file_set_string (cid->pKeyFile, "MPD", "MPD_DIR", cid->mpd_dir);
    else
        g_key_file_set_string (cid->pKeyFile, "MPD", "MPD_DIR", "");
    g_free (cDefaultDir);
    g_key_file_set_string (cid->pKeyFile, "MPD", "MPD_HOST", cid->mpd_host);
    gchar *cEncrypted = NULL;
    cid_encrypt_string (cid->mpd_pass, &cEncrypted);
    g_key_file_set_string (cid->pKeyFile, "MPD", "MPD_PASS", cEncrypted);
    g_free (cEncrypted);
    g_key_file_set_integer (cid->pKeyFile, "MPD", "MPD_PORT", cid->mpd_port);
    
    cid_write_keys_to_file (cid->pKeyFile, cid->config->cConfFile);
}

void 
cid_write_keys_to_file (GKeyFile *pKeyFile, const gchar *cConfFilePath) 
{
    cid_debug ("%s (%s)", __func__, cConfFilePath);
    GError *erreur = NULL;

    gchar *cDirectory = g_path_get_dirname (cConfFilePath);
    if (! g_file_test (cDirectory, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)) 
    {
        cid_info ("Creating path '%s'", cDirectory);
        g_mkdir_with_parents (cDirectory, 7*8*8+7*8+5);
    }
    g_free (cDirectory);


    gsize length;
    gchar *cNewConfFileData = g_key_file_to_data (pKeyFile, &length, &erreur);
    if (erreur != NULL) 
    {
        cid_warning ("Error while fetching data : %s", erreur->message);
        g_error_free (erreur);
        return ;
    }

    g_file_set_contents (cConfFilePath, cNewConfFileData, length, &erreur);
    g_free (cNewConfFileData);
    if (erreur != NULL) 
    {
        cid_warning ("Error while writing data : %s", erreur->message);
        g_error_free (erreur);
        return ;
    }
}
