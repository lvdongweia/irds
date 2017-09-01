#include "MessageSerializable.h"
#include "StringObject.h"

using namespace RobotNetwork;

int MessageSerializable::maxPackageSize = 10240;
int MessageSerializable::headerSize = 36;

MessageSerializable::~MessageSerializable()
{
}
MessageSerializable::MessageSerializable()
{
    init(0);
}

MessageSerializable::MessageSerializable(RobotMessage * message)
{
    init(message);
}

void MessageSerializable::init(RobotMessage * message)
{
    this->message = 0;
    payload = 0;
    Clear();
    this->message = message;
}
int MessageSerializable::getSize()
{
    if (message == 0)
    {
        return 0;
    }
    return (headerSize + (message->obj == 0 ? 0 : message->obj->getSize()));
}
int MessageSerializable::Serialize()
{
    int i = 0;
    packageSize = getSize();
    if (packageSize == 0)
    {
        return -1;
    }
    if (getSize() > maxPackageSize)
    {
        packageSize = maxPackageSize;
    }
    if (payload == 0 || capacity < packageSize)
    {
        payload = new byte[packageSize];
        capacity = packageSize;
    }
    //0.
    i = 0;
    payload[i++] = 0x4D;
    payload[i++] = 0x53;
    payload[i++] = 0x60;
    payload[i++] = (byte)(needReply ? 0x01 : 0x00);
    //4.
    i = 4;
    payload[i++] = (byte)(uid & 0xFF);
    payload[i++] = (byte)((uid & 0xFF00) >> 8);
    payload[i++] = (byte)((uid & 0xFF0000) >> 16);
    payload[i++] = (byte)((uid & 0xFF000000) >> 24);
    //8.
    i = 8;
    payload[i++] = (byte)(peer_uid & 0xFF);
    payload[i++] = (byte)((peer_uid & 0xFF00) >> 8);
    payload[i++] = (byte)((peer_uid & 0xFF0000) >> 16);
    payload[i++] = (byte)((peer_uid & 0xFF000000) >> 24);
    //12.
    i = 12;
    payload[i++] = (byte)(message->what & 0xFF);
    payload[i++] = (byte)((message->what & 0xFF00) >> 8);
    payload[i++] = (byte)((message->what & 0xFF0000) >> 16);
    payload[i++] = (byte)((message->what & 0xFF000000) >> 24);

    //16.
    i = 16;
    payload[i++] = (byte)(message->value & 0xFF);
    payload[i++] = (byte)((message->value & 0xFF00) >> 8);
    payload[i++] = (byte)((message->value & 0xFF0000) >> 16);
    payload[i++] = (byte)((message->value & 0xFF000000) >> 24);

    //20.
    i = 20;
    payload[i++] = (byte)(message->extend & 0xFF);
    payload[i++] = (byte)((message->extend & 0xFF00) >> 8);
    payload[i++] = (byte)((message->extend & 0xFF0000) >> 16);
    payload[i++] = (byte)((message->extend & 0xFF000000) >> 24);

    if (message->obj != 0)
    {
        DataObject::DataBuffer * buffer = new DataObject::DataBuffer(payload, headerSize, packageSize);
        int result = message->obj->read(buffer);

        if (result <= 0)
        {
            return result;
        }
        //24.
        i = 24;
        payload[i++] = (byte)message->obj->getType();
        payload[i++] = (byte)buffer->type;
        payload[i++] = 0;
        payload[i++] = 0;
        //28
        i = 28;
        payload[i++] = 0;
        payload[i++] = 0;
        payload[i++] = (byte)(buffer->count & 0xFF);
        payload[i++] = (byte)((buffer->count & 0xFF00) >> 8);

        //32
        i = 32;
        payload[i++] = (byte)(buffer->sizeOrPosition & 0xFF);
        payload[i++] = (byte)((buffer->sizeOrPosition & 0xFF00) >> 8);
        payload[i++] = (byte)((buffer->sizeOrPosition & 0xFF0000) >> 16);
        payload[i++] = (byte)((buffer->sizeOrPosition & 0xFF000000) >> 24);
        packageSize = buffer->count + i;
        delete buffer;
    }
    else
    {
        //24.
        i = 24;
        payload[i++] = 0;
        payload[i++] = 0;
        payload[i++] = 0;
        payload[i++] = 0;

        //28.
        i = 28;
        payload[i++] = 0;
        payload[i++] = 0;
        payload[i++] = 0;
        payload[i++] = 0;

        //32.
        i = 32;
        payload[i++] = 0;
        payload[i++] = 0;
        payload[i++] = 0;
        payload[i++] = 0;

        packageSize = i;
    }
    return packageSize;
}
int MessageSerializable::DeserializeHeader(byte * bytes, int offset, int count)
{
    int i = offset;
    if (bytes == 0 || count == 0)
    {
        return -21;
    }
    if (CheckType(bytes, offset, count) == false)
    {
        return -22;
    }
    needReply = (bytes[i + 3] == 0x01);

    if (message == 0)
    {
        message = new RobotMessage();
    }
    i = i + 4;
    uid = ((int)bytes[i++] & 0xff) | (((int)bytes[i++] & 0xff) << 8) | (((int)bytes[i++] & 0xff) << 16) | (((int)bytes[i++] & 0xff) << 24);
    peer_uid = ((int)bytes[i++] & 0xff) | (((int)bytes[i++] & 0xff) << 8) | (((int)bytes[i++] & 0xff) << 16) | (((int)bytes[i++] & 0xff) << 24);
    message->what = ((int)bytes[i++] & 0xff) | (((int)bytes[i++] & 0xff) << 8) | (((int)bytes[i++] & 0xff) << 16) | (((int)bytes[i++] & 0xff) << 24);
    message->value = ((int)bytes[i++] & 0xff) | (((int)bytes[i++] & 0xff) << 8) | (((int)bytes[i++] & 0xff) << 16) | (((int)bytes[i++] & 0xff) << 24);
    message->extend = ((int)bytes[i++] & 0xff) | (((int)bytes[i++] & 0xff) << 8) | (((int)bytes[i++] & 0xff) << 16) | (((int)bytes[i++] & 0xff) << 24);

    i = offset + 24;
    objType = bytes[i];
    i = offset + 25;
    blockType = bytes[i];
    i = offset + 30;
    packageSize = ((int)bytes[i++] & 0xff) | (((int)bytes[i++] & 0xff) << 8);
    i = offset + 32;
    sizeOrPosition = ((int)bytes[i++] & 0xff) | (((int)bytes[i++] & 0xff) << 8) | (((int)bytes[i++] & 0xff) << 16) | (((int)bytes[i++] & 0xff) << 24);

    return sizeOrPosition;
}
int MessageSerializable::DeserializeObject(byte * bytes, int offset, int count)
{
    if (objType == DataObject::ObjectType::None || packageSize == 0)
    {
        return 0;
    }
    DataObject::DataBuffer * buffer = new DataObject::DataBuffer(bytes, offset + headerSize, packageSize);
    buffer->sizeOrPosition = sizeOrPosition;
    buffer->type = blockType;
    if (message->obj == 0)
    {
        switch (objType)
        {
        case DataObject::ObjectType::FILE:
            //TODO
            break;
        case DataObject::ObjectType::STRING:
            message->obj = new StringObject();
            break;
        case DataObject::ObjectType::BYTES:
            message->obj = new BytesObject();
            break;
        default:
            return -23;
        }
    }
    return message->obj->write(buffer);
}
int MessageSerializable::Deserialize(byte * bytes, int offset, int count)
{
    int result = DeserializeHeader(bytes, offset, count);
    if (result < 0)
    {
        return result;
    }
    return DeserializeObject(bytes, offset, count);
}
void MessageSerializable::Clear()
{
    if (message != 0)
    {
        delete message;
    }
    if (payload != 0)
    {
        delete payload;
    }
    uid = 0;
    peer_uid = 0;
    message = 0;
    payload = 0;
    packageSize = 0;
    capacity = 0;
    needReply = false;
    objType = 0;
    sizeOrPosition = 0;
    blockType = 0;
}

