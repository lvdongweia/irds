#include <stdio.h>

#include "robot_common_data.h"
#include "DeviceMacros.h"
#include "DeviceTable.h"

namespace DeviceTable
{
    static const struct MotorAndBoard
    {
        short motor_id;
        short board_id;
    } MotorAndBoardList[] =
    {
        //
        // index : joint id, value : {motor id, board_id};
        // ....
        /* start*/{ 0, -1 },
        /* 0x01 */{ ARM_ROTATION_RIGHT, RB_R_ARM_MOTOR_CTRL_ID },       /* 右大臂_前后旋转 */
        /* 0x02 */{ ARM_SWING_RIGHT, RB_R_ARM_MOTOR_CTRL_ID },          /* 右大臂_左右摆动 */
        /* 0x03 */{ FOREARM_ROTATION_RIGHT, RB_R_ARM_MOTOR_CTRL_ID },   /* 右小臂_左右旋转 */
        /* 0x04 */{ FOREARM_SWING_RIGHT, RB_R_ARM_MOTOR_CTRL_ID },      /* 右小臂_上下摆动 */
        /* 0x05 */{ WRIST_RIGHT, RB_R_ARM_MOTOR_CTRL_ID },              /* 右手腕 */
        /* 0x06 */{ 0, 0 },     /* 保留 */
        /* 0x07 */{ 0, 0 },     /* 保留 */
        /* 0x08 */{ 0, 0 },     /* 保留 */
        /* 0x09 */{ ARM_ROTATION_LEFT, RB_L_ARM_MOTOR_CTRL_ID },        /* 左大臂_前后旋转 */
        /* 0x0A */{ ARM_SWING_LEFT, RB_L_ARM_MOTOR_CTRL_ID },           /* 左大臂_左右摆动 */
        /* 0x0B */{ FOREARM_ROTATION_LEFT, RB_L_ARM_MOTOR_CTRL_ID },    /* 左小臂_左右旋转 */
        /* 0x0C */{ FOREARM_SWING_LEFT, RB_L_ARM_MOTOR_CTRL_ID },       /* 左小臂_上下摆动 */
        /* 0x0D */{ WRIST_LEFT, RB_L_ARM_MOTOR_CTRL_ID },               /* 左手腕 */
        /* 0x0E */{ 0, 0 },     /* 保留 */
        /* 0x0F */{ 0, 0 },     /* 保留 */
        /* 0x10 */{ 0, 0 },     /* 保留 */
        /////////////////////////////////////////////////////////////////////////////////////
        /* 0x11 */{ WHEEL_LEFT, RB_BODY_MOTOR_CTRL_ID },     /* 左轮 */
        /* 0x12 */{ WHEEL_RIGHT, RB_BODY_MOTOR_CTRL_ID },     /* 右轮 */
        /* 0x13 */{ WAIST_TILT, RB_BODY_MOTOR_CTRL_ID },                /* 腰_俯仰 */
        /* 0x14 */{ NECK_ROTATION, RB_BODY_MOTOR_CTRL_ID },             /* 脖子_旋转 */
        /* 0x15 */{ NECK_TILT, RB_BODY_MOTOR_CTRL_ID },                 /* 脖子_俯仰 */
        /* 0x16 */{ 0, 0 },     /* 保留 */
        /* 0x17 */{ 0, 0 },     /* 保留 */
        /* 0x18 */{ 0, 0 },                /* 保留 */
        /* 0x19 */{ 0, 0 },               /* 保留 */
        /* 0x1A */{ 0, 0 },     /* 保留 */
        /* 0x1B */{ 0, 0 },     /* 保留 */
        /* 0x1C */{ 0, 0 },     /* 保留 */
        /* 0x1D */{ 0, 0 },     /* 保留 */
        /* 0x1E */{ 0, 0 },     /* 保留 */
        /* 0x1F */{ 0, 0 },     /* 保留 */

        /* 0x20 */{ THUMB_RIGHT, RB_R_ARM_MOTOR_CTRL_ID },              /* 右拇指 */
        /* 0x21 */{ FOREFINGER_RIGHT, RB_R_ARM_MOTOR_CTRL_ID },         /* 右食指 */
        /* 0x22 */{ MIDDLEFINGER_RIGHT, RB_R_ARM_MOTOR_CTRL_ID },       /* 右中指 */
        /* 0x23 */{ RINGFINGER_RIGHT, RB_R_ARM_MOTOR_CTRL_ID },         /* 右无名指 */
        /* 0x24 */{ LITTLEFINGER_RIGHT, RB_R_ARM_MOTOR_CTRL_ID },       /* 右小手指 */

        /* 0x25 */{ THUMB_LEFT, RB_L_ARM_MOTOR_CTRL_ID },               /* 左拇指 */
        /* 0x26 */{ FOREFINGER_LEFT, RB_L_ARM_MOTOR_CTRL_ID },          /* 左食指 */
        /* 0x27 */{ MIDDLEFINGER_LEFT, RB_L_ARM_MOTOR_CTRL_ID },        /* 左中指 */
        /* 0x28 */{ RINGFINGER_LEFT, RB_L_ARM_MOTOR_CTRL_ID },          /* 左无名指 */
        /* 0x29 */{ LITTLEFINGER_LEFT, RB_L_ARM_MOTOR_CTRL_ID },        /* 左小手指 */

        /* 0x2A */{ 0, 0 },     /* 保留 */
        /* 0x2B */{ 0, 0 },     /* 保留 */
        /* 0x2C */{ 0, 0 },     /* 保留 */
        /* 0x2D */{ 0, 0 },     /* 保留 */
        /* 0x2E */{ 0, 0 },     /* 保留 */
        /* 0x2F */{ 0, 0 },     /* 保留 */

        /* 0x30 */{ RC_TARJ_MOVE, RC_TRAJ_CTRL_ID },        /* 前进 */
        /* 0x31 */{ RC_TARJ_TURN, RC_TRAJ_CTRL_ID },        /* 转向 */
        /* 0x32 */{ RC_PLAN_MAZE, RC_TASK_CTRL_ID },        /* 保留 */
        /* 0x33 */{ 0,  RC_TRAJ_CTRL_ID },     /*stop move/turn*/
        /* 0x34 */{ RC_PLAN_STROLL, RC_TASK_CTRL_ID },     /* 保留 */
        /* 0x35 */{ RF_DISPLAY_CMD, RF_DISPLAY_ID },     /* 保留 */
        /* 0x36 */{ 0, 0 },     /* 保留 */
        /* 0x37 */{ 0, 0 },     /* 保留 */
        /* 0x38 */{ 0, RC_TRAJ_CTRL_ID },     /* RB_CTRL_CMD  */
        /* 0x39 */{ RC_SAFE_CMD, RC_MOTION_SAFE_DETECT_ID },     /* RC_SAFE_CMD */
        /* 0x3A */{ RC_TRAJ_TASK, RC_TRAJ_CTRL_ID },
        /* 0x3B */{ 0, 0 },     /* 保留 */
        /* 0x3C */{ RC_PLAN_TASK, RC_TASK_CTRL_ID },     /* 保留 */
        /* 0x3D */{ 0, RM_SYSCTRL_ID },     /* system control */
        /* 0x3E */{ RC_TRAJ_RESET, RC_TRAJ_CTRL_ID },        /* reset */
        /* 0x3F */{ RC_TRAJ_STOP, RC_TRAJ_CTRL_ID },         /* stop */

        /* 0x40  */{ 0, -1 },      /* end */
    }; //MotorAndBoardList

