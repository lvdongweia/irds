#include <stdio.h>

#include "FileObject.h"

using namespace RobotNetwork;

FileObject::~FileObject()
{
    Clear();
}

FileObject::FileObject()
{
    init("", OpenMode::None, 0);
}

FileObject::FileObject(char* name)
{
    init(name, OpenMode::Read, 0);
}

FileObject::FileObject(char* name, int mode)
{
    init(name, mode, 0);
}

FileObject::FileObject(char* name, int mode, int maxSize)
{
    init(name, mode, maxSize);
}

void FileObject::init(char* name_, int mode, int maxSize)
{
    if(name_ != 0)
    {
        int nameSize = strlen(name_);
        name = new char[nameSize + 1];
        memcpy(name, name_, nameSize);
        this->mode = mode;
        this->fileSize = maxSize;
    }
}
void FileObject::Clear()
{
    if(fr != 0)
    {
        fclose(fr);
    }
    fr = NULL;
    
    if(name != 0)
    {
        delete name;
    }
    mode = OpenMode::None;
    objStatus = ObjectStatus::INIT;
    fileSize= 0;
    position = 0;
}

char* FileObject::getName()
{
    return name;
}

int FileObject::getMode()
{
    return mode;
}

int FileObject::getType()
{
    return ObjectType::FILE;
}
int FileObject::getSize()
{
    return fileSize;
}
void FileObject::setSize(int size)
{
    fileSize = size;
}
int FileObject::getPosition()
{
    return position;
}
bool FileObject::isEnd()
{
    if (mode != OpenMode::None && position >= fileSize)
    {
        return true;
    }
    return false;
}
void FileObject::Reset()
{
    return ;
}

int FileObject::open(int mode)
{
    return 0;
}
int FileObject::read(DataBuffer * buffer)
{
    if(mode == OpenMode::Write)
    {
        return -21;
    }
    if (fr == NULL && position == 0)
    {
        if (open(OpenMode::Read) <= 0)
        {
            return -22;
        }
    }
    if ((fileSize - position) <= 0 || buffer == 0)
    {
        return 0;
    }
     if (buffer->count == 0)
    {
        buffer->count = buffer->count - buffer->offset;
    }
     //TODO: change char to UTF-8 byte
    return 0;
}

int FileObject::write(DataBuffer * buffer)
{
    return 0;
}


