/*
 * */
#include <math.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "robot_common_data.h"
#include "RDSLog.h"
#include "DeviceMacros.h"
#include "DeviceTable.h"

#include "CommandObject.h"
#include "RDSTask.h"
#include "RDService.h"

namespace android
{
    #define SENDTIMEOUT 5000 //5 sec
    static int g_task_session_id = 0x0001;

    CommandObject::CommandObject(int sender_id, int cls_id)
    {
        this->sender_id = sender_id;
        this->cls_id = cls_id;

        timeout = 1000 * 5;
        task_id = 0xFF;
        startTime = 0;
        cmd_count = 1;
        board_id = 0;
        flags = 0;
        repeat = 0;
        streams = (unsigned char *) malloc(8);
        task_session_id = g_task_session_id++;
    }
    CommandObject::~CommandObject()
    {
        callback = 0;
        onResult = 0;
        if (streams != NULL) free(streams);
    }
    int CommandObject::Stop()
    {
        return 0;
    }
    int CommandObject::Finish(int result, int errorcode)
    {
        if(onResult.get() != 0 && errorcode == 0)
        {
            onResult->onCompletion(motor_id, result);
        }
        cmd_count--;
        if (cmd_count > 0 && errorcode == 0)
        {
		    return cmd_count;
        }
        if (sender_id > 0 && callback.get() == 0 && onResult.get() == 0)
        {
            RDService::Instance()->notifyCommandCompleted(sender_id, task_session_id, result, errorcode);
        }
        else if (callback.get() != 0)
        {
            callback->onCompletion(task_session_id, result, errorcode);
        }
        if (sender_id != 0)
        {
            delete this;
        }
        return 0;
    }
    int CommandObject::Send()
    {
        //struct timeval tm;
        if (board_id < 0 || RDSTask::Instance() == NULL)
        {
            LOGE("###", "erro 2");
            return -1;
        }
        //gettimeofday(&tm, NULL);
        //startTime =  (tm.tv_sec * 1000000 + tm.tv_usec)/1000;
        return RDSTask::Instance()->sendCommand(board_id, streams, streams[1]);
    }
    int CommandObject::startSend()
    {
        if (sender_id == 0)
        {
            int result = 0;
            task_id = 0xFF;
            result = Send();
            if (result < 0)
            {
                return result;
            }
            return task_session_id;
        }
        else
        {
            return RDSTask::Instance()->addCommand(this);
        }
    }
    bool CommandObject::Blocked(const CommandObject * obj)
    {
        if (Equal(obj) == false)
        {
            return false;
        }

        return ((flags & 0x80 ) != 0);
    }

    bool CommandObject::Equal(const CommandObject * chr)
    {
        if (cls_id == chr->cls_id)
        {
            return true;
        }
        return false;
    }

    //
    // HWSetting object
    //
    
