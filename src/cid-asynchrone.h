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

/// Type of frequency for a periodic task. The frequency of the Task is divided by 2, 4, and 10 for each state.
typedef enum {
    CID_FREQUENCY_NORMAL = 0,
    CID_FREQUENCY_LOW,
    CID_FREQUENCY_VERY_LOW,
    CID_FREQUENCY_SLEEP,
    CID_NB_FREQUENCIES
} CidFrequencyState;

/// Definition of the asynchronous job, that does the heavy part.
typedef void (* CidGetDataAsyncFunc ) (gpointer pSharedMemory);
/// Definition of the synchronous job, that update the dock with the results of the previous job. Returns TRUE to continue, FALSE to stop
typedef gboolean (* CidUpdateSyncFunc ) (gpointer pSharedMemory);

/// Definition of a periodic and asynchronous Task.
struct _CidTask {
    /// ID of the timer of the Task.
    gint iSidTimer;
    /// ID of the timer to check the end of the thread.
    gint iSidTimerUpdate;
    /// Atomic value, set to 1 when the thread is running.
    gint iThreadIsRunning;
    /// function carrying out the heavy job.
    CidGetDataAsyncFunc get_data;
    /// function carrying out the update of the dock. Returns TRUE to continue, FALSE to stop.
    CidUpdateSyncFunc update;
    /// interval of time in seconds, 0 to run the Task once.
    gint iPeriod;
    /// state of the frequency of the Task.
    CidFrequencyState iFrequencyState;
    /// structure passed as parameter of the 'get_data' and 'update' functions. Must not be accessed outside of these 2 functions !
    gpointer pSharedMemory;
    /// timer to get the accurate amount of time since last update.
    GTimer *pClock;
    /// time elapsed since last update.
    double fElapsedTime;
    /// function called when the task is destroyed to free the shared memory (optionnal).
    GFreeFunc free_data;
    /// TRUE when the task has been discarded.
    gboolean bDiscard;
} ;


/** Launch a periodic Task, beforehand prepared with #cid_new_task. The first iteration is executed immediately. The frequency returns to its normal state.
*@param pTask the periodic Task.
*/
void cid_launch_task (CidTask *pTask);

/** Same as above but after a delay.
*@param pTask the periodic Task.
*@param fDelay delay in ms.
*/
void cid_launch_task_delayed (CidTask *pTask, double fDelay);

/** Create a periodic Task.
*@param iPeriod time between 2 iterations, possibly nul for a Task to be executed once only.
*@param get_data asynchonous function, which carries out the heavy job parallel to the dock; stores the results in the shared memory.
*@param update synchonous function, which carries out the update of the dock from the result of the previous function. Returns TRUE to continue, FALSE to stop.
*@param free_data function called when the Task is destroyed, to free the shared memory (optionnal).
*@param pSharedMemory structure passed as a parameter of the get_data and update functions. Must not be accessed outside of these  functions !
*@return the newly allocated Task, ready to be launched with #cid_launch_task. Free it with #cid_free_task.
*/
CidTask *cid_new_task_full (int iPeriod, CidGetDataAsyncFunc get_data, CidUpdateSyncFunc update, GFreeFunc free_data, gpointer pSharedMemory);

/** Create a periodic Task.
*@param iPeriod time between 2 iterations, possibly nul for a Task to be executed once only.
*@param get_data asynchonous function, which carries out the heavy job parallel to the dock; stores the results in the shared memory.
*@param update synchonous function, which carries out the update of the dock from the result of the previous function. Returns TRUE to continue, FALSE to stop.
*@param pSharedMemory structure passed as a parameter of the get_data and update functions. Must not be accessed outside of these  functions !
*@return the newly allocated Task, ready to be launched with #cid_launch_task. Free it with #cid_free_task.
*/
#define cid_new_task(iPeriod, get_data, update, pSharedMemory) cid_new_task_full (iPeriod, get_data, update, NULL, pSharedMemory)

/** Stop a periodic Task. If the Task is running, it will wait until the asynchronous thread has finished, and skip the update. The Task can be launched again with a call to #cid_launch_task.
*@param pTask the periodic Task.
*/
void cid_stop_task (CidTask *pTask);

/** Discard a periodic Task. The asynchronous thread will continue, and the Task will be freed when it ends. Use this function carefully, since you don't know when the free will occur (especially if you've set a free_data callback). The Task should be considered as destroyed after a call to this function.
*@param pTask the periodic Task.
*/
void cid_discard_task (CidTask *pTask);

/** Stop and destroy a periodic Task, freeing all the allocated ressources. Unlike \ref cid_discard_task, the task is stopped before being freeed, so this is a blocking call.
*@param pTask the periodic Task.
*/
void cid_free_task (CidTask *pTask);

/** Tell if a Task is active, that is to say is periodically called.
*@param pTask the periodic Task.
*@return TRUE if the Task is active.
*/
gboolean cid_task_is_active (CidTask *pTask);

/** Tell if a Task is running, that is to say it is either in the thread or waiting for the update.
*@param pTask the periodic Task.
*@return TRUE if the Task is running.
*/
gboolean cid_task_is_running (CidTask *pTask);

/** Change the frequency of a Task. The next iteration is re-scheduled according to the new period.
*@param pTask the periodic Task.
*@param iNewPeriod the new period between 2 iterations of the Task, in s.
*/
void cid_change_task_frequency (CidTask *pTask, int iNewPeriod);
/** Change the frequency of a Task and relaunch it immediately. The next iteration is therefore immediately executed.
*@param pTask the periodic Task.
*@param iNewPeriod the new period between 2 iterations of the Task, in s, or -1 to let it unchanged.
*/
void cid_relaunch_task_immediately (CidTask *pTask, int iNewPeriod);

/** Downgrade the frequency of a Task. The Task will be executed less often (this is typically useful to put on stand-by a periodic measure).
*@param pTask the periodic Task.
*/
void cid_downgrade_task_frequency (CidTask *pTask);
/** Set the frequency of the Task to its normal state. This is also done automatically when launching the Task.
*@param pTask the periodic Task.
*/
void cid_set_normal_task_frequency (CidTask *pTask);

/** Get the time elapsed since the last time the Task has run.
*@param pTask the periodic Task.
*/
#define cid_get_task_elapsed_time(pTask) (pTask->fElapsedTime)

G_END_DECLS
#endif
