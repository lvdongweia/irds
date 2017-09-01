#ifndef STRINGOBJECT_H_
#define STRINGOBJECT_H_

#include "BytesObject.h"

namespace RobotNetwork
{
    class DLLEXPORT StringObject : public BytesObject
    {
    public:
        ~StringObject();
        StringObject();
        StringObject(const char * s);
        const char * ToString();
        int getType();
    };
}

#endif 

