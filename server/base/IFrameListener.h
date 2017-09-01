#ifndef IFrameListener_H_
#define IFrameListener_H_

class IFrameListener
{
    public:
        virtual ~IFrameListener(){}
        virtual int update(int timeSincelastFrame) = 0;
};

#endif //IFrameListener_H_
