/*
 */

#ifndef IRDS_DEATH_NOTIFIER_H_
#define IRDS_DEATH_NOTIFIER_H_

#include <utils/threads.h>
#include <utils/RefBase.h>
#include <binder/IBinder.h>
#include <utils/SortedVector.h>


namespace android
{

class IRDService;

class IDeathNotifier: virtual public RefBase
{
public:
    IDeathNotifier() { addObitRecipient(this); }
    virtual ~IDeathNotifier() { removeObitRecipient(this); }

    virtual void died() = 0;
    static const sp<IRDService>& GetRDService(int timeout = 0);

private:
    IDeathNotifier &operator=(const IDeathNotifier &);
    IDeathNotifier(const IDeathNotifier &);

    static void addObitRecipient(const wp<IDeathNotifier>& recipient);
    static void removeObitRecipient(const wp<IDeathNotifier>& recipient);

    class DeathRecipient: public IBinder::DeathRecipient
    {
    public:
                DeathRecipient() {}
        virtual ~DeathRecipient();

        virtual void binderDied(const wp<IBinder>& who);
    };

    friend class DeathRecipient;

    static  Mutex                                 sServiceLock;
    static  sp<IRDService>                        sRDService;
    static  sp<DeathRecipient>                    sDeathNotifier;
    static  SortedVector< wp<IDeathNotifier> >    sObitRecipients;
};

}; /* namespace android */

#endif /* RM_CAN_ICAN_DEATH_NOTIFIER_H_ */