    SettingObject::SettingObject(int sender_id) : CommandObject(sender_id, 0x01)
    {
        Clear();
    }
    SettingObject::~SettingObject()
    {
        Clear();
    }
    void SettingObject::Clear()
    {
        for (unsigned int i = 0; i < sizeof(idset); i++)
        {
            idset[i] = 0;
        }
        enabled = false;
        cmd_count = 0;
        val = 0;
    }
    int SettingObject::Finish(int result, int errorcode)
    {
        switch ( co_id )
        {
            case 0x01: // motor set enable
            case 0x03: // sensor set enable
                if (result != 0)
                {
                    enabled = false;
                }
                break;
            case 0x02: // query motor status
                if (result == 0)
                {
                    enabled = true;
                }
                break;
            case 0x04: // query sensor status
                if (result != 0)
                {
                    enabled = true;
                }
                break;
            case 0x05:
                break;
            case 0x06:
                if(result != 0)
                {
                    enabled = true;
                }
                break;
        }

        return CommandObject::Finish((co_id == 0x05) ? result : (enabled ? 1 : 0), errorcode);
    }
    int SettingObject::Send()
    {
        this->timeout = 1000 * 15;
        int cmd = DeviceTable::getSettingCommand(co_id);
        int dev_id = 0;

        streams[0] = cmd;
        streams[2] = (unsigned char)task_id;

        enabled = (co_id == 0x01 || co_id == 0x03) ? true : false;

        for (int i = 0; i < cmd_count; i++)
        {
            streams[1] = 0;
            switch(co_id)
             {
                 case 0x01://RB_SET_MOTION_ONOFF:
                     RDSTask::motorDisabled = false;
                 case 0x02://RB_QUERY_MOTION_ONOFF:
                 case 0x05://get motor angle
                     DeviceTable::getMotorAndBoardID(idset[i], dev_id, board_id);
                     motor_id = idset[i];
                     streams[1] = (co_id == 0x01) ? 5 : 4;
                     streams[3] = (unsigned char)dev_id;
                     streams[4] = (unsigned char)(val == 0 ? 1 : 0);
                     break;
                 case 0x03:
                 case 0x04:
                     DeviceTable::getMotorAndBoardID(0x39, dev_id, board_id);
                     dev_id = DeviceTable::getSensorId(idset[i]);
                     streams[1] = (co_id == 0x03) ? 4 : 3;
                     streams[2] = dev_id;
                     streams[3] = (unsigned char)val;
                     break;
                 case 0x06:
                    streams[1] = 4;
                    dev_id = DeviceTable::getSensorId(idset[i]); // dev_id = RS_GYRO
                    streams[3] = dev_id;
                    board_id = 0x14;
                    break;
                 default:
                     break;
            }
            if (streams[0] > 0)
            {
                CommandObject::Send();
            }
        }
        return 0;
    }
    bool SettingObject::Equal(const CommandObject * obj)
    {
        if (CommandObject::Equal(obj) == false)
        {
            return false;
        }
        return (co_id == obj->co_id);
    }
    int SettingObject::Run(int id, int cmd, int arg1, int arg2, const sp<IResultListener> & onResultListener)
    {
        Clear();
        if (id >= 0xF0)
        {
            cmd_count = DeviceTable::getSettingIDList(id, idset, sizeof(idset));
        }
        else
        {
            idset[0] = id;
            cmd_count = 1;
        }
        if (cmd_count == 0 || cmd == 0)
        {
            return Finish(0, -1);
        }
        this->val = arg1;
        this->co_id = cmd;
		 this->onResult = onResultListener;

       int result = startSend();
		if (co_id == 0x01 && val == 0x01) //RB_SET_MOTION_OFF
		{
			startTime = 2500;
			RDSTask::motorDisabled = true;
		}

        return result;
    }

    //
    // motor motion object
    //

