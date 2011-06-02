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
   *                         cid-gui-factory.h
   *                                -------
   *                          Conky Images Display
   *                    --------------------------------
   *
*/
#ifndef __CID_CONF_PANEL_FACTORY__
#define __CID_CONF_PANEL_FACTORY__

#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>
#include "cid-struct.h"

G_BEGIN_DECLS

/// Types of widgets that Cid can automatically build.
typedef enum {
	/// boolean in a button to tick.
	CID_WIDGET_CHECK_BUTTON='b',
	/// boolean in a button to tick, that will control the sensitivity of the next widget.
	CID_WIDGET_CHECK_CONTROL_BUTTON='B',
	/// integer in a spin button.
	CID_WIDGET_SPIN_INTEGER='i',
	/// integer in an horizontal scale.
	CID_WIDGET_HSCALE_INTEGER='I',
	/// pair of integers for dimansion WidthxHeight
	CID_WIDGET_SIZE_INTEGER='j',
	/// double in a spin button.
	CID_WIDGET_SPIN_DOUBLE='f',
	/// 3 doubles with a color selector (RGB).
	CID_WIDGET_COLOR_SELECTOR_RGB='c',
	/// 4 doubles with a color selector (RGBA).
	CID_WIDGET_COLOR_SELECTOR_RGBA='C',
	/// double in an horizontal scale.
	CID_WIDGET_HSCALE_DOUBLE='e',
	
	/// list of views.
	CID_WIDGET_VIEW_LIST='n',
	/// list of themes in a combo, with preview and readme.
	CID_WIDGET_THEME_LIST='h',
	/// same but with a combo-entry to let the user enter any text.
	CID_WIDGET_THEME_LIST_ENTRY='H',
	/// list of user dock themes, with a check button to select several themes.
	CID_WIDGET_USER_THEME_SELECTOR='x',
	/// list of dock themes, sortable by name, rating, and sobriety.
	CID_WIDGET_THEME_SELECTOR='R',
	/// list of available animations.
	CID_WIDGET_ANIMATION_LIST='a',
	/// list of available desklet decorations.
	CID_WIDGET_DESKLET_DECORATION_LIST='O',
	/// same but with the 'default' choice too.
	CID_WIDGET_DESKLET_DECORATION_LIST_WITH_DEFAULT='o',
	/// list of gauges themes.
	CID_WIDGET_GAUGE_LIST='g',
	/// list of existing docks.
	CID_WIDGET_DOCK_LIST='d',
	/// list of installed icon themes.
	CID_WIDGET_ICON_THEME_LIST='w',
	/// list of available modules.
	CID_WIDGET_MODULE_LIST='N',
	/// a button to jump to another module inside the config panel.
	CID_WIDGET_JUMP_TO_MODULE='m',
	/// same but only if the module exists.
	CID_WIDGET_JUMP_TO_MODULE_IF_EXISTS='M',
	
	/// a text entry.
	CID_WIDGET_STRING_ENTRY='s',
	/// a text entry with a file selector.
	CID_WIDGET_FILE_SELECTOR='S',
	/// a text entry with a folder selector.
	CID_WIDGET_FOLDER_SELECTOR='D',
	/// a text entry with a file selector and a 'play' button, for sound files.
	CID_WIDGET_SOUND_SELECTOR='u',
	/// a text entry with a shortkey selector.
	CID_WIDGET_SHORTKEY_SELECTOR='k',
	/// a text entry with a class selector.
	CID_WIDGET_CLASS_SELECTOR='K',
	/// a text entry, where text is hidden and the result is encrypted in the .conf file.
	CID_WIDGET_PASSWORD_ENTRY='p',
	/// a font selector button.
	CID_WIDGET_FONT_SELECTOR='P',
	
	/// a text list.
	CID_WIDGET_LIST='L',
	/// a combo-entry, that is to say a list where one can add a custom choice.
	CID_WIDGET_LIST_WITH_ENTRY='E',
	/// a combo where the number of the line is used for the choice.
	CID_WIDGET_NUMBERED_LIST='l',
	/// a combo where the number of the line is used for the choice, and for controlling the sensitivity of the widgets below.
	CID_WIDGET_NUMBERED_CONTROL_LIST='y',
	/// a combo where the number of the line is used for the choice, and for controlling the sensitivity of the widgets below; controlled widgets are indicated in the list : {entry;index first widget;nb widgets}.
	CID_WIDGET_NUMBERED_CONTROL_LIST_SELECTIVE='Y',
	/// a tree view, where lines are numbered and can be moved up and down.
	CID_WIDGET_TREE_VIEW_SORT='T',
	/// a tree view, where lines can be added, removed, and moved up and down.
	CID_WIDGET_TREE_VIEW_SORT_AND_MODIFY='U',
	/// a tree view, where lines are numbered and can be selected or not.
	CID_WIDGET_TREE_VIEW_MULTI_CHOICE='V',
	
	/// an empty GtkContainer, to use by applets that want to build custom widgets.
	CID_WIDGET_EMPTY_WIDGET='_',
	/// a simple text label.
	CID_WIDGET_TEXT_LABEL='>',
	/// a simple text label.
	CID_WIDGET_LINK='W',
	/// a label containing the handbook of the applet.
	CID_WIDGET_HANDBOOK='A',
	/// an horizontal separator.
	CID_WIDGET_SEPARATOR='v',
	/// a frame. The previous frame will be closed.
	CID_WIDGET_FRAME='F',
	/// a frame inside an expander. The previous frame will be closed.
	CID_WIDGET_EXPANDER='X',
    
    /// an option only available in testing mode.
	CID_WIDGET_TESTING='t',
	CID_NB_GUI_WIDGETS
} CidGUIWidgetType;

typedef enum {
    CID_MODEL_NAME = 0,
    CID_MODEL_RESULT,
    CID_MODEL_DESCRIPTION_FILE,
    CID_MODEL_ACTIVE,
    CID_MODEL_ORDER,
    CID_MODEL_IMAGE,
    CID_MODEL_ICON,
    CID_MODEL_NB_COLUMNS
} _CidModelColumns;

void _cid_set_original_value (GtkButton *button, gpointer *data);

void cid_config_panel_destroyed (void);

gboolean cid_edit_conf_file_with_panel (GtkWindow *pWindow, gchar *cConfFilePath, const gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CidReadConfigFunc pConfigFunc, gchar *cGettextDomain);

gboolean cid_edit_conf_file_core (GtkWindow *pWindow, gchar *cConfFilePath, const gchar *cTitle, int iWindowWidth, int iWindowHeight, gchar iIdentifier, gchar *cPresentedGroup, CidReadConfigFunc pConfigFunc, gchar *cGettextDomain);

GtkWidget *cid_generate_ihm_from_keyfile (GKeyFile *pKeyFile, const gchar *cTitle, GtkWindow *pParentWindow, GSList **pWidgetList, gboolean bApplyButtonPresent, gchar iIdentifier, gchar *cPresentedGroup, gchar *cGettextDomain, GPtrArray *pDataGarbage);

G_END_DECLS
#endif
