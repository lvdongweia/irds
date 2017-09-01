#ifndef RDS_MACROS__H_
#define RDS_MACROS__H__

#include "rm_message_defs.h"

enum RDSTopicEvent
{
    RDS_EVENT_TOUCH = 0x01,
    RDS_EVENT_SENSOR,
    RDS_EVENT_SYSTEM,
    RDS_EVENT_SOUND_LOCALIZATION,
    RDS_EVENT_SOUND_TEXT,
};

enum RobotResetTypes
{
     ROBOT_RESET_ALL        = 0,
     ROBOT_RESET_ARMS       = 1,
     ROBOT_RESET_LEFT_ARM   = 2,
     ROBOT_RESET_RIGHT_ARM  = 3,
     ROBOT_RESET_NECK       = 4,
     ROBOT_RESET_WAIST      = 5,
     ROBOT_RESET_EMOJI      = 0x12,
};
 
enum RobotStopTypes
{
     ROBOT_STOP_ALL        = 0,
     ROBOT_STOP_ARMS       = 1,
     ROBOT_STOP_LEFT_ARM   = 2,
     ROBOT_STOP_RIGHT_ARM  = 3,
     ROBOT_STOP_WHEELS     = 4,
     ROBOT_STOP_ACTIONS    = 0x11,
     ROBOT_STOP_EMOJI      = 0x12,
};


#define RM_SYSTEM_CMD 0xA7
#define RC_SAFE_CMD 0xA8


#endif