    MotorMotion::MotorMotion(int sender_id) : CommandObject(sender_id, 0x02)
    {
        dataCap = 0;
        pData = NULL;
        Clear();
    }
    MotorMotion::~MotorMotion()
    {

    }
    void MotorMotion::Clear()
    {
        co_id = 0;
        dataSize = 0;
    }
    int MotorMotion::Send()
    {
        int val = 0;
        int motor_id = 0;

        if (dataSize > 0)
        {
            //format
            //0x8EF1:reserved(1):motor_id(1):angle/distance(2):flags(1):duration(1):direction(1):reserved(1)//8 byte/unit
            if (dataSize % 10 != 0)
            {
                LOGE("###", "motor motion send error ");
                return -1;
            }
            cmd_count = dataSize / 10;

            for (int i = 0; i < dataSize; )
            {
                i += 3; //skip reserved byte

                co_id = pData[i++];

                if (co_id == 0x3E) //
                {
                    int unit = pData[i];
                    i += 6;
                    ResetMotion::Run(unit);
                    continue;
                }
                if (co_id == 0x3F)
                {
                    int unit = pData[i];
                    i += 6;
                    StopMotion::Run(unit);
                    continue;
                }
                DeviceTable::getMotorAndBoardID(co_id, motor_id, board_id);
                if (board_id <= 0)
                {
                    i += 6;
                    continue;
                }
                //LOGE("####", "co_id:0x%x board_id:0x%x motor_id:0x%x", co_id, board_id, motor_id);
                if (board_id == RC_TRAJ_CTRL_ID) //
                {
                    PositionMotion::Send(task_id, co_id, &pData[i], 6);
                    i += 6;
                }
                else    //
                {
                    streams[0] = RB_MOTOR_CTRL_RUN; //type
                    streams[1] = 8;//length
                    streams[2] = task_id;
                    streams[3] = motor_id;
                    streams[4] = pData[i++]; //angle
                    streams[5] = pData[i++]; //angle
                    streams[6] = pData[i++]; //flags   
                    val = pData[i++]; // duration 100ms
                    if (val <= 0 || val > 100)
                    {
                        val = (duration < 100 ? 100 : duration)/100;
                    }
                    if (duration > 0)
                    {
                        val = duration / 100;
                    }
                    //LOGE("#####", "commobject:%d", val);
                    streams[7] = (unsigned char)val;
                    i++;//direct
                    i++;//reserved
                    CommandObject::Send();
                }
            }
        }
        else
        {
            DeviceTable::getMotorAndBoardID(co_id, motor_id, board_id);
            unsigned short ang = angle;

            streams[0] = RB_MOTOR_CTRL_RUN; //type
            streams[1] = 8;//length
            streams[2] = task_id;
            streams[3] = motor_id;
            streams[4] = (unsigned char)(ang & 0x00FF); //angle
            streams[5] = (unsigned char)((ang & 0xFF00) >> 8); //angle
            streams[6] = (unsigned char)(0x03 & flags); //flags
            streams[7] = (unsigned char)(duration / 100); //duration
            CommandObject::Send();

        }
        return 0;
    }
    int MotorMotion::Run(int joint_id, int angle, int duration, int flags)
    {
        Clear();
        this->co_id = joint_id;
        this->angle = angle;
        this->duration = duration;
        this->flags = flags;
        return startSend();
    }
    int MotorMotion::Run(const unsigned char * pdata, int len, int duration)
    {
        Clear();
        this->co_id = 0x2F; 
        if (len > dataCap)
        {
            if (pData != NULL) free(pData);
            pData = (unsigned char *)malloc(len);
            dataCap = len;
        }
        this->duration = duration;
        this->dataSize = len;
        memcpy(this->pData, pdata, len);

        return startSend();
    }

    bool MotorMotion::Equal(const CommandObject * co)
    {
        if (CommandObject::Equal(co) == false)
        {
            return false;
        }
        if (dataSize > 0)
        {
            //TODO
            return false;
        }
        MotorMotion * mo = (MotorMotion *)co;

        if (co_id == mo->co_id)
        {
            return true;
        }
        return false;
    }
    //
    // action motion
    //

    ActionMotion::ActionMotion(int sender_id) : CommandObject(sender_id, 0x04)
    {
        
    }
    ActionMotion::~ActionMotion()
    {
    }

    int ActionMotion::Send()
    {
        unsigned char motion_id = (unsigned char)DeviceTable::getSystemMotion(action_id);
        DeviceTable::getMotorAndBoardID((action_id & 0xFF00) >> 8, task_type_id, board_id);
        this->timeout = 1000 * 15;
        LOGE("###", "action send: action_id=0x%x motion_id=0x%x board_id=0x%x", action_id, motion_id, board_id);
        streams[0] = motion_id;
        streams[1] = 4;
        streams[2] = task_id;
        streams[3] = 1; 
        /*
        streams[0] = task_type_id;
        streams[1] = 6;
        streams[2] = task_id;
        streams[3] = motion_id; 
        streams[4] = 1; // 1 run, 0 stop
        streams[5] = 1; */

        CommandObject::Send();
        return 0;
    }
    int ActionMotion::Run(int action_id, int repeat, int duration)
    {
        this->co_id = 0x3A;
        this->task_type_id = 0;
        this->action_id = action_id;
        this->repeat = repeat;
        this->duration = duration;

        return startSend();
    }

