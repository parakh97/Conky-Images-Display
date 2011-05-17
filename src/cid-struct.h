/*
   *
   *                    cid-struct.h
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
*/
#ifndef __CID_STRUCT__
#define  __CID_STRUCT__

#include <gtk/gtk.h>
#include "cid-datatables.h"
#include "config.h"

G_BEGIN_DECLS
///\______ Structures de donnees
typedef struct _CidMainContainer CidMainContainer;
typedef struct _CidConfig CidConfig;
typedef struct _CidRuntime CidRuntime;
typedef struct _CidDefault CidDefault;
typedef struct _CidLabelDescription CidLabelDescription;
typedef struct _CidControlFunctionsList CidControlFunctionsList;
typedef struct _CidColorContainer CidColorContainer;

typedef struct _CidModule CidModule;
typedef struct _CidModuleInterface CidModuleInterface;
typedef struct _CidModuleInstance CidModuleInstance;
typedef struct _CidVisitCard CidVisitCard;
typedef struct _CidInternalModule CidInternalModule;

///\______ Encapsulations
typedef void (* CidReadConfigFunc) (CidMainContainer **pCid, gchar *cConfFile);
typedef void (* CidControlFunction) (CidMainContainer **pCid);
typedef void (* CidManagePlaylistFunction) (gchar *cSong);
typedef void (* CidConnectPlayer) (gint iDelay);
typedef void (* CidDisconnectPlayer) (void);

/// Structure de données utilisée pour stocker les informations fournies par Rhythmbox et Amarok
struct data {
    /// URI du fichier lu
    gchar *playing_uri;
    /// nom de l'artiste en cours de lecture
    gchar *playing_artist;
    /// nom de l'album en cours de lecture
    gchar *playing_album;
    /// titre de la musique en cours de lecture
    gchar *playing_title;
    /// URI de la pochette
    gchar *playing_cover;

    /// numéro de la piste jouée
    guint playing_track;
    /// durée totale de la piste (en secondes)
    guint playing_duration;
    /// id de la fonction de recherche de pochettes
    guint iSidCheckCover;

    /// A-t-on trouvé une pochette ?
    gboolean cover_exist;
    /// en cours de lecture si TRUE
    gboolean playing;
    gboolean opening;
} musicData;

/// Liste des lecteurs supportés
typedef enum {
    PLAYER_NONE,
    PLAYER_RHYTHMBOX,
    PLAYER_AMAROK_1,
    PLAYER_EXAILE,
    PLAYER_AMAROK_2,
    PLAYER_MPD 
} PlayerIndice;

/// Liste des animations
typedef enum {
    CID_ROTATE,
    CID_FADE_IN_OUT,
    CID_DIAPOSITIVE,
    CID_FOCUS_IN,
    CID_FOCUS_OUT
} AnimationType;

/// Codes de retours spécifiques à CID
typedef enum {
    CID_EXIT_SUCCESS=0,
    CID_ERROR_READING_FILE,
    CID_ERROR_READING_ARGS,
    CID_GTK_ERROR,
    CID_OPEN_GL_ERROR,
    CID_PLAYER_ERROR,
    CID_EXIT_ERROR
} codesRetours;

/// Tailles possible pour le téléchargement des pochettes
typedef enum {
    MEDIUM_IMAGE,
    LARGE_IMAGE
} ImageSizes;

/// Differents etats du lecteur
typedef enum {
    CID_PLAY,
    CID_PAUSE,
    CID_NEXT,
    CID_PREV
} StateSymbol;

/// Couleurs disponibles pour les 'styles'
typedef enum {
    CID_WHITE,
    CID_YELLOW,
    CID_RED
} SymbolColor;

/// Structure de couleurs
struct _CidColorContainer {
    /// red
    gdouble dRed;
    /// green
    gdouble dGreen;
    /// blue
    gdouble dBlue;
    /// alpha;
    gdouble dAlpha;
};

/// Fonctions de controle des lecteurs
struct _CidControlFunctionsList {
    /// fonction 'play/pause'
    CidControlFunction p_fPlayPause;
    /// fonction 'next'
    CidControlFunction p_fNext;
    /// fonction 'previous'
    CidControlFunction p_fPrevious;
    /// fonction d'ajout a la playlist
    CidManagePlaylistFunction p_fAddToQueue;
};