bool MessageSerializable::CheckType(byte * bytes, int offset, int count)
{
    if (bytes == 0 || count == 0)
    {
        return false;
    }
    if (count < headerSize || bytes[0] != 0x4D || bytes[1] != 0x53 || bytes[2] != 0x60)
    {
        return false;
    }
    return true;
}
int MessageSerializable::TryParseUID(byte * bytes, int offset, int count)
{
    if (CheckType(bytes, offset, count) == false)
    {
        return 0;
    }
    int i = 4 + offset;
    return ((int)bytes[i++] & 0xff) | (((int)bytes[i++] & 0xff) << 8) | (((int)bytes[i++] & 0xff) << 16) | (((int)bytes[i++] & 0xff) << 24);
}
int MessageSerializable::TryParsePayloadSize(byte * bytes, int offset, int count)
{
    if (CheckType(bytes, offset, count) == false)
    {
        return 0;
    }
    int i = 30 + offset;
    return ((int)bytes[i++] & 0xff) | (((int)bytes[i++] & 0xff) << 8);
}
int MessageSerializable::TryParsePosition(byte * bytes, int offset, int count)
{
    if (CheckType(bytes, offset, count) == false)
    {
        return 0;
    }
    int i = 32 + offset;
    return ((int)bytes[i++] & 0xff) | (((int)bytes[i++] & 0xff) << 8) | (((int)bytes[i++] & 0xff) << 16) | (((int)bytes[i++] & 0xff) << 24);
}