    //
    // position motion object
    //

    PositionMotion::PositionMotion(int sender_id) : CommandObject(sender_id, 0x08)
    {
    }
    PositionMotion::~PositionMotion()
    {
    }
    int PositionMotion::newStopCommand()
    {
        int act_id = 0;
        unsigned char len = 0;
        if (co_id == 0x32/*maze*/)
        {
            DeviceTable::getMotorAndBoardID(co_id, act_id, board_id);
            len = 4;
            streams[0] = act_id;
            streams[1] = len;
            streams[2] = 0xFF;
            streams[3] = 0; 
        }
        else
        {
            DeviceTable::getMotorAndBoardID(0x3F/*stop*/, act_id, board_id);
            len = 5;
            streams[0] = act_id;
            streams[1] = len;
            streams[2] = 0xFF;
            streams[3] = ROBOT_STOP_WHEELS; 
            streams[4] = 0;
        }
        return len;
    }
    int PositionMotion::Stop()
    {
        int len = newStopCommand();
        RDSTask::Instance()->stopMotion(cls_id, board_id, streams, len);

        CommandObject::Finish(0, -1);
        return 0;
    }
    int PositionMotion::Send()
    {
        int act_id = 0;

        if (distance == 0)
        {
            LOGE("####", "invalid position params");
            return 0;
        }

        DeviceTable::getMotorAndBoardID(co_id, act_id, board_id);
        unsigned int val = (unsigned int)abs(distance);
        unsigned char len = 0;
        if(co_id == 0x32 || co_id == 0x34)
        {
            timeout = (val == 0xAAAA) ? -1 : /*maze/stroll*/ 1000 * 600;
            len = 4;
        }
        else
        {
            timeout = (val == 0xAAAA) ? -1 : 1000 * 60;
            len = 8;
        }
        streams[0] = act_id; //type //move/turn
        streams[1] = len;//length
        streams[2] = task_id;
        streams[3] = distance > 0 ? 1 : 0;
        streams[4] = (unsigned char)(val & 0x00FF); //angle/distance
        streams[5] = (unsigned char)((val & 0xFF00) >> 8); //angle/distance
        streams[6] = (unsigned char)speed; //speed   
        streams[7] = 0; //
        //LOGE("###", "send position--> dis:%d low:%d hight:%d", distance, streams[4], streams[5]);

        CommandObject::Send();
        return 0;
    }
    int PositionMotion::Send(int task_id, int act_id, unsigned char * pdata, int len)
    {
        //angle/distance(2):flags(1):speed(1):direction(1):reserved(1)//
        LOGE("###", "position send enter");
        PositionMotion pos;
        
        int value = ((int)pdata[0] | (int)(((int)pdata[1]) << 8));

        if (value == 0)
        {
            LOGE("####", "invalid position params act_id=0x%x", act_id);
            return 0;
        }
        int flags =  pdata[2];
        int speed = pdata[3];
        int direct = pdata[4];

        if (direct == 0)
        {
            value *= -1;
        }
        pos.task_id = task_id;
        pos.co_id = act_id;
        pos.distance = value;
        pos.speed = speed;

        if (act_id == 0x33)
        {
            LOGE("####", "position stop run");
            return pos.Stop();
        }
        return pos.Send();
    }