/// Cid main container
struct _CidMainContainer {
    /// fenêtre principale
    GtkWidget *pWindow;
    /// panneau de configuration
    GtkWidget *pConfigPanel;
    
    ///\________ Toutes nos images
    
    /// pochette
    cairo_surface_t *p_cSurface;
    /// pochette precedente
    cairo_surface_t *p_cPreviousSurface;
    /// croix pour le deplacement
    cairo_surface_t *p_cCross;
    /// icone de connexion
    cairo_surface_t *p_cConnect;
    /// icone de deconnexion
    cairo_surface_t *p_cDisconnect;
    /// play
    cairo_surface_t *p_cPlay;
    /// pause
    cairo_surface_t *p_cPause;
    /// play big
    cairo_surface_t *p_cPlay_big;
    /// pause big
    cairo_surface_t *p_cPause_big;
    /// next
    cairo_surface_t *p_cNext;
    /// prev
    cairo_surface_t *p_cPrev;

    /// cairo context
    cairo_t *p_cContext;
    
    /// configuration
    CidConfig *config;
    
    /// runtime
    CidRuntime *runtime;
    
    /// valeurs par defaut
    CidDefault *defaut;
    
    /// keyFile
    GKeyFile *pKeyFile;
    
    /// handler connexion
    CidConnectPlayer p_fConnectHandler;
    /// handler deconnexion
    CidDisconnectPlayer p_fDisconnectHandler;
    
    ///\__ Dimensions de l'ecran
    
    /// largeur de l'écran
    int XScreenWidth;
    /// longueur de l'écran
    int XScreenHeight;
    
    ///\__ MPD specific config
    
    /// répertoire de lecture
    gchar *mpd_dir;
    /// adresse du serveur MPD
    gchar *mpd_host;
    /// mot de passe MPD
    gchar *mpd_pass;
    /// port du serveur
    gint mpd_port;
};

/// Structure contenant les paramètres d'exécution de CID
struct _CidRuntime {
    /// currently drawing
    gint iCurrentlyDrawing;
    /// temps ecoule avant le telechargement
    gint iCheckIter;
    /// position en x du curseur
    gint iCursorX;
    /// position en y du curseur
    gint iCursorY;
    /// indice de l'image à utiliser
    gint iImageIndex;
    
    /// avancement de l'animation
    gdouble dAnimationProgress;
    
    /// pipe
    guint iPipe;
    
    /// fonctions de monitoring
    CidControlFunctionsList *pMonitorList;
    
    /// current animation angle
    gdouble dAngle;
    /// variation focus
    gdouble dFocusVariation;
    /// alpha pour le fade in
    gdouble dFadeInOutAlpha;
    /// CID lancé ?
    gboolean bRunning;
    /// animation en cours ?
    gboolean bAnimation;
    /// animation de focus en cours ?
    gboolean bFocusAnimation;
    /// lecteur en lecture ?
    gboolean bCurrentlyPlaying;
    /// survol de cid ?
    gboolean bCurrentlyFlying;
    /// La fenetre de conf est ouverte ?
    gboolean bBlockedWidowActive;
    /// Est-ce qu'un panneau de configuration est deja present ?
    gboolean bConfFilePanel;
    /// On a lance un pipe ?
    gboolean bPipeRunning;
    /// On est connecte au lecteur ?
    gboolean bConnected;
    
    /// liste des patterns de pochettes à rechercher
    CidDataTable *pCoversList;
    /// liste des images trouvées 
    CidDataTable *pImagesList;
    
    /// Répertoire dans lequel on recherche des pochettes
    GDir *pLookupDirectory;
};

