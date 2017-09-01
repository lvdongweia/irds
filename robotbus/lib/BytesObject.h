#ifndef BYTESOBJECT_H_
#define BYTESOBJECT_H_

#include "DataObject.h"

namespace RobotNetwork
{
    class DLLEXPORT BytesObject : public DataObject
    {
    public:
        byte * data;
        int dataSize;
    private:
        int mode;
        int status;
        int position;
        bool selfOwner;
    public:
        virtual ~BytesObject();
        BytesObject();
        BytesObject(int size);
        BytesObject(byte * bytes, int offset, int size);

        void Clear();
        int read(DataBuffer * buffer);
        int write(DataBuffer * buffer);
        byte * getData();
        void setData(byte * bytes, int size);
        int getType();
        int getSize();
        void setSize(int size);
        int getPosition();
        bool isEnd();
        void Reset();
    };
}

#endif

