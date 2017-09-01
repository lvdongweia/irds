#ifndef MOTIONFRAMES_H_
#define MOTIONFRAMES_H_

#include <utils/Vector.h>

class MotionFrames
{
public:
    class Frame
    {
        public:
            int runTime;
            int duration;
            int timePosition;
            int commandLength;
            unsigned char * commandList;
        public:
            Frame(int duration)
            {
                this->timePosition = 0;
                this->commandList = NULL;
                this->commandLength = 0;
                this->duration = duration;
                this->runTime = duration;
            }
            ~Frame()
            {
                if (commandList != NULL)
                {
                    delete commandList;
                }
                commandList = NULL; 
            }
    };
    public:
        int frameTime;
        int frameType;
        int Rhythm;
        int duration;
        android::Vector<Frame *> frameList;
    public:
        MotionFrames()
        {
            frameTime = 0;
            frameType = 0;
            duration = 0;
            Rhythm = 0;
        }
        ~MotionFrames()
        {
            int size = frameList.size();
            for (int i = 0;  i < size ; i++ )
            {
                Frame * frame = frameList[i];
                if (frame) delete frame;
            }
            frameList.clear();
        }
};

#endif //MOTIONFRAMES_H_


