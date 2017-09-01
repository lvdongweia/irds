/*
*/
#include <jni.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils/Mutex.h>

#include "RDSLog.h"
#include "RDSMacros.h"
#include "RDSTask.h"
#include "RDService.h"
#include "CANSession.h"
#include "rm_message_defs.h"

#undef LOG_TAG
#define LOG_TAG "RDS.CANSession"

namespace android
{
    CANSession * CANSession::instance = NULL;
    static char buffer_[16];

    CANSession::CANSession()
    {
        instance = this;

        m_canClient = new RmCANClient();
        m_canClient->RmSetReceiver(RM_SCHEDULER_ID, this);
    }
    CANSession::~CANSession()
    {
        m_canClient = NULL;
    }
    CANSession * CANSession::Instance()
    {
        if (instance == NULL)
        {
            new CANSession();
        }
        return instance;
    }
    
    void CANSession::RmRecvCANData(int priority, int src_id, const void *pdata, int len)
    {
        const unsigned char * pStream = (const unsigned char *) pdata;
        if (pStream == NULL) return;

        uint8_t length = 0;
        length = pStream[1];

        if (length != len || length <= 1)
        {
            return;
        }
        if (rawTopic.get() != 0)
        {
            rawTopic->onNotify(priority, src_id, pdata, len);
        }
        unsigned int system_id = (src_id & 0xF0) >> 4;
        switch (system_id)
        {
            case RB_R_ARM_ID:
            case RB_L_ARM_ID:
            case RB_BODY_ID:
                ParseRbMessage(pStream, len);
                break;
            case RF_SYSTEM_ID:
                ParseRfMessage(pStream, len);
                break;
            case RC_SYSTEM_ID:
                ParseRcMessage(pStream, len);
                break;
            case RM_SYSTEM_ID:
                ParseRmSysMessage(src_id, pStream, len);
                break;
            default:
                break;
        }
    }
    void CANSession::RmCANServiceDied()
    {
    }
    void CANSession::CompleteTask(const unsigned char *streams, int len)
    {
        unsigned char length = 0;

        length = streams[1];
        if(/*4 != length && */length != len) return;

        RDSTask::Instance()->onCommandCompletion(/*task_id*/ streams[2], /*result*/streams[3]);

    }

    void CANSession::RbCompleteTask(int type, const unsigned char *streams, int len)
    {
        switch (type)
        {
            case RB_QUERY_MOTION_STATE_QUERY_RESP:
                {
                    int length = streams[1];
                    if (length == len)
                    {
                        int task_id = streams[2];
                        int16_t angle = 0;
                        angle = streams[3] | (streams[4] << 8);

                        RDSTask::Instance()->onCommandCompletion(task_id, angle);
                    }
                }
                break;
            case RB_QUERY_MOTION_COUNT_RESP:
            {
                int length = streams[1];
                if (length == len)
                {
                    int task_id = streams[2];
                    int32_t count = 0;
                    count |= streams[3] << 24;
                    count |= streams[4] << 16;
                    count |= streams[5] << 8;
                    count |= streams[6];

                    RDSTask::Instance()->onCommandCompletion(task_id, count);
                }
            }   break;
            default:
                break;
        }

    }

    void CANSession::ReportEvent(int from, int event, int arg1, int arg2)
    {
        RDService::Instance()->notifyMessage(from, event, arg1, arg2);
    }

    int CANSession::ParseRcMessage(const unsigned char *streams, int len)
    {
        char type = 0;
        //Note: if 'type' is not unique in RcSystem, need module_id argument
        type = streams[0];
        switch (type)
        {
            case RC_REP_TSK:
                CompleteTask(streams, len);
                break;
            case RC_EVENT_REPORT:
                {
                    uint8_t event = streams[3];
                    if (len == 4)
                        ReportEvent(RC_SENSOR_TYPE, event, -1);
                    else if (len == 5)
                    {
                        uint8_t value = streams[4];
                        ReportEvent(RC_SENSOR_TYPE, event, value);
                    }
                    break;
                }
            case RM_QUERY_MOTION_SAFE_STATUS_RESP:
            case RM_ENABLE_MOTION_SAFE_DETECT_RESP:
            case RM_ENABLE_SENSER_OBSTACLE_RESP:
                {
                    int8_t result = streams[2];
                    RDSTask::Instance()->onCommandCompletion(type, result, 0x01);
                    break;
                }
                /* length == 4 */
            case RM_QUERY_SENSER_OBSTACLE_FLAG_RESP:
            case RC_LOW_POWER_CONSUMPTION_RESP:
                {
                    //uint8_t sensor_id = streams[2];
                    uint8_t result = streams[3];
                    RDSTask::Instance()->onCommandCompletion(type, result, 0x01);
                    break;
                }
                /* notify, rc->rm: safe-status have changed */
            case RM_CHANGE_MOTION_SAFE_STATUS:
                {
                    /* len = 4, s[2]: task_id */
                    int8_t status = streams[3]; //0: safe, 1: no safe
                    ReportEvent(RC_EVENT_TYPE, type, status);
                    break;
                }
            case ROBOT_SENSOR_RETVAL:
                {
                    if(len > 3)
                    {
                        CompleteTask(streams, len);
                    }
                    break;
                }
            default:
                break;
        }
        return 0;
    }