    static const int MotorCount = 0x40;

    static const struct SysMotionTable_
    {
        unsigned short id;
        unsigned char motion_id;
    } SysMotionTable[] =
    {
        { 0x3A01, RC_TARJ_SHAKE_HANDS },
        { 0x3A02, RC_TARJ_WAVE },
        { 0x3A03, RC_TARJ_CHEER },
//      { 0x3A04, RC_TARJ_MOVE },
//      { 0x3A05, RC_TARJ_TURN },
        { 0x3A06, RC_TARJ_RUN },
        { 0x3A07, RC_TRAJ_CLAP },
        { 0x3A08, RC_TRAJ_AKIMBO },
        { 0x3A09, RC_TRAJ_SALUTE },
        { 0x3A0A, RC_TRAJ_FOLDARM },
        { 0x3A0B, RC_TRAJ_FLYKISS },
        { 0x3A0C, RC_TRAJ_HIGHFIVE },
        { 0x3A0D, RC_TRAJ_HUG },
        { 0x3A0E, RC_TRAJ_KISS },
        { 0x3A0F, RC_TRAJ_YES },
        { 0x3A10, RC_TRAJ_NO },
        { 0x3A11, RC_TRAJ_YE },
        { 0x3A12, RC_TRAJ_THANK1 },
        { 0x3A13, RC_TRAJ_THANK2 },
        { 0x3A14, RC_TRAJ_THANK3 },
        { 0x3A15, RC_TRAJ_LAUGH },
        { 0x3A16, RC_TRAJ_STRETCH },
        { 0x3A17, RC_TRAJ_LISTEN },
        { 0x3A18, RC_TRAJ_LOOK1 },
        { 0x3A19, RC_TRAJ_LOOK2 },
        { 0x3A20, RC_TRAJ_WORRY },
        { 0x3A21, RC_TRAJ_SHY },
        { 0x3A22, RC_TRAJ_TELL },
        { 0x3A23, RC_TRAJ_ME },
        { 0x3A24, RC_TRAJ_WE },
        { 0x3A25, RC_TRAJ_HANDSBACK },
        { 0x3A26, RC_TRAJ_RIGHT_HAND_MOTION1 },
        { 0x3A27, RC_TRAJ_RIGHT_HAND_MOTION2 },
        { 0x3A28, RC_TRAJ_RIGHT_HAND_MOTION3 },
        { 0x3A29, RC_TRAJ_RIGHT_HAND_MOTION4 },
        { 0x3A30, RC_TRAJ_RIGHT_HAND_MOTION5 },
        { 0x3A31, RC_TRAJ_ROSHAMBO_ROCK },
        { 0x3A32, RC_TRAJ_ROSHAMBO_SCISSOR },
        { 0x3A33, RC_TRAJ_ROSHAMBO_PAPER },
        { 0x3A34, RC_TRAJ_OFFSET_HORIZONTALLY },
        { 0x3A35, RC_TRAJ_WAKE },
        { 0x3A36, RC_TRAJ_FRIGHTENED },
        { 0x3A37, RC_TRAJ_DONT_TICKLE_RIGHT },
        { 0x3A38, RC_TRAJ_DONT_TICKLE_LEFT },
        { 0x3A39, RC_TRAJ_TWO_HANDS_PUSH_FORWARD },
        { 0x3A3A, RC_TRAJ_INCOMING_CALL },
        { 0x3A3B, RC_TRAJ_MJ_DANCE_POSE },
        { 0x3A3C, RC_TRAJ_BALLET_DANCE_POSE },
        { 0x3A3D, RC_TRAJ_TAKE_PHOTO },
        { 0x3A3E, RC_TRAJ_UPGRADE },
        { 0x3A3F, RC_TRAJ_TURN_BOOK },
        { 0x3A40, RC_TRAJ_STOP_TALKING },
        { 0x3A41, RC_TRAJ_DONT_TOUCH_ME },
        { 0x3A42, RC_TRAJ_OH_YEAH },
        { 0x3A43, RC_TRAJ_READY_TO_FOLLOW },
        { 0x3A44, RC_TRAJ_FOLLOWING },
        { 0x3A45, RC_TRAJ_WIPE_PERSPIRATION },
        { 0x3A46, RC_TRAJ_RAIN },
        { 0x3A47, RC_TRAJ_SNOW },
        { 0x3A48, RC_TRAJ_SELF_PROTECTION },
        { 0x3A49, RC_TRAJ_NARRATE },
        { 0x3A4A, RC_TRAJ_IDLE},
        { 0x3A4B, RC_TRAJ_CHAT_1},
        { 0x3A4C, RC_TRAJ_CHAT_2},
        { 0x3A4D, RC_TRAJ_CHAT_3},
        { 0x3A4E, RC_TRAJ_CHAT_4},
        { 0x3A4F, RC_TRAJ_PLEASE},
//      { 0x3A37, RC_TRAJ_RESET },
//      { 0x3A38, RC_TRAJ_STOP },
        { 0x3C01, RC_PLAN_MAZE },
        { 0x3C02, RC_PLAN_OBS_AVOID_SWITCH },
        { 0, 0 },
    };
    
