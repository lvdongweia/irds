#include "StringObject.h"

using namespace RobotNetwork;

StringObject::~StringObject()
{
}
StringObject::StringObject()
{
    Clear();
}

StringObject::StringObject(const char * s) : BytesObject()
{
    Clear();

    if (s != 0 && strlen(s) > 0)
    {
        int count = strlen(s);
        dataSize = count + 1;
        data = new byte[dataSize];
        memcpy(data, s, count);
        data[count] = 0;
    }
}
const char * StringObject::ToString()
{
    if (data == 0 || dataSize <= 0)
    {
        return "";
    }
    return (const char *)data;
}
int StringObject::getType()
{
    return ObjectType::STRING;
}
