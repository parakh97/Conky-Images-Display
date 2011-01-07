/*
   *
   *                    simple-dbus.h
   *                       -------
   *                 Conky Images Display
   *             ----------------------------
   *
   *
   *    Source originale: Cairo-dock
   *    Auteur origial: Adrien Pilleboue
*/
#ifndef __CID_DBUS__
#define  __CID_DBUS__

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

G_BEGIN_DECLS

/**
* Retourne la connexion 'session' de DBus.
* @return la connexion au bus.
*/
DBusGConnection *get_session_connection (void);
#define get_dbus_connection get_session_connection
/**
* Enregistre un nouveau service sur le bus.
* @param cServiceName le nom du service.
*/
void register_service_name (const gchar *cServiceName);
/**
* Dis si le bus est disponible.
* @return TRUE ssi le bus a pu etre recupere precedemment.
*/
gboolean dbus_is_enabled (void);

/**
* Cree un nouveau proxy pour la connexion 'session'.
* @param name un nom sur le bus.
* @param path le chemin associe.
* @param interface nom de l'interface associee.
* @return le proxy nouvellement cree.
*/
DBusGProxy *create_new_session_proxy (const char *name, const char *path, const char *interface);
#define create_new_dbus_proxy create_new_session_proxy
/**
* Cree un nouveau proxy pour la connexion 'system'.
* @param name un nom sur le bus.
* @param path le chemin associe.
* @param interface nom de l'interface associee.
* @return le proxy nouvellement cree.
*/
DBusGProxy *create_new_system_proxy (const char *name, const char *path, const char *interface);

/**
* Detecte si une application est couramment lancee.
* @param cName nom de l'application.
* @return TRUE ssi l'application est lancee et possede un service sur le bus.
*/
gboolean dbus_detect_application (const gchar *cName);

/**
* Recupere la valeur d'un parametre booleen sur le bus.
* @param pDbusProxy associe a la connexion.
* @param cParameter nom du parametre.
* @return la valeur du parametre.
*/
gboolean dbus_get_boolean (DBusGProxy *pDbusProxy, const gchar *cParameter);
/**
* Recupere la valeur d'un parametre entier non signe sur le bus.
* @param pDbusProxy associe a la connexion.
* @param cParameter nom du parametre.
* @return la valeur du parametre.
*/
guint dbus_get_uinteger (DBusGProxy *pDbusProxy, const gchar *cParameter);
/**
* Recupere la valeur d'un parametre 'chaine de caracteres' sur le bus.
* @param pDbusProxy associe a la connexion.
* @param cParameter nom du parametre.
* @return la valeur du parametre.
*/
gchar *dbus_get_string (DBusGProxy *pDbusProxy, const gchar *cParameter);
/**
* Appelle une commande sur le bus.
* @param pDbusProxy associe a la connexion.
* @param cCommand nom de la commande.
*/
void dbus_call (DBusGProxy *pDbusProxy, const gchar *cCommand);
/**
* Appelle une commande sur le bus.
* @param pDbusProxy associe a la connexion.
* @param cCommand nom de la commande.
* @param bAction lorsque le callback a besoin d'un boolean.
*/
void dbus_call_boolean (DBusGProxy *pDbusProxy, const gchar *cCommand, gboolean bAction);

G_END_DECLS
#endif
