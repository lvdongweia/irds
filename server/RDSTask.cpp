/*
 * */

#include <math.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "robot_common_data.h"
#include "RDSLog.h"
#include "CANSession.h"
#include "DeviceMacros.h"
#include "DeviceTable.h"

#include "CommandObject.h"
#include "RDSTask.h"
#include "RDService.h"


namespace android
{
    static Mutex listLock;
    static Mutex sendLock;

    RDSTask * RDSTask::instance = NULL;
	bool RDSTask::motorDisabled = false;

    RDSTask * RDSTask::Instance()
    {
        if (instance == NULL)
        {
            new RDSTask();
        }
        return instance;
    }
    //
    //
    RDSTask::RDSTask()
    {
        instance = this;

        for (int i = 0; i < 255; i++)
        {
            outList[i] = NULL;
        }
        RDService::Instance()->addFrameListener(this);
    }

    RDSTask::~RDSTask()
    {
        instance = NULL;
        RDService::Instance()->removeFrameListener(this);
    }
    void RDSTask::Destroy()
    {
        if (instance != NULL)
            delete instance;
        instance = NULL;
    }
    
    void RDSTask::checkTimeout(int timeSincelastFrame)
    {
        for (int i = 1; i < 252; i++)
        {
            CommandObject * obj = outList[i];
            if (obj == NULL) continue;
			obj->timeout -= timeSincelastFrame;
			if (obj->timeout <= 0)
			{
                obj->Finish(0, -2);
                outList[i] = NULL;
			}
        }
    }
    int RDSTask::update(int timeSincelastFrame)
    {
        Mutex::Autolock lock(listLock);
        int ind = 0;
        bool blocked = false;
        CommandObject * obj = NULL;

        List<CommandObject *>::iterator iter;

        for (iter = inList.begin(); iter != inList.end();)
        {
            while(iter != inList.end() && (obj = *iter) == NULL)
            {
                iter = inList.erase(iter);
            }
            if (iter == inList.end())
            {
                break;
            }

			obj->startTime -= timeSincelastFrame;
			if (obj->startTime > 0)
			{
                iter++;
				continue;
			}

            blocked = false;

            for(int i = 1; i < 252; i++)
            {
                if (outList[i] == NULL)
                {
                    ind = i;
                }
                if(outList[i] != NULL && outList[i]->Blocked(obj))
                {
                    blocked = true;
                }
            }
            if (ind == 0)
            {
                break;
            }
            if (blocked == true)
            {
                iter++;
                continue;
            }
            iter = inList.erase(iter);
            obj->task_id = ind;
            //LOGE("####", "task queue send comand:%d", ind);
            if (obj->Send() < 0)
            {
                obj->Finish(0, -4);
                break;
            }
            if (obj->timeout > 0)
            {
                outList[ind] = obj;
            }
            else
            {
                obj->Finish(0, 0);
            }
        }
        checkTimeout(timeSincelastFrame);
        return 0;
    }
    int RDSTask::addCommand(CommandObject * obj)
    {
        Mutex::Autolock lock(listLock);

		if (motorDisabled == true)
		{
			return 0;
		}

        inList.push_back(obj);
        return obj->task_session_id;
    }
    void RDSTask::stopMotion(int clsid, int dest, const unsigned char * streams, int len)
    {
        sendCommand(dest, streams, len);

        Mutex::Autolock lock(listLock);
        CommandObject * obj = NULL;

        for(int i = 1; i < 252; i++)
        {
            obj = outList[i];
            if (obj != NULL && obj->cls_id == clsid)
            {
                obj->Finish(0, -3);
                outList[i] = NULL;
            }
        }
        List<CommandObject *>::iterator iter = inList.begin();
        for (; iter != inList.end(); )
        {
            obj = (*iter);
            if (obj != NULL && obj->cls_id == clsid)
            {
                iter = inList.erase(iter);
                obj->Finish(0, -3);
            }
            else
            {
                iter++;
            }
        }
    }
    void RDSTask::cancelCommand(int session_id)
    {
        Mutex::Autolock lock(listLock);
        CommandObject * obj = NULL;

        if (session_id == 0)
        {
            inList.clear();
            for(int i = 1; i < 252; i++)
            {
                obj = outList[i];
                if (obj == NULL)
                {
                    continue;
                }
                obj->Finish(0, -3);
                outList[i] = NULL;
            }
        }
        else
        {
            for(int i = 1; i < 252; i++)
            {
                obj = outList[i];
                if (obj != NULL && obj->task_session_id == session_id)
                {
                    obj->Finish(0, -3);
                    outList[i] = NULL;
                }
            }
            List<CommandObject *>::iterator iter = inList.begin();
            for (; iter != inList.end();)
            {
                obj = (*iter);
                if (obj != NULL && obj->task_session_id == session_id)
                {
                    iter = inList.erase(iter);
                    obj->Finish(0, -3);
                }
                else
                {
                    iter++;
                }
            }
        }
    }

    int RDSTask::sendCommand(int dest, const unsigned char * streams, int len)
    {
        Mutex::Autolock lock(sendLock);

		if (motorDisabled == true)
		{
			return 0;
		}

        //LOGE("####", "start send Command to device");
        return CANSession::Instance()->Send(dest, streams, len);
    }
    void RDSTask::onCommandCompletion(int task_id, int result, int flags)
    {
        Mutex::Autolock lock(listLock);
        CommandObject * obj = NULL;
        int cmd = 0;

        if ((flags & 0x0F) == 0x01)
        {
            cmd = DeviceTable::getSysCtrlCmd(task_id);
        }
        for(int i = 1; i < 252; i++)
        {
            obj = outList[i];
            if (obj != NULL && 
                (((flags & 0x0F) == 0 && obj->task_id == task_id) ||
                ((flags & 0x0F) == 1 && obj->co_id == cmd))
               )
            {
                if (obj->Finish(result, 0) > 0)
                {
                    break;
                }
                outList[i] = NULL;
                break;
            }
        }
    }
};