    static const struct EmojiTable_
    {
        unsigned char id;
        unsigned char emoji_id;
    } EmojiTable[] =
    {
        { 0x01, RF_EXPRESSION_CLEAR_CMD },
        { 0x02, RF_EXPRESSION_SMILE_CMD },
        { 0x03, RF_EXPRESSION_SAD_CMD },
        { 0x04, RF_EXPRESSION_LAUGHT_CMD },
        { 0x05, RF_EXPRESSION_SURPRISE_CMD },
        { 0x06, RF_EXPRESSION_CRY_CMD },
        { 0x07, RF_EXPRESSION_DOUBT_CMD },
        { 0x08, RF_EXPRESSION_SHH_CMD },
        { 0x09, RF_EXPRESSION_SHY_CMD },
        { 0x0A, RF_EXPRESSION_COVER_SMILE_CMD },
        { 0x0B, RF_EXPRESSION_GRIMACE_CMD },
        { 0x0C, RF_EXPRESSION_NAUGHTY_CMD },
        { 0x0D, RF_EXPRESSION_HEARTED_CMD },
        { 0x0E, RF_EXPRESSION_ANGRY_CMD },
        { 0x0F, RF_EXPRESSION_THINKING_CMD },
        { 0x10, RF_EXPRESSION_POWER_ON },
        { 0x11, RF_EXPRESSION_POWER_OFF },
        { 0x12, RF_EXPRESSION_WAKE_UP },
        { 0x13, RF_EXPRESSION_SLEEP },
        { 0x14, RF_EXPRESSION_TALK },
        { 0x15, RF_EXPRESSION_LISTEN },
        { 0x16, RF_EXPRESSION_DEFAULT },
        { 0x17, RF_EXPRESSION_BLINK  },
        { 0x18, RF_EXPRESSION_EYECLOSE },
        { 0x19, RF_EXPRESSION_EYEOPEN },
        { 0x1A, RF_EXPRESSION_FROWN  },
        { 0x1B, RF_EXPRESSION_EYEBINDONE },
        { 0x1C, RF_EXPRESSION_INDIFFERENT },
        { 0, 0 },
    };

