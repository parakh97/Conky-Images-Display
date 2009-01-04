/*
   *
   *                             cid-animation.c
   *                                -------
   *                          Conky Images Display
   *                       03/01/2009 - Benjamin SANS
   *             --------------------------------------------
   *
*/

#include "cid.h"

/* On change la couleur de fond de la fenêtre au passage de la souris */
gboolean cid_focus_in(void *ptr) {
	cid->iCurrentlyDrawing = 1;
    
	cid->dAlpha+=.05;
	
	if (cid->bThreaded)
		gdk_threads_enter();
	gtk_widget_queue_draw(cid->cWindow);
	if (cid->bThreaded)
		gdk_threads_leave();
	
	cid->iCurrentlyDrawing = 0;
	return cid->dAlpha < cid->dFlyingColor[3];
}

/* On restaure la couleur de fond d'origine lorsqu'on quitte la fenêtre */
gboolean cid_focus_out(void *ptr) {
	cid->iCurrentlyDrawing = 1;
    
	cid->dAlpha-=.05;

	if (cid->dAlpha <= cid->dColor[3]) {
		cid->dRed = cid->dColor[0];
		cid->dGreen = cid->dColor[1];
		cid->dBlue = cid->dColor[2];
	}
	
	if (cid->bThreaded)
		gdk_threads_enter();
	gtk_widget_queue_draw(cid->cWindow);
	if (cid->bThreaded)
		gdk_threads_leave();
	
	cid->iCurrentlyDrawing = 0;
	return cid->dAlpha > cid->dColor[3];
}

/* On appelle la fonction adéquate selon qu'on "expose" ou non la fenêtre */
void cid_focus (GtkWidget *pWidget, GdkEventExpose *event, gpointer *userdata) {

	gboolean bFocusIn = GPOINTER_TO_INT (&userdata[0]);
	
	cid_info ("CID is currently focused %s.\n", bFocusIn ? "in" : "out");
	
	if (bFocusIn) {
		// On change la couleur du fond par celle choisie
		cid->dRed = cid->dFlyingColor[0];
		cid->dGreen = cid->dFlyingColor[1];
		cid->dBlue = cid->dFlyingColor[2];
		// Coloration "smoothie"
		cid_animation(CID_FOCUS_IN);
	} else 
		cid_animation(CID_FOCUS_OUT);

}

/* callback de rotation threadé */
gboolean cid_rotate_on_changing_song (void *ptr) {
	cid->iCurrentlyDrawing = 1;
    cid->bAnimation = TRUE;
    
    if (cid->dAngle < 360) {
        cid->dAngle+=.5;
        if (cid->bThreaded)
        	gdk_threads_enter();
        gtk_widget_queue_draw(cid->cWindow);
        if (cid->bThreaded)
        	gdk_threads_leave();
    } else {
    	cid->bAnimation = FALSE;
    	cid->dAngle = 0;
	}

    cid->iCurrentlyDrawing = 0;
    
    return cid->bAnimation;
}

/* on lance un thread pour la rotation */
gboolean cid_threaded_animation (gpointer *data) {
	AnimationType cAnim = GPOINTER_TO_INT(&data[0]);
	static gboolean first_execution = TRUE;

    //use a safe function to get the value of currently_drawing so
    //we don't run into the usual multithreading issues
    gint drawing_status = g_atomic_int_get(&cid->iCurrentlyDrawing);

    //if we are not currently drawing anything, launch a thread to 
    //update our pixmap
    if(drawing_status == 0){
        static pthread_t thread_info;
        int  iret;
        if(first_execution != TRUE){
            pthread_join(thread_info, NULL);
        }
        switch(cAnim) {
        	case CID_ROTATE:
        		iret = pthread_create( &thread_info, NULL, cid_rotate_on_changing_song, NULL);
        		break;
        	case CID_FOCUS_IN:
        		iret = pthread_create( &thread_info, NULL, cid_focus_in, NULL);
        		break;
        	case CID_FOCUS_OUT:
        		iret = pthread_create( &thread_info, NULL, cid_focus_out, NULL);
        		break;
		}
    }
    
    first_execution = FALSE;

    return cid->bAnimation;
}

/* G_PRIORITY_HIGH */
/* On anime l'image par une rotation */
void cid_animation (AnimationType iAnim) {
	if (cid->bRunAnimation) {
		switch(iAnim) {
        	case CID_ROTATE:
        		if (!cid->bAnimation)
        			if (cid->bThreaded)
						g_timeout_add_full (-50, 5, (GSourceFunc) cid_threaded_animation, GINT_TO_POINTER(CID_ROTATE), NULL);
					else 
        				g_timeout_add_full (-50, 5, (GSourceFunc) cid_rotate_on_changing_song, NULL, NULL);
        		break;
        	case CID_FOCUS_IN:
        		//if (cid->bThreaded)
				//	g_timeout_add (15, (GSourceFunc) cid_threaded_animation, GINT_TO_POINTER(CID_FOCUS_IN));
				//else 
        			g_timeout_add (15, (GSourceFunc) cid_focus_in, NULL);
        		break;
        	case CID_FOCUS_OUT:
        		//if (cid->bThreaded)
				//	g_timeout_add (15, (GSourceFunc) cid_threaded_animation, GINT_TO_POINTER(CID_FOCUS_OUT));
				//else 
        			g_timeout_add (15, (GSourceFunc) cid_focus_out, NULL);
        		break;
		}	
	}
}
