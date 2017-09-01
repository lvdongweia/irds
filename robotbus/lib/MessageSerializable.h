#ifndef MESSAGESERIALIZABLE_H_
#define MESSAGESERIALIZABLE_H_

#include "RobotMessage.h"

namespace RobotNetwork
{

    class DLLEXPORT MessageSerializable
    {
    public:
        static int maxPackageSize;
        static int headerSize;

        int uid;
        int peer_uid;
        bool needReply;
        RobotMessage * message;
        byte * payload;
        int packageSize;
    private:
        int capacity;
        int objType;
        int sizeOrPosition;
        int blockType;
    private:
        void init(RobotMessage * message);
    public:
        MessageSerializable();
        MessageSerializable(RobotMessage * message);
        virtual ~MessageSerializable();
        int getSize();
        int Serialize();
        int DeserializeHeader(byte * bytes, int offset, int count);
        int DeserializeObject(byte * bytes, int offset, int count);
        int Deserialize(byte * bytes, int offset, int count);
        virtual void Clear();
        static bool CheckType(byte * bytes, int offset, int count);
        static int TryParseUID(byte * bytes, int offset, int count);
        static int TryParsePayloadSize(byte * bytes, int offset, int count);
        static int TryParsePosition(byte * bytes, int offset, int count);
    };

}
#endif //MESSAGESERIALIZABLE_H_

