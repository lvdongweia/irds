#include <stdio.h>
#include <string.h>

#include <utils/List.h>
#include "ArmFile.h"
#include "RDSLog.h"

class BinaryReader
{
    public:
        BinaryReader()
        {
            fp = NULL;
            bytes = NULL;
            capacity = 0;
            fpos = 0;
            pStream = NULL;
            this->offset = 0;
            this->size = 0;
        }
        ~BinaryReader()
        {
            if (bytes != NULL)
            {
                delete bytes;
            }
            if (fp != NULL)
            {
                fclose(fp);
            }
            bytes = NULL;
            fp = NULL;
        }
    public:
        int OpenFile(const char * fileName, int offset, int size)
        {
            if (fileName == NULL || (fp = fopen(fileName, "rb")) == NULL)
            {
                return -1;
            }
            fseek(fp, 0, SEEK_END);
            int fSize = ftell(fp);
            if (size > 0 && (fSize - offset) < size)
            {
                fclose(fp);
                fp = NULL;
                return -2;
            }
            fseek(fp, offset, SEEK_SET);
            this->size = size <= 0 ? fSize : size;
            this->offset = offset;
            this->capacity = 255;
            this->bytes = new byte[capacity];

            return 0;

        }
        int OpenStream(const byte * stream, int size)
        {
            this->size = size;
            this->offset = 0;
            this->capacity = 255;
            this->bytes = new byte[255];
            this->pStream = stream;

            return 0;
        }
    public:
        void Close()
        {
            if (fp != NULL)
            {
                fclose(fp);
            }
            fp = NULL;
            if (bytes != NULL)
            {
                delete bytes;
            }
            fpos = 0;
            size = 0;
            bytes = NULL;
            pStream = NULL;
        }
        long CurrentPosition()
        {
            if (fp == NULL)
            {
                return fpos;
            }
            return ftell(fp);
        }
    public:
        byte * Read(int len)
        {
            if (len > capacity)
            {
                delete bytes;
                capacity = len;
                bytes = new byte[capacity];
            }

            if (fp == 0 && pStream != NULL)
            {
                int i = 0;
                for (i = 0; i < len && fpos < size; i++)
                {
                    bytes[i] = pStream[fpos];
                    fpos++;
                }
                if(i != len)
                {
                    return NULL;
                }
                return bytes;
            }
            if (fp == 0 || ftell(fp) >= (offset + size))
            {
                return NULL;
            }
            int num = fread(bytes, 1, len, fp);

            if (num != len)
            {
                return NULL;
            }
            return bytes;
        }
        byte * ReadCommand(int len)
        {
            byte * buffer = new byte[len];
            int num = 0;
            if (fp == 0 && pStream != NULL)
            {
                for (num = 0; num < len && fpos < size; num++)
                {
                    buffer[num] = pStream[fpos];
                    fpos++;
                }
            }
            else if(fp != 0)
            {
                num = fread(buffer, 1, len, fp);
            }
            if (num != len)
            {
                delete buffer;
                LOGE("###", "Load arc file read action error:%d %d", num, len);
                return NULL;
            }
            return buffer;

        }
        int Skip(int offset)
        {
            if (fp == 0 && pStream != NULL)
            {
                if ((fpos + offset) > size)
                {
                    return -1;
                }
                fpos += offset;
            }
            long i = fp == 0 ? -1 : ftell(fp);

            LOGE("###", "fp:%d current:%ld offset:%d size:%ld this.offset:%ld", (int)fp, i, offset, size, this->offset);
            if (fp == 0 || (ftell(fp) + offset) >= (this->offset + size))
            {
                LOGE("###", "arm file skip fail");
                return -1;
            }
            return fseek(fp, offset, SEEK_CUR);
        }
        int ReadUInt16()
        {
            if (Read(2) == NULL)
            {
                return -1;
            }
            short d = (short)(((int)bytes[0] & 0xff) | (((int)bytes[1] & 0xff) << 8));
            return d;
        }
        int ReadUInt32()
        {
            if (Read(4) == NULL)
            {
                return -1;
            }
            int d = ((int)bytes[0] & 0xff) | (((int)bytes[1] & 0xff) << 8) | (((int)bytes[2] & 0xff) << 16) | (((int)bytes[3] & 0xff) << 24);
            return d;
        }
        int ReadByte()
        {
            if (Read(1) == NULL)
            {
                return -1;
            }
            return bytes[0];
        }
    public:
        long size;