    int CANSession::ParseRbMessage(const unsigned char *streams, int len)
    {
        char type = 0;

        type = streams[0];
        switch (type)
        {
            case RB_TASK_RESP:
            case RB_SET_MOTION_ONOFF_RESP:
            case RB_QUERY_MOTION_ONOFF_RESP:
                {
                    CompleteTask(streams, len);
                    break;
                }
            case RB_QUERY_MOTION_STATE_QUERY_RESP:
            case RB_QUERY_MOTION_COUNT_RESP:
                {
                    RbCompleteTask(type, streams, len);
                    break;
                }
            case RB_EVENT_REPORT:
                {
                    uint8_t motor_id = streams[2];
                    uint8_t event    = streams[3];
                    ReportEvent(RB_EVENT_TYPE, event, motor_id);
                }   break;
            default:
                break;
        }
        return 0;
    }

    int CANSession::ParseRfMessage(const unsigned char *streams, int len)
    {
        char type = 0;

        type = streams[0];
        switch (type)
        {
            case RF_DISPLAY_CMD_REP:
            case RF_DISPLAY_ENABLE_CMD:
            case RF_DISPLAY_SWITCH_QUERY_REP:
            case RF_DISPLAY_EYE_ENABLE_CMD:
                CompleteTask(streams, len);
                break;
            case RF_EVENT_REPORT:
                {
                    if(len > 3)
                    {
                        uint8_t event = streams[2];
                        uint8_t position = streams[3];
                        ReportEvent(RF_EVENT_TYPE, event, position);
                    }

                } 
                break;
            default:
                break;
        }
        return 0;
    }

    int CANSession::ParseRmSysMessage(int src_id, const unsigned char *streams, int len)
    {
        char type = 0;
        
        type = streams[0];

        switch (type)
        {
            case COMMON_SYSCTRL_LOW_POWER_NOTI:
            case COMMON_SYSCTRL_NOT_LOW_POWER_NOTI:
                {
                    unsigned char bytes[2];
                    bytes[0] = type == COMMON_SYSCTRL_LOW_POWER_NOTI ? COMMON_SYSCTRL_LOW_POWER_NOTI_REP : COMMON_SYSCTRL_NOT_LOW_POWER_NOTI_REP;
                    bytes[1] = 2;
                    Send(src_id, bytes, 2);
                    RDSTask::Instance()->onCommandCompletion(type, 0, 0x01);
                }
                break;
                /* cmd, length == 2 */
            case COMMON_SYSCTRL_INIT_CMD:
            case RM_SYSCTRL_REGISTER_CMD:
            case RM_SYSCTRL_LOGIN_CMD:
            case COMMON_SYSCTRL_SUBSYS_STATE_QUERY:
            case RM_SYSCTRL_REGISTERED_QUERY:
            case RM_SYSCTRL_LOGIN_QUERY:
            case COMMON_SYSCTRL_WAKE_CMD:
            case COMMON_SYSCTRL_SECURITY_CMD:
            case COMMON_SYSCTRL_EXIT_SECURITY_CMD:
            case COMMON_SYSCTRL_HALT_CMD:
            case COMMON_SYSCTRL_UPGRADE_CMD:
            case COMMON_SYSCTRL_EXIT_UPGRADE_CMD:
                /* notify, length == 2 */
            case COMMON_SYSCTRL_POWER_CHARGE_NOTI:
            case COMMON_SYSCTRL_POWER_CHARGE_END_NOTI:
                ReportEvent(RM_EVENT_TYPE, type, 0);
                break;
                /* cmd, length == 3 */
            case COMMON_SYSCTRL_SLEEP_CMD:
            case COMMON_SYSCTRL_DEBUG_CMD:
            case COMMON_SYSCTRL_MODE_CMD:
                /* notify, length == 3 */
            case COMMON_SYSCTRL_SYS_STATE_CHANGE_NOTI:
            case COMMON_SYSCTRL_HDMI_STATE_CHANGE_NOTI:
                {
                    int8_t arg1 = streams[2];
                    ReportEvent(RM_EVENT_TYPE, type, arg1);
                    break;
                }
                /* reply, length == 3 */
            case COMMON_SYSCTRL_SLEEP_CMD_REP:
            case COMMON_SYSCTRL_WAKE_CMD_REP:
            case COMMON_SYSCTRL_SECURITY_CMD_REP:
            case COMMON_SYSCTRL_EXIT_SECURITY_CMD_REP:
            case COMMON_SYSCTRL_HALT_CMD_REP:
            case COMMON_SYSCTRL_DEBUG_CMD_REP:
            case COMMON_SYSCTRL_MODE_CMD_REP:
            case RM_SYSCTRL_RESTORE_FACTORY_CMD_REP:
            case RM_SYSCTRL_BATTERY_QUERY_REP:
            case COMMON_SYSCTRL_UPGRADE_CMD_REP:
            case COMMON_SYSCTRL_EXIT_UPGRADE_CMD_REP:
                {
//                  int8_t result = streams[2];
                    RDSTask::Instance()->onCommandCompletion(type, /*result*/streams[2], 0x01);
                } break;

            default:
                ALOGD("RmTransceivers: unknown system-control cmd %d", type);
                break;

        }
        return 0;
    }

    int CANSession::Send(int dest, const unsigned char * streams, int len)
    {
        /*int i = 0;
        for (; i < len; i++)
        {
            if (i == 14)
            {
                break;
            }
            sprintf(&buffer_[i], "%x", (int)streams[i]);
        }
        buffer_[i] = 0;
        ALOGD("### CanSession Send=> dest:%d streams:0x%s", dest, buffer_);*/
        //ALOGD("### dest=%d command:%d len=%d", dest, streams[0], len);
        if(streams[0] != 128 && streams[0] != 25 && streams[0] != 66 && streams[0] != 146)
        {
            LOGE("###", "dest=%d command:%d len=%d", dest, streams[0], len);
        }
        m_canClient->RmSendCANData(dest, streams, len);
        return 0;
    }
    void CANSession::SetRawTopic(sp<ITopic> topic)
    {
        rawTopic = topic;
    }
}; //namespace
