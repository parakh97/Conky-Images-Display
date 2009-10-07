/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>

#include "applet-struct.h"
#include "applet-init.h"


CID_APPLET_DEFINITION (N_("CID_APPLET_NAME"),
    2, 0, 0,
    CAIRO_DOCK_CATEGORY_ACCESSORY,
    N_("Useful description\n"
    "and manual"),
    "CID_MY_NAME")


//\___________ Here is where you initiate your applet. myConfig is already set at this point, and also myIcon, myContainer, myDock, myDesklet (and myDrawContext if you're in dock mode). The macro CD_APPLET_MY_CONF_FILE and CD_APPLET_MY_KEY_FILE can give you access to the applet's conf-file and its corresponding key-file (also available during reload). If you're in desklet mode, myDrawContext is still NULL, and myIcon's buffers has not been filled, because you may not need them then (idem when reloading).
CD_APPLET_INIT_BEGIN
    if (myDesklet)
    {
        CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
    }
    
    if (myIcon->acFileName == NULL)  // set a default icon if none is specified.
    {
        CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);
    }
    
    /// To be continued ...
    
    
    
    CD_APPLET_REGISTER_FOR_CLICK_EVENT;
    CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
CD_APPLET_INIT_END


//\___________ Here is where you stop your applet. myConfig and myData are still valid, but will be reseted to 0 at the end of the function. In the end, your applet will go back to its original state, as if it had never been activated.
CD_APPLET_STOP_BEGIN
    CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
    CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
    
    /// To be continued ...
    
    
CD_APPLET_STOP_END


//\___________ The reload occurs in 2 occasions : when the user changes the applet's config, and when the user reload the cairo-dock's config or modify the desklet's size. The macro CD_APPLET_MY_CONFIG_CHANGED can tell you this. myConfig has already been reloaded at this point if you're in the first case, myData is untouched. You also have the macro CD_APPLET_MY_CONTAINER_TYPE_CHANGED that can tell you if you switched from dock/desklet to desklet/dock mode.
CD_APPLET_RELOAD_BEGIN
    if (myDesklet)
    {
        CD_APPLET_SET_DESKLET_RENDERER ("Simple");  // set a desklet renderer.
    }
    
    if (CD_APPLET_MY_CONFIG_CHANGED)
    {
        if (myIcon->acFileName == NULL)
        {
            CD_APPLET_SET_LOCAL_IMAGE_ON_MY_ICON (MY_APPLET_ICON_FILE);  // set a default icon if none is specified.
        }
        
        /// To be continued ...
        
    }
CD_APPLET_RELOAD_END