    static const struct BodyParts_
    {
        unsigned char id;
        unsigned char parts_;
    } BodyPartTable[] =
    {
        { 0xFF, ROBOT_RESET_ALL/*ROBOT_STOP_ALL*/},
        { 0xFE, ROBOT_RESET_ARMS/*ROBOT_STOP_ARMS*/},
        { 0xFD, ROBOT_RESET_RIGHT_ARM /*ROBOT_STOP_RIGHT_ARM*/},
        { 0xFC, ROBOT_RESET_LEFT_ARM/*ROBOT_STOP_LEFT_ARM*/},
        { 0xFB, ROBOT_STOP_WHEELS},
        { 0xFA, ROBOT_RESET_NECK},
        { 0xF9, ROBOT_STOP_EMOJI/*ROBOT_RESET_EMOJI*/},
        { 0xF8, ROBOT_RESET_WAIST},
        { 0xF1, ROBOT_STOP_ACTIONS},
        { 0x00, 0},
    };
    static const struct RobotSensorTypeHandleMap_{
        uint8_t type;   //rc define
        uint8_t id; //android define
    }RobotSensorMap[]=
    {
        {RS_FRONT_LEFT_45_LOWER_IR, RSENSOR_ID_IR_FRONT_LEFT_45_LOWER},
        {RS_FRONT_LEFT_LOWER_IR,    RSENSOR_ID_IR_FRONT_LEFT_LOWER},
        {RS_FRONT_RIGHT_LOWER_IR,   RSENSOR_ID_IR_FRONT_RIGHT_LOWER},
        {RS_FRONT_RIGHT_45_LOWER_IR,RSENSOR_ID_IR_FRONT_RIGHT_45_LOWER},
        {RS_LEFT_CENTER_LOWER_IR,   RSENSOR_ID_IR_LEFT_CENTER_LOWER},
        {RS_RIGHT_CENTER_LOWER_IR,  RSENSOR_ID_IR_RIGHT_CENTER_LOWER},
        {RS_BACK_CENTER_LOWER_IR,   RSENSOR_ID_IR_BACK_CENTER_LOWER},
        {RS_FRONT_BELLY_US,         RSENSOR_ID_US_FRONT_BELLY},
        {RS_BACK_WAIST_US,          RSENSOR_ID_US_BACK_WAIST},
        {RS_ACC,                    RSENSOR_ID_ACCELEROMETER},
        {RS_MAG,                    RSENSOR_ID_MAGNETIC_FIELD},
        {RS_GYRO,                   RSENSOR_ID_GYROSCOPE},
        {RS_LEFT_ANKLE_OUTSIDE_US,  RSENSOR_ID_US_LEFT_ANKLE_OUTSIDE},
        {RS_RIGHT_ANKLE_OUTSIDE_US, RSENSOR_ID_US_RIGHT_ANKLE_OUTSIDE},
        {RS_CHASSIS_US,             RSENSOR_ID_US_CHASSIS},
        {RS_COLLISION,              RSENSOR_ID_COLLISION},
    };

