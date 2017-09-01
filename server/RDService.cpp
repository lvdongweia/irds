/*
*/
/* Proxy for CAN implementations */


#define LOG_TAG "RDS"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include "RDSLog.h"

#include "RDSTask.h"

//service
#include "service/RawCommandService.h"
#include "service/SystemController.h"
#include "service/MotionService.h"
#include "service/MotionPlayer.h"
#include "service/AdbForwardService.h"
//
#include "CANSession.h"
#include "ITopic.h"
#include "RDService.h"
#include "IMessageListener.h"

namespace android
{
//
    RDService * RDService::instance = NULL;

    RDService * RDService::Instance()
    {
        if (instance == NULL)
        {
            defaultServiceManager()->addService(String16("rds.device.service"), new RDService());
        }
        return instance;
    }
    RDService::Looper::Looper()
    {
        bStop = false;
        frameTime = 100;
    }
    RDService::Looper::~Looper()
    {
        bStop = true;
    }

    bool RDService::Looper::threadLoop()
    {
        if (bStop == true)
        {
            return false;
        }
        usleep(1 * 1000 * frameTime);/**/

        RDService::Instance()->update();
        Mutex::Autolock lock(listLock);

        List<IFrameListener*>::iterator iter = listenerList.begin();
        for (; iter != listenerList.end() && bStop == false; iter++)
        {
            (*iter)->update(frameTime);
        }
        return (bStop == false);
    }
    void RDService::Looper::stop()
    {
        bStop = true;
        requestExitAndWait();
    }
    void RDService::Looper::addFrameListener(IFrameListener * listener)
    {
        Mutex::Autolock lock(listLock);

        List<IFrameListener*>::iterator iter = listenerList.begin();
        for (; iter != listenerList.end(); iter++)
        {
            if ((*iter) == listener)
            {
                return;
            }
        }
        listenerList.push_front(listener);
    }
    void RDService::Looper::removeFrameListener(IFrameListener * listener)
    {
        Mutex::Autolock lock(listLock);

        List<IFrameListener*>::iterator iter = listenerList.begin();
        for (; iter != listenerList.end(); iter++)
        {
            if ((*iter) == listener)
            {
                listenerList.erase(iter);
                break;
            }
        }
    }
    RDService::RDService()
    {
        instance = this; 
        Main();
    }

    RDService::~RDService()
    {
        RDSTask::Destroy();
        rawCommandService = NULL;
        looper->stop();
        looper.clear();
        adbForwardService->Stop();
    } 
    void RDService::Main()
    {
        looper = new Looper();
        looper->run("rds main looper");

        CANSession::Instance();
        RDSTask::Instance();//->run("rds main looper");
        adbForwardService = new AdbForwardService();
        adbForwardService->Run();
    }
    void RDService::update()
    {
        Mutex::Autolock lock(serviceLock);
        List< sp<SystemController> >::iterator iter0 = systemControllers.begin();
        for (; iter0 != systemControllers.end(); iter0++)
        {
            sp<SystemController> srv = (*iter0);
            if (srv->isUnset == true)
            {
                systemControllers.erase(iter0);
                srv = NULL;
                break;
            }
        }
        List< sp<MotionService> >::iterator iter1 = motionServices.begin();
        for (; iter1 != motionServices.end(); iter1++)
        {
            sp<MotionService> srv = (*iter1);
            if (srv->isUnset == true)
            {
                motionServices.erase(iter1);
                srv = NULL;
                break;
            }
        }
        List< sp<MotionPlayer> >::iterator iter2 = motionPlayers.begin();
        for (; iter2 != motionPlayers.end(); iter2++)
        {
            sp<MotionPlayer> srv = (*iter2);
            if (srv->isUnset == true)
            {
                motionPlayers.erase(iter2);
                srv = NULL;
                break;
            }
        }
    }
    void RDService::addFrameListener(IFrameListener * listener)
    {
        looper->addFrameListener(listener);
    }
    void RDService::removeFrameListener(IFrameListener * listener)
    {
        looper->removeFrameListener(listener);
    }
    void RDService::notifyCommandCompleted(int sender_id, int session_id, int result, int errorcode)
    {
        Mutex::Autolock lock(serviceLock);
        List< sp<MotionService> >::iterator iter = motionServices.begin();
        for (; iter != motionServices.end(); iter++)
        {
            if ((*iter)->uid == sender_id && (*iter)->isUnset == false)
            {
                (*iter)->onCompletion(session_id, result, errorcode);
            }
        }
    }
    void RDService::notifyMessage(int from, int what, int arg1, int arg2)
    {
        Mutex::Autolock lock(serviceLock);
        List< sp<SystemController> >::iterator iter = systemControllers.begin();
        for (; iter != systemControllers.end(); iter++)
        {
            if ((*iter)->isUnset == false && (*iter)->listener.get() != 0)
            {
                (*iter)->listener->onMessage(from, what, arg1, arg2);
            }
        }
    }
    sp<IRawCommandService> RDService::getRawCommandService(const void * token, int len)
    {
        rawCommandService = new RawCommandService(token, len);
        return rawCommandService; 
    }
    sp<ISystemController> RDService::getSystemController(const void * token, int len)
    {
        Mutex::Autolock lock(serviceLock);
        SystemController * srv = new SystemController(token, len);
        systemControllers.push_front(srv);
        return srv;
    }

    sp<IMotionService> RDService::getMotionService(const void * token, int len)
    {
        Mutex::Autolock lock(serviceLock);
        MotionService * srv = new MotionService(token, len);
        motionServices.push_front(srv);
        return srv;
    }
    sp<IMotionPlayer> RDService::getMotionPlayer(const void * token, int len)
    {
        Mutex::Autolock lock(serviceLock);
        MotionPlayer * srv = new MotionPlayer(token, len);
        motionPlayers.push_front(srv);
        return srv;
    }
    int RDService::subscribe(const sp<ITopic> & topic, const void * token, int len)
    {
        CANSession::Instance()->SetRawTopic(topic);
        return 0; 
    }

};