    private:
        FILE * fp;
        long offset;
        int capacity;
        int fpos;
        byte * bytes;
        const byte * pStream;
};

static byte startcode[] = { 0x50, 0x95, 0x28, 0x00 };
static byte framestart[] = { 0x46, 0x52, 0x41, 0x4d };

static bool ByteCompare(byte * bytes1, byte * bytes2, int len)
{
    if(bytes1 == NULL || bytes2 == NULL)
    {
        return false;
    }
    for (int i = 0; i < len; i++) {
        if (bytes1[i] != bytes2[i]) {
            return false;
        }
    }
    return true;
}

MotionFrames * ArmFile::LoadFromStream(const byte * bytes, int size)
{
    BinaryReader * reader = new BinaryReader();

    reader->OpenStream(bytes, size);

    MotionFrames * frames = ArmFile::LoadFrame(reader);

    delete reader;
    return frames;
}


MotionFrames * ArmFile::LoadFromFile(const char * fileName, int offset, int size)
{
    BinaryReader * reader = new BinaryReader();

    if(reader->OpenFile(fileName, offset, size) != 0)
    {
        delete reader;
        return NULL;
    }

    MotionFrames * frames = ArmFile::LoadFrame(reader);

    delete reader;
    return frames;
}

MotionFrames * ArmFile::LoadFrame(BinaryReader * reader)
{
    byte * bytes;

//    reader.Skip(offset);
//    LOGE("###", "Load skip position:%ld", reader.CurrentPosition());
    bytes = reader->Read(4); //file startcode
//    LOGE("###", "Load read 4byte position:%ld", reader->CurrentPosition());

    if (ByteCompare(startcode, bytes, 4) == false)
    {
        LOGE("###", "Load ARMfile startcode error");
        return NULL;
    }
    int version = reader->ReadUInt16(); // version
    int contentSize = reader->ReadUInt32();
    if (contentSize != reader->size)
    {
        LOGE("###", "Load ARMfile size error");
        return NULL;
    }
    reader->Read(255); //reserverd
    int frameTime = reader->ReadUInt16();
    int frameCount = reader->ReadUInt32();
    int duration = reader->ReadUInt16(); //sec
    int frameType = reader->ReadByte();
    int rhythm = reader->ReadByte();
    
    int actCount = 0;
    int frameDuration = 0;
    int frameRunTime = 0;

    if (frameCount == 0)
    {
        LOGE("###", "Load ARMfile framecount error");
        return NULL;
    }
    MotionFrames * frames = new MotionFrames();

    frames->frameTime = frameTime;
    frames->frameType = frameType;
    frames->duration = duration;
    frames->Rhythm = 2;
    if(version >= 0x1606)
    {
        LOGE("###", "arm file rhythm:%d", rhythm);
        frames->Rhythm = rhythm;
    }
    frames->frameList.resize(frameCount);
    for (int i = 0; i < frameCount; i++)
    {
        frames->frameList.replaceAt(NULL, i);
    }
    for (int i = 0; i < frameCount; i++)
    {
        bytes = reader->Read(4); //frame startcode
        if (ByteCompare(framestart, bytes, 4) == false)
        {
            LOGE("###", "Load arc file framestart error:%d", i);
            break;
        }
        if (version >= 0x1606)
        {
            frameDuration = reader->ReadUInt16();
            frameRunTime = reader->ReadUInt16();
            actCount = reader->ReadUInt16();
            reader->ReadUInt16();
        }
        else
        {
            frameDuration = reader->ReadUInt32();
            actCount = reader->ReadUInt32();
            frameRunTime = frameDuration;
        }
        if (frameDuration == 0 || frames->frameType == 1/*CFR*/)
        {
            frameDuration = frames->frameTime;
        }
        MotionFrames::Frame * frame = new MotionFrames::Frame(frameDuration);
        frame->runTime = frameRunTime;
        if (i > 0)
        {
            MotionFrames::Frame * preFrame = frames->frameList[i - 1];
            frame->timePosition = preFrame->timePosition + preFrame->duration;
        }
        if (actCount > 0)
        {
            int len = actCount * 10;
            byte * buffer = new byte[len];
            buffer = reader->ReadCommand(len);
            if(buffer == NULL)
            {
                delete frame;
                break;
            }

            frame->commandLength = len;
            frame->commandList = buffer;
        }
        frames->frameList.replaceAt(frame, i);
    }

    reader->Close();
    return frames;
}