    static const struct RmSysCtrlCmdMap_{
        unsigned char cmd;
        unsigned char reply;
    } RmSysCtrlCmdMap[]=
    {
        {COMMON_SYSCTRL_HALT_CMD,           COMMON_SYSCTRL_HALT_CMD_REP},
        {COMMON_SYSCTRL_WAKE_CMD,           COMMON_SYSCTRL_WAKE_CMD_REP},
        {COMMON_SYSCTRL_SECURITY_CMD,       COMMON_SYSCTRL_SECURITY_CMD_REP},
        {COMMON_SYSCTRL_EXIT_SECURITY_CMD,  COMMON_SYSCTRL_EXIT_SECURITY_CMD_REP},
        {RM_SYSCTRL_BATTERY_QUERY,          RM_SYSCTRL_BATTERY_QUERY_REP},
        {COMMON_SYSCTRL_SLEEP_CMD,          COMMON_SYSCTRL_SLEEP_CMD_REP},
        {COMMON_SYSCTRL_DEBUG_CMD,          COMMON_SYSCTRL_DEBUG_CMD_REP},
        {COMMON_SYSCTRL_MODE_CMD,           COMMON_SYSCTRL_MODE_CMD_REP},
        {RM_SYSCTRL_RESTORE_FACTORY_CMD,    RM_SYSCTRL_RESTORE_FACTORY_CMD_REP},
        {COMMON_SYSCTRL_LOW_POWER_NOTI,    COMMON_SYSCTRL_LOW_POWER_NOTI},
        {COMMON_SYSCTRL_NOT_LOW_POWER_NOTI,    COMMON_SYSCTRL_NOT_LOW_POWER_NOTI},
    }, RcSafeStatusCmdMap[]=
    {
        {RM_QUERY_MOTION_SAFE_STATUS, RM_QUERY_MOTION_SAFE_STATUS_RESP},
        {RM_ENABLE_MOTION_SAFE_DETECT,RM_ENABLE_MOTION_SAFE_DETECT_RESP},
        {/*RM_ENABLE_SENSER_OBSTACLE*/0x03,   RM_ENABLE_SENSER_OBSTACLE_RESP},
        {/*RM_QUERY_SENSER_OBSTACLE_FLAG*/0x04,RM_QUERY_SENSER_OBSTACLE_FLAG_RESP},
        {RC_LOW_POWER_CONSUMPTION,  RC_LOW_POWER_CONSUMPTION_RESP},
    };

    static const struct SettingCmdMap_{
        unsigned char appCmd;
        unsigned char hwCmd;
    } SettingCmdMap[]= 
    {
        { 0x01, RB_SET_MOTION_ONOFF},
        { 0x02, RB_QUERY_MOTION_ONOFF},
        { 0x03, RM_ENABLE_SENSER_OBSTACLE}, //sensor on/off
        { 0x04, RM_QUERY_SENSER_OBSTACLE_FLAG},
        { 0x05, RB_QUERY_MOTION_STATE_QUERY},
        { 0x06, ROBOT_SENSOR_CALIBRATION}, // RS_GYRO
        { 0x00, 0x00},
    };
    
