/*
   *
   *                        cid-asynchrone.c
   *                             -------
   *                       Conky Images Display
   *             --------------------------------------------
   *
*/

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "cid-asynchrone.h"
#include "cid-messages.h"

#define cid_schedule_next_iteration(pTask) do {\
    if (pTask->iSidTimer == 0 && pTask->iPeriod)\
        pTask->iSidTimer = g_timeout_add_seconds (pTask->iPeriod, (GSourceFunc) _cid_timer, pTask); } while (0)

#define cid_cancel_next_iteration(pTask) do {\
    if (pTask->iSidTimer != 0) {\
        g_source_remove (pTask->iSidTimer);\
        pTask->iSidTimer = 0; } } while (0)

#define cid_perform_task_update(pTask) do {\
    gboolean bContinue = FALSE;\
    if (pTask->update != NULL){ \
        bContinue = pTask->update (pTask->pSharedMemory); }\
    if (! bContinue) {\
        cid_cancel_next_iteration (pTask); }\
    else {\
        pTask->iFrequencyState = CID_FREQUENCY_NORMAL;\
        cid_schedule_next_iteration (pTask); } } while (0)

#define cid_set_elapsed_time(pTask) do {\
    pTask->fElapsedTime = g_timer_elapsed (pTask->pClock, NULL);\
    g_timer_start (pTask->pClock); } while (0)

#define _free_task(pTask) do {\
    if (pTask->free_data)\
        pTask->free_data (pTask->pSharedMemory);\
    g_timer_destroy (pTask->pClock);\
    g_free (pTask); } while (0)

static gboolean 
_cid_timer (CidTask *pTask)
{
    cid_launch_task (pTask);
    return TRUE;
}

static gpointer 
_cid_threaded_calculation (CidTask *pTask)
{
    //\_______________________ On obtient nos donnees.
    cid_set_elapsed_time (pTask);
    pTask->get_data (pTask->pSharedMemory);
    
    //\_______________________ On indique qu'on a fini.
    g_atomic_int_set (&pTask->iThreadIsRunning, 0);
    return NULL;
}

static gboolean 
_cid_check_for_update (CidTask *pTask)
{
    int iThreadIsRunning = g_atomic_int_get (&pTask->iThreadIsRunning);
    if (! iThreadIsRunning)  // le thread a fini.
    {
        if (pTask->bDiscard)  // la tache s'est faite abandonnee.
        {
            //g_print ("free discared task...\n");
            _free_task (pTask);
            //g_print ("done.\n");
            return FALSE;
        }
        
        // On met a jour avec ces nouvelles donnees et on lance/arrete le timer.
        pTask->iSidTimerUpdate = 0;
        cid_perform_task_update (pTask);
        
        return FALSE;
    }
    return TRUE;
}

void 
cid_launch_task (CidTask *pTask)
{
    g_return_if_fail (pTask != NULL);
    if (pTask->get_data == NULL)  // pas de thread, tout est dans la fonction d'update.
    {
        cid_set_elapsed_time (pTask);
        cid_perform_task_update (pTask);
    }
    else
    {
        if (g_atomic_int_compare_and_exchange (&pTask->iThreadIsRunning, 0, 1))  // il etait egal a 0, on lui met 1 et on lance le thread.
        {
            GError *erreur = NULL;
            GThread* pThread = g_thread_create ((GThreadFunc) _cid_threaded_calculation, pTask, FALSE, &erreur);
            if (erreur != NULL)  // on n'a pas pu lancer le thread.
            {
                cid_warning ("%s",erreur->message);
                g_error_free (erreur);
                g_atomic_int_set (&pTask->iThreadIsRunning, 0);
            }
        }
        
        if (pTask->iSidTimerUpdate == 0)
            pTask->iSidTimerUpdate = g_timeout_add (MAX (100, MIN (0.10 * pTask->iPeriod, 333)), (GSourceFunc) _cid_check_for_update, pTask);
    }
}


static gboolean 
_cid_one_shot_timer (CidTask *pTask)
{
    pTask->iSidTimer = 0;
    cid_launch_task (pTask);
    return FALSE;
}

void 
cid_launch_task_delayed (CidTask *pTask, double fDelay)
{
    cid_cancel_next_iteration (pTask);
    if (fDelay == 0)
        pTask->iSidTimer = g_idle_add ((GSourceFunc) _cid_one_shot_timer, pTask);
    else
        pTask->iSidTimer = g_timeout_add (fDelay, (GSourceFunc) _cid_one_shot_timer, pTask);
}

