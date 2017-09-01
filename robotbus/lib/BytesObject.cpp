#include "BytesObject.h"

using namespace RobotNetwork;

BytesObject::~BytesObject()
{
    Clear();
}

byte * BytesObject::getData()
{
    return data;
}
void BytesObject::setData(byte * bytes, int size)
{
    Clear();
    data = bytes;
    dataSize = size;
    position = 0;
    selfOwner = false;
}
int BytesObject::getType()
{ 
    return ObjectType::BYTES; 
}

int BytesObject::getSize()
{
    return dataSize;
}

void BytesObject::setSize(int size)
{
    Clear();

    if (data == 0 || dataSize != size)
    {
        data = new byte[size];
    }
}
int BytesObject::getPosition()
{
    return position;
}

bool BytesObject::isEnd()
{
    if (data == 0 || dataSize <= 0)
    {
        return true;
    }
    if (position >= dataSize)
    {
        return true;
    }
    return false;
}

BytesObject::BytesObject() : data(0)
{
    Clear();
}
BytesObject::BytesObject(int size) : data(0)
{
    Clear();
    dataSize = size;
    data = new byte[size];
}
BytesObject::BytesObject(byte * bytes, int offset, int count) : data(0)
{
    Clear();
    if (bytes != 0)
    {
        dataSize = count;
        data = new byte[count];
        memcpy(data, bytes + offset, count);
    }
}
void BytesObject::Clear()
{
    if (data != 0 && selfOwner)
    {
        delete data;
    }
    data = 0;
    position = 0;
    dataSize = 0;
    selfOwner = true;
    mode = OpenMode::None;
    status = ObjectStatus::INIT;
}

void BytesObject::Reset()
{
    position = 0;
    mode = OpenMode::None;
    status = ObjectStatus::INIT;
}
int BytesObject::read(DataBuffer * buffer)
{
    if (data == 0 || dataSize <= 0 || mode == OpenMode::Write)
    {
        return -1;
    }
    if (position >= dataSize)
    {
        return 0;
    }
    if (mode == OpenMode::None)
    {
        position = 0;
        mode = OpenMode::Read;
        status = ObjectStatus::Ready;
    }
    int size = dataSize - position;
    if (size > (buffer->count - buffer->offset))
    {
        size = buffer->count - buffer->offset;
    }
    memcpy(buffer->bytes + buffer->offset, data + position, size);
    buffer->count = size;
    buffer->sizeOrPosition = (status == ObjectStatus::Ready) ? dataSize : position;
    buffer->type = (status == ObjectStatus::Ready) ? 0x01 : 0x00;
    position += size;
    status = ObjectStatus::IO;

    return size;
}
int BytesObject::write(DataBuffer * buffer)
{
    if (buffer->bytes == 0 || buffer->count == 0 || mode == OpenMode::Read)
    {
        return -1;
    }
    if (buffer->type == 0x01)
    {
        position = 0;
        mode = OpenMode::Write;
        status = ObjectStatus::IO;
        data = new byte[buffer->sizeOrPosition];
        dataSize = buffer->sizeOrPosition;
        buffer->sizeOrPosition = 0;
    }
    if (mode != OpenMode::Write)
    {
        return -2;
    }
    if (data == 0 || dataSize < buffer->count)
    {
        return -3;
    }
    if (position >= dataSize)
    {
        return 0;
    }
    if (buffer->sizeOrPosition < position)
    {
        return 0xE0;
    }
    else if (buffer->sizeOrPosition > position)
    {
        return 0xE1;
    }
    memcpy(data + position, buffer->bytes + buffer->offset, buffer->count);
    position += buffer->count;
    return 1;
}
