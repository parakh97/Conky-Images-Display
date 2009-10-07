
#ifndef __CID_APPLET_STRUCT__
#define  __CID_APPLET_STRUCT__

#include <cid.h>

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
    gboolean bSomeBooleanValue;
    gint iSomeIntegerValue;
    gchar *cSomeStringValue;
} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
    int no_data_yet;
} ;


#endif
