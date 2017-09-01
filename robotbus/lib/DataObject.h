#ifndef DATAOBJECT_H_
#define DATAOBJECT_H_

#include "Common.h"

namespace RobotNetwork
{

    class DLLEXPORT DataObject
    {
    public:
        class OpenMode
        {
        public:
            enum 
            {
                None = 0,
                Read = 1,
                Write = 2,
            };
        };
        class ObjectType
        {
        public:
            enum
            {
                None = 0,
                FILE = 1,
                BYTES = 2,
                STRING = 3,
            };
        };
        class ObjectStatus
        {
        public:
            enum
            {
                INIT = 0,
                Ready = 1,
                IO = 2,
            };
        };
        class DataBuffer
        {
        public:
            byte * bytes;
            int offset;
            int count;
            int sizeOrPosition;
            int type;
        public:
            DataBuffer(byte * bytes, int offset, int count)
            {
                this->bytes = 0;
                clear();
                this->bytes = bytes;
                this->offset = offset;
                this->count = count;
            }
            ~DataBuffer()
            {
                clear();
            }
            void clear()
            {
                bytes = 0;
                offset = 0;
                count = 0;
                type = 0;
                sizeOrPosition = 0;
            }
        };
        virtual int getType() = 0;
        virtual int getSize() = 0;
        virtual void setSize(int size) = 0;
        virtual int getPosition() = 0;
        virtual bool isEnd() = 0;
        virtual void Clear() { };
        virtual void Reset() = 0;
        virtual int read(DataBuffer * buffer) { return 0; }
        virtual int write(DataBuffer * buffer) { return 0; }
    public:
        DataObject();
        virtual ~DataObject();
    };

}

#endif // !DATAOBJECT_H_
