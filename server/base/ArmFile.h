#ifndef ARM_FILE_H_
#define ARM_FILE_H_

#include "MotionFrames.h"

typedef unsigned char byte;

class BinaryReader;

class ArmFile
{
    private:
        static MotionFrames * LoadFrame(BinaryReader * reader);
    public:
        static MotionFrames * LoadFromStream(const byte * bytes, int size);
        static MotionFrames * LoadFromFile(const char * fileName, int offset, int size);
};

#endif // ARM_FILE_H_