    static const unsigned char AllMotor[] = 
    {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x13, 0x14, 0x15, 0x11, 0x12, 0x00
    };
	static const unsigned char AllArmMotor[] = 
    {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x00
    };
    static const unsigned char RightArmMotor[] = 
    {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x00
    };
    static const unsigned char LeftArmMotor[] = 
    {
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x00
    };
	 static const unsigned char NeckMotor[] = 
    {
        0x14, 0x15, 0x00
    };
	 static const unsigned char WaistMotor[] = 
    {
        0x13, 0x00
    };
    static const unsigned char WheelMotor[] = 
    {
        0x18, 0x19, 0x00
    };
    static const unsigned char AllSensor[] = 
    {
        0x01, 0x02, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x00
    };
    static const struct SettingIDMap_{
        const unsigned char id;
        const unsigned char *idset;
    } SettingIDMap[] = 
    {
        { 0xFF, AllMotor     },
		{ 0xFE, AllArmMotor  },
        { 0xFD, RightArmMotor},
        { 0xFC, LeftArmMotor },
        { 0xFB, WheelMotor   },
        { 0xFA, NeckMotor    },
        { 0xF8, WaistMotor   },
        { 0xF0, AllSensor    },
        { 0x00, 0x00         },
    };
   
    int getSettingCommand(int cmd)
    {
        int i = 0;
        while(SettingCmdMap[i].appCmd != 0x00 && SettingCmdMap[i].appCmd != cmd) i++;
        return SettingCmdMap[i].hwCmd;
    }
    int getSettingIDList(int ids, unsigned char * idList, int len)
    {
        int i = 0, j = 0;
        
        while(SettingIDMap[i].id != 0x00 && SettingIDMap[i].id != ids) i++;

        if (SettingIDMap[i].id == 0x00)
        {
            return 0;
        }
        for (j = 0; SettingIDMap[i].idset[j] != 0x00; j++)
        {
            if (j >= len)
            {
                break;
            }
            idList[j] = SettingIDMap[i].idset[j];
        }
		 if(SettingIDMap[i].id == 0xF0 && j < len - 1)
		 {
			idList[++j] = 0x00; 
		 }
        return j;
    }
    int getRobotBodyPart(int id)
    {
        int i = 0;
        while (BodyPartTable[i].id != 0x00 && BodyPartTable[i].id != id) i++;

        return BodyPartTable[i].parts_;
    }
    int getMotorAndBoardID(int joint_id, int & motor_id, int & board_id)
    {
        board_id = -1;
        motor_id = 0;

        if (joint_id < 1 || joint_id > MotorCount)
        {
            return -1;
        }
        motor_id = MotorAndBoardList[joint_id].motor_id;
        board_id = MotorAndBoardList[joint_id].board_id;
        return 0;
    }

    int getSystemMotion(int id)
    {
        int i = 0;
        while (SysMotionTable[i].id != 0 && SysMotionTable[i].id != id) i++;

        return SysMotionTable[i].motion_id;
    }

    int getEmoji(int id)
    {
        int i = 0;
        while (EmojiTable[i].id != 0 && EmojiTable[i].id != id) i++;

        return EmojiTable[i].emoji_id;
    }
    int getSensorId(uint8_t id)
    {
        size_t len = sizeof(RobotSensorMap)/sizeof(struct RobotSensorTypeHandleMap_);
        for (size_t i = 0; i < len; i++) {
            if (id == RobotSensorMap[i].id)
                return RobotSensorMap[i].type;
        }

        return -1;
    }
    int getSysCtrlCmd(int respid)
    {
        for(unsigned int i = 0; i < sizeof(RmSysCtrlCmdMap)/sizeof(struct RmSysCtrlCmdMap_); i++){
            if(respid == RmSysCtrlCmdMap[i].reply){
                return RmSysCtrlCmdMap[i].cmd;
            }
        }

        for(unsigned int i = 0; i < sizeof(RcSafeStatusCmdMap)/sizeof(struct RmSysCtrlCmdMap_); i++){
            if(respid == RcSafeStatusCmdMap[i].reply){
                return RcSafeStatusCmdMap[i].cmd;
            }
        }
        return 0;
    }

};//namespace  DeviceTable
