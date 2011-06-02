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
   *                        cid-asynchrone.h
   *                             -------
   *                       Conky Images Display
   *             --------------------------------------------
   *
*/

#ifndef __CID_ASYNCHRONE__
#define  __CID_ASYNCHRONE__

#include <gtk/gtk.h>
G_BEGIN_DECLS

typedef struct _CidTask CidTask;

typedef enum {
    CID_FREQUENCY_NORMAL = 0,
    CID_FREQUENCY_LOW,
    CID_FREQUENCY_VERY_LOW,
    CID_FREQUENCY_SLEEP,
    CID_NB_FREQUENCIES
} CidFrequencyState;

/// Definition de la fonction asynchrone.
typedef void (* CidGetDataAsyncFunc ) (gpointer pSharedMemory);
/// Definition de la fonction synchrone qui met à jour CID une fois 
/// la fonction asynchrone terminee. Retourne FALSE pour terminer, 
/// et TRUE pour continuer.
typedef gboolean (* CidUpdateSyncFunc ) (gpointer pSharedMemory);

/// Definition d'une tache periodique et asynchrone.
struct _CidTask {
    /// ID du timer de la tache.
    gint iSidTimer;
    /// ID du timer pour controler la fin du thread.
    gint iSidTimerUpdate;
    /// Valeure atomique, vaut 1 quand le thread est en cours.
    gint iThreadIsRunning;
    /// fonction asynchrone.
    CidGetDataAsyncFunc get_data;
    /// function synchrone appelee lorsque get_data a termine.
    CidUpdateSyncFunc update;
    /// interval de temps en secondes, 0 pour lancer la tache une 
    /// seule fois.
    gint iPeriod;
    /// etat de la frequence de la tache.
    CidFrequencyState iFrequencyState;
    /// structure passee en parametre a 'get_data' et 'update'. 
    /// Ne doit jamais etre modifiee en dehors de ces 2 fonctions !
    gpointer pSharedMemory;
    /// timer pour connaitre le temps ecoule depuis la derniere 
    /// iteration.
    GTimer *pClock;
    /// temps ecoule depuis le dernier update.
    double fElapsedTime;
    /// fonction appelee lorsque la tache est liberee pour liberer 
    /// la memoire partagee (optionnel).
    GFreeFunc free_data;
    /// TRUE quand la tache a ete abandonnee.
    gboolean bDiscard;
} ;


/** 
 * Lance une tache periodique prealablement declaree avec #cid_task_new.
 * La premiere iteration est lancee immediatement.
 * @param pTask la tache periodique.
 */
void cid_launch_task (CidTask *pTask);

/** 
 * Pareil que #cid_launch_task a part que la premiere iteration a 
 * lieu apres le delai fourni en parametre.
 * @param pTask la tache periodique.
 * @param fDelay delai en ms.
 */
void cid_launch_task_delayed (CidTask *pTask, double fDelay);

/** 
 * Cree une tache periodique
 * @param iPeriod temps entre 2 iteration. Si 0, la tache n'est executee
 * qu'une seule fois.
 * @param get_data fonction asynchrone executee dans un thread 
 * parallele.
 * @param update fonction synchrone executee a la fin de la fonction 
 * get_data. Retourne TRUE pour continuer, FALSE pour arreter.
 * @param free_data fonction appelee lorsque la tache est liberee pour 
 * liberer la memoire partagee (optionnel).
 * @param pSharedMemory structure passee en parametre a 'get_data' et 
 * 'update'. Ne doit jamais etre modifiee en dehors de ces 2 fonctions !
 * @return la nouvelle tache alouee a lancer avec #cid_launch_task. 
 * A liberer avec #cid_free_task.
 */
CidTask *cid_new_task_full (int iPeriod, 
                            CidGetDataAsyncFunc get_data, 
                            CidUpdateSyncFunc update, 
                            GFreeFunc free_data, 
                            gpointer pSharedMemory);

/** 
 * Cree une tache periodique
 * @param iPeriod temps entre 2 iteration. Si 0, la tache n'est 
 * executee qu'une seule fois.
 * @param get_data fonction asynchrone executee dans un thread 
 * parallele.
 * @param update fonction synchrone executee a la fin de la fonction 
 * get_data. Retourne TRUE pour continuer, FALSE pour arreter.
 * @param pSharedMemory structure passee en parametre a 'get_data' et 
 * 'update'. Ne doit jamais etre modifiee en dehors de ces 2 fonctions !
 * @return la nouvelle tache alouee a lancer avec #cid_launch_task. 
 * A liberer avec #cid_free_task.
 */
#define cid_new_task(iPeriod, get_data, update, pSharedMemory) \
    cid_new_task_full (iPeriod, get_data, update, NULL, pSharedMemory)

/** 
 * Arrete une tache. Si une tache est en cours, on attend que le thread 
 * asynchrone termine, et on saute l'update.
 * On peut relancer la tache avec #cid_launch_task.
 * @param pTask la tache.
 */
void cid_stop_task (CidTask *pTask);

/** 
 * Abandonne une tache. La fonction asynchrone continue, et la memoire 
 * est liberee a la fin. 
 * A utiliser avec precaution puisqu'on ne peut pas savoir quand la 
 * liberation a lieu (en particulier si on ajoute une fonction de 
 * callback free_data). 
 * On peut considerer la tache detruite une fois cette fonction appelee.
 * @param pTask la tache.
 */
void cid_discard_task (CidTask *pTask);

/** 
 * Arrete et detruit une tacheen liberant les resources. A la 
 * difference de \ref cid_discard_task, la tache est arretee avant 
 * d'etre liberer, c'est donc un appel bloquant.
 * @param pTask la tache.
 */
void cid_free_task (CidTask *pTask);

/** 
 * Permet de savoir si une tache est active.
 * Autrement dit, est-elle appelee periodiquement ?
 * @param pTask la tache.
 * @return TRUE si elle est active.
 */
gboolean cid_task_is_active (CidTask *pTask);

/** 
 * Permet de savoir si une tache est en cours. 
 * Autrement dit, un thread est-il en cours ou attend-on un update ?
 * @param pTask la tache.
 * @return TRUE si elle est en cours.
 */
gboolean cid_task_is_running (CidTask *pTask);

/** 
 * Change la frequence d'une tache. La prochaine iteration est 
 * re-plannifiee en fonction de la nouvelle periode.
 * @param pTask la tache.
 * @param iNewPeriod la nouvelle periode entre deux iteration en s.
*/
void cid_change_task_frequency (CidTask *pTask, int iNewPeriod);

/** 
 * Change la frequence d'une tache est la relance immediatement. 
 * @param pTask la tache.
 * @param iNewPeriod la nouvelle periode entre deux iteration 
 * en s (-1 pour ne rien changer).
 */
void cid_relaunch_task_immediately (CidTask *pTask, int iNewPeriod);

/** 
 * Diminue la frequence d'une tache
 * @param pTask the periodic Task.
 */
void cid_downgrade_task_frequency (CidTask *pTask);

/** 
 * Reinitialise la frequence d'une tache (automatiquement fait au 
 * lancement d'une nouvelle tache).
 * @param pTask la tache.
 */
void cid_set_normal_task_frequency (CidTask *pTask);

/** 
 * Temps ecoule depuis le dernier lancement de la tache
 * @param pTask la tache.
 */
#define cid_get_task_elapsed_time(pTask) (pTask->fElapsedTime)

G_END_DECLS
#endif
