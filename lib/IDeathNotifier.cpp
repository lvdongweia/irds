/*
*/

#undef LOG_TAG
#define LOG_TAG "RDS.DEATH_NOTIFIER"

#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include "IRDService.h"
#include "IDeathNotifier.h"

#include "RDSLog.h"


namespace android {

/* client singleton for binder interface to services */
Mutex IDeathNotifier::sServiceLock;
sp<IRDService> IDeathNotifier::sRDService;
sp<IDeathNotifier::DeathRecipient> IDeathNotifier::sDeathNotifier;
SortedVector< wp<IDeathNotifier> > IDeathNotifier::sObitRecipients;

/* establish binder interface to RmCANService, static function */
const sp<IRDService>& IDeathNotifier::GetRDService(int timeout /*sec*/)
{
    Mutex::Autolock _l(sServiceLock);
    int tv = 0;/*msec*/

    if (sRDService == 0)
    {
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        do
        {
            binder = sm->getService(String16("rds.device.service"));
            if (binder != 0)
            {
                break;
            }
            if (tv >= timeout * 1000)
            {
                break;
            }
            usleep(500000); 
            tv += 500;
        } while (true);

        if (binder != 0)
        {
            if (sDeathNotifier == NULL)
            {
                sDeathNotifier = new DeathRecipient();
            }
            binder->linkToDeath(sDeathNotifier);
            sRDService = interface_cast<IRDService>(binder);
        }
    }
    if (sRDService == 0) LOGE("###", "no device service!?");
    return sRDService;
}

/*static*/
void IDeathNotifier::addObitRecipient(const wp<IDeathNotifier>& recipient)
{
    Mutex::Autolock _l(sServiceLock);
    sObitRecipients.add(recipient);
}

/*static*/
void IDeathNotifier::removeObitRecipient(const wp<IDeathNotifier>& recipient)
{
    Mutex::Autolock _l(sServiceLock);
    sObitRecipients.remove(recipient);
}

void IDeathNotifier::DeathRecipient::binderDied(const wp<IBinder>& who)
{
    LOGW(LOG_TAG, "can server died");

    /* Need to do this with the lock held */
    SortedVector< wp<IDeathNotifier> > list;
    {
        Mutex::Autolock _l(sServiceLock);
        sRDService.clear();
        list = sObitRecipients;
    }

    /* Notify application when can server dies.
     * Don't hold the static lock during callback in case app
     * makes a call that needs the lock. */
    size_t count = list.size();
    for (size_t iter = 0; iter < count; ++iter)
    {
        sp<IDeathNotifier> notifier = list[iter].promote();
        if (notifier != 0) {
            notifier->died();
        }
    }
}

IDeathNotifier::DeathRecipient::~DeathRecipient()
{
    Mutex::Autolock _l(sServiceLock);
    sObitRecipients.clear();
    if (sRDService != 0)
    {
        sRDService->asBinder()->unlinkToDeath(this);
    }
}

}; /* namespace android */
