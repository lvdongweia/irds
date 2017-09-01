#include "RobotMessage.h"
#include "StringObject.h"

using namespace RobotNetwork;

RobotMessage::RobotMessage()
{
    Clear();
}
RobotMessage::~RobotMessage()
{
    if (obj != 0)
    {
        delete obj;
    }
    Clear();
}
RobotMessage::RobotMessage(int what, int value)
{
    Clear();
    this->what = what;
    this->value = value;
}
RobotMessage::RobotMessage(int what, int value, const char * str)
{
    Clear();
    this->what = what;
    this->value = value;
    if (str != 0 && strlen(str) > 0)
    {
        obj = new StringObject(str);
    }
}
RobotMessage::RobotMessage(int what, byte * bytes, int offset, int count)
{
    Clear();
    this->what = what;
    if (bytes != 0)
    {
        obj = new BytesObject(bytes, offset, count);
    }
}
void RobotMessage::Clear()
{
    what = 0;
    value = 0;
    extend = 0;
    obj = 0;
}