CidTask *
cid_new_task_full (int iPeriod, CidGetDataAsyncFunc get_data, CidUpdateSyncFunc update, GFreeFunc free_data, gpointer pSharedMemory)
{
    CidTask *pTask = g_new0 (CidTask, 1);
    pTask->iPeriod = iPeriod;
    pTask->get_data = get_data;
    pTask->update = update;
    pTask->free_data = free_data;
    pTask->pSharedMemory = pSharedMemory;
    pTask->pClock = g_timer_new ();
    return pTask;
}


static void 
_cid_pause_task (CidTask *pTask)
{
    if (pTask == NULL)
        return ;
    
    cid_cancel_next_iteration (pTask);
    
    if (pTask->iSidTimerUpdate != 0)
    {
        g_source_remove (pTask->iSidTimerUpdate);
        pTask->iSidTimerUpdate = 0;
    }
}

void 
cid_stop_task (CidTask *pTask)
{
    if (pTask == NULL)
        return ;
    
    _cid_pause_task (pTask);
    
    cid_message ("***waiting for thread's end...(%d)", g_atomic_int_get (&pTask->iThreadIsRunning));
    while (g_atomic_int_get (&pTask->iThreadIsRunning))
        g_usleep (10);
        ///gtk_main_iteration ();
    cid_message ("***ended.");
}


static gboolean 
_free_discarded_task (CidTask *pTask)
{
    //g_print ("%s ()\n", __func__);
    cid_free_task (pTask);
    return FALSE;
}

void cid_discard_task (CidTask *pTask)
{
    if (pTask == NULL)
        return ;
    
    cid_cancel_next_iteration (pTask);
    g_atomic_int_set (&pTask->bDiscard, 1);
    
    if (pTask->iSidTimerUpdate == 0)
        pTask->iSidTimerUpdate = g_idle_add ((GSourceFunc) _free_discarded_task, pTask);
}

void 
cid_free_task (CidTask *pTask)
{
    if (pTask == NULL)
        return ;
    cid_stop_task (pTask);
    
    _free_task (pTask);
}

gboolean 
cid_task_is_active (CidTask *pTask)
{
    return (pTask != NULL && pTask->iSidTimer != 0);
}

gboolean 
cid_task_is_running (CidTask *pTask)
{
    return (pTask != NULL && pTask->iSidTimerUpdate != 0);
}

static void 
_cid_restart_timer_with_frequency (CidTask *pTask, int iNewPeriod)
{
    gboolean bNeedsRestart = (pTask->iSidTimer != 0);
    _cid_pause_task (pTask);
    
    if (bNeedsRestart && iNewPeriod != 0)
        pTask->iSidTimer = g_timeout_add_seconds (iNewPeriod, (GSourceFunc) _cid_timer, pTask);
}

void 
cid_change_task_frequency (CidTask *pTask, int iNewPeriod)
{
    g_return_if_fail (pTask != NULL);
    pTask->iPeriod = iNewPeriod;
    
    _cid_restart_timer_with_frequency (pTask, iNewPeriod);
}

void 
cid_relaunch_task_immediately (CidTask *pTask, int iNewPeriod)
{
    cid_stop_task (pTask);  // on stoppe avant car on ne veut pas attendre la prochaine iteration.
    if (iNewPeriod >= 0)  // sinon valeur inchangee.
        cid_change_task_frequency (pTask, iNewPeriod); // nouvelle frequence.
    cid_launch_task (pTask);  // mesure immediate.
}

void 
cid_downgrade_task_frequency (CidTask *pTask)
{
    if (pTask->iFrequencyState < CID_FREQUENCY_SLEEP)
    {
        pTask->iFrequencyState ++;
        int iNewPeriod;
        switch (pTask->iFrequencyState)
        {
            case CID_FREQUENCY_LOW :
                iNewPeriod = 2 * pTask->iPeriod;
            break ;
            case CID_FREQUENCY_VERY_LOW :
                iNewPeriod = 4 * pTask->iPeriod;
            break ;
            case CID_FREQUENCY_SLEEP :
                iNewPeriod = 10 * pTask->iPeriod;
            break ;
            default :  // ne doit pas arriver.
                iNewPeriod = pTask->iPeriod;
            break ;
        }
        
        cid_message ("degradation de la mesure (etat <- %d/%d)", pTask->iFrequencyState, CID_NB_FREQUENCIES-1);
        _cid_restart_timer_with_frequency (pTask, iNewPeriod);
    }
}

void 
cid_set_normal_task_frequency (CidTask *pTask)
{
    if (pTask->iFrequencyState != CID_FREQUENCY_NORMAL)
    {
        pTask->iFrequencyState = CID_FREQUENCY_NORMAL;
        _cid_restart_timer_with_frequency (pTask, pTask->iPeriod);
    }
}
