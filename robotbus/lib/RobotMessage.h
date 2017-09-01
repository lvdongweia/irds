#ifndef  ROBOTMESSAGE_H_

#define ROBOTMESSAGE_H_

#include "Common.h"
#include "DataObject.h"

namespace RobotNetwork
{
    class DLLEXPORT RobotMessage
    {
    public:
        int what;
        int value;
        int extend;
        DataObject * obj;
    public:
        RobotMessage();
        ~RobotMessage();
        RobotMessage(int what, int value);
        RobotMessage(int what, int value, const char * str);
        RobotMessage(int what, unsigned char * bytes, int offset, int count);
        void Clear();
    };
}

#endif // ! ROBOTMESSAGE_H_

