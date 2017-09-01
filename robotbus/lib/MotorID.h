#pragma once

class MotorID
{
public:
    enum
    {
        ShoulderRightFrontBack = 1,
        ShoulderRight = 2,
        ShoulderRightRotate = 3,
        ElbowRight = 4,
        WristRightRotate = 5,
        ShoulderLeftFrontBack = 9,
        ShoulderLeft = 10,
        ShoulderLeftRotate = 11,
        ElbowLeft = 12,
        WristLeftRotate = 13,
        WAIST_TILT = 19,
        Neck_Rotate = 20,
        NectTilt = 21,
        ThumbRight =32,
        ForeFingerRight = 33,
        MiddleFingerRight = 34,
        RingFingerRight = 35,
        LittleFingerRight = 36,
        ThumbLeft = 37,
        ForeFingerLeft = 38,
        MiddleFingerLeft = 39,
        RingFingerLeft = 40,
        LittleFingerLeft = 41,
        Walk = 48,
        Turn = 49,
        Stop = 50,
    };
};
