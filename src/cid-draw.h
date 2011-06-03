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
   *                              cid-draw.h
   *                                -------
   *                          Conky Images Display
   *                    --------------------------------
   *
*/
#ifndef __CID_DRAW__
#define  __CID_DRAW__

#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>
#include <cairo.h>
//#include <librsvg/rsvg.h>
//#include <librsvg/rsvg-cairo.h>

G_BEGIN_DECLS

static gchar *STATE_SYMBOL[] = {"play","pause","next","prev"};
static gchar *STATE_COLOR[]  = {"white","yellow","red"};

/// Liste des lecteurs supportés
typedef enum {
    SHAPE_NONE,
    SHAPE_PLAY,
    SHAPE_PAUSE,
    SHAPE_NEXT,
    SHAPE_PREV
} CidShapes;

/**
 * Permet de redessiner la fenêtre
 */
#define CID_REDRAW gtk_widget_queue_draw(cid->pWindow)

/**
 * affiche l'image dont le chemin
 * est passé en argument. 
 * @param URI de l'image à afficher
 */
void cid_display_image(gchar *image);

/**
 * crée une fenêtre, la déplace à la position
 * voulue, et la dimensionne 
 */
void cid_create_main_window();

/**
 * Crée un buffer de pixels à partir d'un
 * GtkWidget image 
 */
GdkPixbuf *cid_get_pixbuf (GtkWidget **imageWidget);

/**
 * Crée un GtkWidget image à partir de
 * l'URI de l'image
 */
GtkWidget *cid_get_image_widget(gchar **imageURI);

/**
 * determine si un gestionnaire de composite est présent 
 * @param fenêtre à évaluer
 * @param non utilisé
 * @param pointeur de données (non utilisé)
 */
void cid_set_colormap (GtkWidget *widget, 
                       GdkScreen *old_screen, 
                       gpointer userdata);

/**
 * renvoie une image à partir d'un buffer de pixels 
 * @param buffer de pixels
 * @return image à partir du buffer
 */
cairo_surface_t *cid_get_image_from_pixbuf (GdkPixbuf **pixbuf);

/** 
 * retourne une image aux dimensions voulues à partir de son URI.
 * @param cImagePath URI de l'image à charger.
 * @param iWidth Largeur.
 * @param iHeight Hauteur.
 * @return image.
 */
cairo_surface_t *cid_get_cairo_image (gchar *cImagePath, 
                                      gdouble iWidth, 
                                      gdouble iHeight);

/**
 * Fonction appelée pour dessiner la fenêtre
 * @param La fenêtre à dessiner
 * @param ?
 * @param non utilisé
 */
void cid_draw_window (GtkWidget *widget, 
                      GdkEventExpose *event, 
                      gpointer *userdata);

/**
 * On dessine l'etat du lecteur
 */
void cid_set_state_icon (void);

/**
 * Permet de charger tous les symboles de control du lecteur
 */
void cid_load_symbols (void);

G_END_DECLS
#endif
