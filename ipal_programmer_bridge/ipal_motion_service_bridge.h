
#ifndef __INCL_IPOL_MOTION_SERVICE_BRIDGE_H__
#define __INCL_IPOL_MOTION_SERVICE_BRIDGE_H__

class jI_MotionEventListener {
public:
    virtual ~jI_MotionEventListener() {}

public:
    virtual int onCompletion(int session_id, int result, int error_code) = 0;
};

class jI_MotionPlayerBridge {
public:
    jI_MotionPlayerBridge(){}
    virtual ~jI_MotionPlayerBridge() {}
public:
    virtual int setCompletionListener(jI_MotionEventListener* onCompletionListener) = 0;
    virtual int stop(int session_id = 0) = 0;
    virtual int reset(int id = 0) = 0;
    virtual int doAction(int id, int repeat, int duration) = 0;
    virtual int run(const void * pdata, int len, int duration = 0, int flags = 0) = 0;
    virtual int runMotor(int id, int angle, int duration/*ms*/, int flags) = 0;
    virtual int walk(int distance/*cm*/, int speed, int flag = 0) = 0;
    virtual int walkTo(int target, int speed, int flag = 0) = 0;
    virtual int turn(int angle, int speed) = 0;
    virtual int emoji(int id, int duration, int repeat) = 0;
};

#ifdef _WIN32
#define jI__EXPORT    __declspec(dllexport)
#else
#define jI__EXPORT    __attribute__((visibility ("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

jI__EXPORT jI_MotionPlayerBridge*  jI_MotionPlayerBridge__New(void* reserved);
jI__EXPORT void  jI_MotionPlayerBridge__Destroy(jI_MotionPlayerBridge* j__bridge);

#ifdef __cplusplus
};
#endif
#endif /* __INCL_IPOL_MOTION_SERVICE_BRIDGE_H__ */