    int PositionMotion::Walk(int distance, int speed, int flag)
    {
        this->co_id = 0x30;

        if (distance == 0)
        {
           return Stop();
        }
        if ((flag & 0x01) != 0)
        {
            distance = 0xAAAA;
        }
        LOGE("###", " position walk %d", distance);

        this->distance = distance;
        this->speed = speed;
        
        return startSend();
    }
    int PositionMotion::WalkTo(int target, int speed, int flag)
    {
        if (target == 0)
        {
            this->co_id = 0x32;
        }
        else if(target == 1)
        {
            this->co_id = 0x34;
        }
        if(flag == 0)
        {
            this->distance = -1;
        }
        else
        {
            this->distance = 1;
        }
        this->speed = speed;
        return startSend();
    }
    int PositionMotion::Turn(int angle, int speed)
    {
        if (angle == 0)
        {
           return Stop();
        }
        this->co_id = 0x31;
        this->angle = angle;
        this->speed = speed;
        LOGE("###", " position turn %d", angle);

        return startSend();
    }
    //
    //emoji motion object
    //

    EmojiMotion::EmojiMotion(int sender_id) : CommandObject(sender_id, 0x0A)
    {
        co_id = 0x35;
    }
    EmojiMotion::~EmojiMotion()
    {
    }
    int EmojiMotion::Send()
    {
        int act_id = 0;
        DeviceTable::getMotorAndBoardID(co_id/*expression*/, act_id, board_id);

        if (board_id <= 0)
        {
            return 0;
        }
        streams[0] = act_id;
        streams[1] = 4;//length
        streams[2] = task_id;
        streams[3] = DeviceTable::getEmoji(id);
        CommandObject::Send();

        return task_session_id;
    }
    int EmojiMotion::Run(int id, int duration, int repeat)
    {
        this->id = id;
        this->duration = duration;
        this->repeat = repeat;
        return startSend();
    }

    //
    // stop
    //

    StopMotion::StopMotion(int sender_id) : CommandObject(sender_id, 0xF0)
    {
        co_id = 0x3F;
    }
    StopMotion::~StopMotion()
    {
    }
    int StopMotion::Send()
    {
        return 0;
    }
    int StopMotion::Run(int id, int bodyPart, int arg1)
    {
        int act_id, board_id;
        
        DeviceTable::getMotorAndBoardID(0x3F/*close*/, act_id, board_id);
        unsigned char streams[8];
        streams[0] = act_id;
        streams[1] = 5;
        streams[2] = 0xFF;
        streams[3] = DeviceTable::getRobotBodyPart(bodyPart);
        streams[4] = (arg1 == 0 ? 0 : 1);
        LOGE("###--------------", "stopmotion run................");
        return RDSTask::Instance()->sendCommand(board_id, streams, streams[1]);
    }
    int StopMotion::Run(int board_id, unsigned char * data)
    {
        unsigned char streams[8];
        memcpy(streams, data, 8);
        return RDSTask::Instance()->sendCommand(board_id, streams, streams[1]);
    }
    //
    // reset
    //
    ResetMotion::ResetMotion(int sender_id) : CommandObject(sender_id, 0xC0)
    {
        co_id = 0x3E;
    }
    ResetMotion::~ResetMotion()
    {
    }
    int ResetMotion::Send()
    {
        return 0;
    }
    int ResetMotion::Run()
    {
        return ResetMotion::Run(0);       
    }
    int ResetMotion::Run(int id)
    {
        if (id == 0x08/*ROBOT_RESET_EMOJI*/)
        {
            EmojiMotion emoji;
            return emoji.Run(DeviceTable::getEmoji(0x02), 0, 0);
        }
        else
        {
            int act_id, board_id;

            DeviceTable::getMotorAndBoardID(0x3E/*reset*/, act_id, board_id);
            unsigned char streams[8];
            streams[0] = act_id;
            streams[1] = 5;
            streams[2] = 0xFF;
            streams[3] = DeviceTable::getRobotBodyPart(id); 
            streams[4] = 1;    
            LOGE("###", "ResetMotion run(%d)", streams[3]);
            return RDSTask::Instance()->sendCommand(board_id, streams, streams[1]);
        }
    }
};
