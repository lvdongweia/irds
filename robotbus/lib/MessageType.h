#ifndef MESSAGE_TYPE_H_

#define MESSAGE_TYPE_H_

namespace RobotNetwork
{
    class MessageType
    {
        class PLAYERSID
        {
            enum ID
            {
                PREPARE = 0x10101,
                RUN = 0x1002,
                PAUSE = 0x1003,
                RESUME = 0x1004,
                STOP = 0x1005,
            };
        };
        class FILESID
        {
            enum ID
            {
                GET_FILELIST = 0x1401,
                GET_FILETHUMB = 0x1402,
                POST_FILE = 0x1403,
                REMOVE_FILE = 0x1404,
                RUN_FILE = 0x1405,
            };
        };
        class MOTIONSID
        {
            enum ID
            {
                DO_ACTION = 0x1501,
                DO_EMOJI = 0x1502,
                DO_WALK = 0x1503,
                GO_AHEAD = 0x1504,
                DO_TURN = 0x1505,
                NODDING = 0x1506,
                SHAKING = 0x1507,
                RUN_MOTOR = 0x1508,
                STOP = 0x1509,
            };
        };
        class CHATSID
        {
            enum ID
            {
                USER_SAY = 0x1301,
                ROBOT_SAY = 0x1302,
            };
        };
    };
}
#endif // !MESSAGE_TYPE_H_