/// Structure contenant les paramètres de configuration de CID
struct _CidConfig {
    /// largeur de cid
    gint iWidth;
    /// hauteur de cid
    gint iHeight;
    /// position en x
    gint iPosX;
    /// position en y
    gint iPosY;
    /// interval par défaut
    gint iInter;
    /// temps avant de telecharger les pochettes manquantes
    gint iTimeToWait;
    /// taille des images "extras"
    gint iExtraSize;
    /// taille des images prev/next
    gint iPrevNextSize;
    /// taille des images play/pause big
    gint iPlayPauseSize;
    /// vitesse de l'animation
    gint iAnimationSpeed;
    
    /// lecteur a monitorer
    PlayerIndice iPlayer;
    // type d'animation
    AnimationType iAnimationType;
    /// taille des covers à télécharger
    ImageSizes iImageSize;
    /// couleur des symboles
    SymbolColor iSymbolColor;
    
    /// type de fenêtre
    GdkWindowTypeHint iHint;
    
    /// couleur de la fenêtre
    gdouble *dColor;
    /// couleur au survol
    gdouble *dFlyingColor;
    /// couleur de police
    gdouble *dPoliceColor;
    /// couleur de contour de police
    gdouble *dOutlineTextColor;
    /// angle de cid
    gdouble dRotate;
    /// opacité de la fenêtre
    gdouble dAlpha;
    /// rouge
    gdouble dRed;
    /// vert
    gdouble dGreen;
    /// bleu
    gdouble dBlue;
    /// taille de la police
    gdouble dPoliceSize;
    
    /// image par défaut
    gchar *cDefaultImage;
    /// fichier de conf
    gchar *cConfFile;
    /// verbosité du programme
    gchar *cVerbosity;
    /// Download path
    gchar *cDLPath;
    
    /// caché ?
    gboolean bHide;
    /// ne pas couper les angles ?
    gboolean bKeepCorners;
    /// en mode testing ?
    gboolean bTesting;
    /// déplacer en cliquant ?
    gboolean bLockPosition;
    /// afficher sur tous les bureaux ?
    gboolean bAllDesktop;
    /// télécharger les pochettes manquantes ?
    gboolean bDownload;
    /// on autorise l'animation ?
    gboolean bRunAnimation;
    /// on autorise le monitoring du player ?
    gboolean bMonitorPlayer;
    /// animation threadee ?
    gboolean bThreaded;
    /// mode developpeur ?
    gboolean bDevMode;
    /// options instables ?
    gboolean bUnstable;
    /// afficher le statut
    gboolean bPlayerState;
    /// on affiche un masque au survol ?
    gboolean bMask;
    /// afficher le titre ?
    gboolean bDisplayTitle;
    /// CID en mode de secours ?
    gboolean bSafeMode;
    /// On a active/desactive les options instables ?
    gboolean bChangedTestingConf;
    /// Afficher les controles ?
    gboolean bDisplayControl;
    /// Passe cid au premier plan lors d'un survol ?
    gboolean bShowAbove;
    /// On ne fait que configurer cid ?
    gboolean bConfigPanel;
    /// La configuration est invalide ?
    gboolean bUnvalidKey;

    
    /// taille de la couleur
    gsize iColorSize;
    /// taille de la couleur de survol
    gsize iFlyiniColorSize;
    /// taille de la couleur de police
    gsize iPlainTextSize;
    /// taille de la couleur de contour de police
    gsize iOutlineTextSize;
    /// nombre de motifs de pochettes
    gsize iNbPatterns;
    
    /// liste de motif de pochettes à rechercher
    gchar **t_cCoverPatternList;
};

/// Valeurs de configuration par défaut
struct _CidDefault {
    /// Download path
    gchar *cDLPath;
    
};

struct _CidLabelDescription {
    // Taille de la police (et hauteur du texte en pixels).
    gint iSize;
    // Police de caracteres.
    gchar *cFont;
    // Epaisseur des traits.
    PangoWeight iWeight;
    // Style du trace (italique ou droit).
    PangoStyle iStyle;
    // Couleur de debut du dégradé.
    gdouble fColorStart[3];
    // Couleur de fin du dégradé.
    gdouble fColorStop[3];
    // TRUE ssi le dégradé est du haut vers le bas.
    gboolean bVerticalPattern;
    // Couleur du fond.
    gdouble fBackgroundColor[4];
};

G_END_DECLS
#endif
