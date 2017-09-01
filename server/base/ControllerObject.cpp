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
#include "ControllerObject.h"

namespace android
{
    //
    //controller object
    //

    ControllerObject::ControllerObject(int sender_id) : CommandObject(sender_id, 0x1000)
    {
        co_id = 0;
    }
    ControllerObject::~ControllerObject()
    {
    }
    int ControllerObject::Send()
    {
        int act_id = 0;
        DeviceTable::getMotorAndBoardID(target/*system control*/, act_id, board_id);

        if (board_id < 0)
        {
            LOGE("###", "ControllerObject send erro 5");
            return 0;
        }
        LOGE("### ", "controller send:board_id=0x%x target:0x%x co_id:%d", board_id, target, co_id );
        
        streams[0] = co_id;

        if (target == 0x38)
        {
            RbSerialization();
        }
        else if (target == 0x39)
        {
            RcSerialization();
        }
        else
        {
            RmSerialiaztion();
        }
        CommandObject::Send();
        return task_session_id;
    }
    int ControllerObject::RmSerialiaztion()
    {
        switch (co_id)
        {
            /* length == 2 */
            case COMMON_SYSCTRL_ABNORMAL_RESOLVE_NOTI:
            case COMMON_SYSCTRL_HALT_CMD:
            case COMMON_SYSCTRL_WAKE_CMD:
            case COMMON_SYSCTRL_SECURITY_CMD:
            case COMMON_SYSCTRL_EXIT_SECURITY_CMD:
            case RM_SYSCTRL_BATTERY_QUERY:
            case COMMON_SYSCTRL_UPGRADE_CMD:
            case COMMON_SYSCTRL_EXIT_UPGRADE_CMD:
            case COMMON_SYSCTRL_LOW_POWER_NOTI:
            case COMMON_SYSCTRL_NOT_LOW_POWER_NOTI:
                streams[1] = 2;
                break;
                /* length == 3 */
            case RM_SYSCTRL_REGISTERED_NOTI:
            case RM_SYSCTRL_LOGIN_NOTI:
            case RM_SYSCTRL_REGISTERED_QUERY_REP:
            case RM_SYSCTRL_LOGIN_QUERY_REP:
            case COMMON_SYSCTRL_INIT_CMD_REP:
            case RM_SYSCTRL_REGISTER_CMD_REP:
            case RM_SYSCTRL_LOGIN_CMD_REP:
            case COMMON_SYSCTRL_SLEEP_CMD:
            case COMMON_SYSCTRL_DEBUG_CMD:
            case COMMON_SYSCTRL_MODE_CMD:
            case RM_SYSCTRL_RESTORE_FACTORY_CMD:
            case COMMON_SYSCTRL_SLEEP_CMD_REP:
            case COMMON_SYSCTRL_WAKE_CMD_REP:
            case COMMON_SYSCTRL_SECURITY_CMD_REP:
            case COMMON_SYSCTRL_EXIT_SECURITY_CMD_REP:
            case COMMON_SYSCTRL_HALT_CMD_REP:
            case COMMON_SYSCTRL_DEBUG_CMD_REP:
            case COMMON_SYSCTRL_MODE_CMD_REP:
            case COMMON_SYSCTRL_UPGRADE_CMD_REP:
            case COMMON_SYSCTRL_EXIT_UPGRADE_CMD_REP:
                streams[1] = 3;
                streams[2] = (int8_t)arg1;
                break;

                /* length == 4 */
            case COMMON_SYSCTRL_SUBSYS_STATE_QUERY_REP:
            case COMMON_SYSCTRL_SUBSYS_STATE_CHANGE_NOTI:
                streams[1] = 4;
                streams[2] = (uint8_t)arg1; /* the first state */
                streams[3] = (uint8_t)arg2; /* the second state */
                break;

                /* length == 6 */
            case COMMON_SYSCTRL_ABNORMAL_OCCUR_NOTI:
                streams[1] = 6;
                streams[2] = (((int32_t)arg1) && 0xFF000000) >> 24;   /* big-endian */
                streams[3] = (((int32_t)arg1) && 0x00FF0000) >> 16;
                streams[4] = (((int32_t)arg1) && 0x0000FF00) >> 8;
                streams[5] = (((int32_t)arg1) && 0x000000FF);
                break;
            default:
                return -1;
        }

        return 0;

    }
    int ControllerObject::RbSerialization()
    {
        //arg1: joint id, arg2: ctrl arg

        int motor_id = 0;
        DeviceTable::getMotorAndBoardID(arg1, motor_id, board_id);

        switch (co_id)
        {
            //len = 5
            case RB_SET_MOTION_ONOFF:
                streams[1] = 5;
                streams[2] = (unsigned char)task_id;
                streams[3] = (unsigned char)motor_id;
                streams[4] = (unsigned char)arg2;
                break;

                //len = 4
            case RB_QUERY_MOTION_ONOFF:
            case RB_QUERY_MOTION_COUNT:
            case RB_QUERY_MOTION_STATE_QUERY:
                streams[1] = 4;
                streams[2] = (unsigned char)task_id;
                streams[3] = (unsigned char)motor_id;
                break;
            default:
                return -1;
        }
        return 0;
    }

    int ControllerObject::RcSerialization()
    {
        switch (co_id)
        {
            //len = 3
            case RM_QUERY_MOTION_SAFE_STATUS:
                streams[1] = 3;
                streams[2] = (uint8_t)arg1; // 00:query safe status, 01:query safe enable status;
                break;
            case RM_ENABLE_MOTION_SAFE_DETECT:
                streams[1] = 3;
                streams[2] = (uint8_t)arg1; //on-off flag
                break;
            case RM_QUERY_SENSER_OBSTACLE_FLAG:
                {
                    //sensor handle to rc-type
                    int sensor_id = DeviceTable::getSensorId((uint8_t)arg1);

                    streams[1] = 3;
                    streams[2] = (uint8_t)sensor_id;
                } 
				  break;
            case RM_ENABLE_SENSER_OBSTACLE:
                {
                    //sensor handle to rc-type
                    int sensor_id = DeviceTable::getSensorId((uint8_t)arg1);

                    streams[1] = 4;
                    streams[2] = (uint8_t)sensor_id;
                    streams[3] = (uint8_t)arg2; //on-off flag
                } 
			     break;
             case RC_LOW_POWER_CONSUMPTION:
                {
                    board_id = 0x10;
                    streams[1] = 4;
                    streams[2] = (uint8_t)arg1;
                    streams[3] = (uint8_t)arg2;
                }
            default:
                return -1;
        }
        return 0;
    }

    int ControllerObject::Run(int to, int cmd, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener)
    {
        this->co_id = cmd; 
        this->target = to;
        this->arg1 = arg1;
        this->arg2 = arg2;
        this->callback = onCompletionListener;
        return startSend();
    }
    bool ControllerObject::Equal(const CommandObject * co)
    {
        if (CommandObject::Equal(co) == false)
        {
            return false;
        }
        return (co_id == co->co_id);
    }
};
