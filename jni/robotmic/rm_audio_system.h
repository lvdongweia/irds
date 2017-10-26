#ifndef __RM_AUDIO_SYSTEM_H_
#define __RM_AUDIO_SYSTEM_H_

#include <utils/Mutex.h>

#include "JNIHelp.h"
#include "jni.h"

class RmAudioSystem
{
public:
    enum MicTypeApp
    {
        MIC_SINGLE = 0,
        MIC_ARRAY = 1,
    };

    static RmAudioSystem *instance();
    static void destroy();

    int setMicMode(int value);
    int getMicMode(int &value);

    int setMicType(int type);
    int getMicType(int &type);

private:
    enum MicNode
    {
        MIC_NO = -1,
        MIC_CTL = 0,
        MIC_SELECT = 1,
    };

    RmAudioSystem();
    ~RmAudioSystem();

    int openMicSelectNode();
    int openMicCtlNode();
    int openMicNode(const char* path);

    int closeMicNode();
    bool checkMicNode(int node);

    int writeMicPara(const char *value, size_t len, int node);
    int readMicPara(int &value, int node);

    bool checkMicType();

    int mMicType;
    android::Mutex mLock;

    int mNodeFd;
    int mMicNode;
    static RmAudioSystem *mAudioSystem;
};

int registerAudioSystem(JNIEnv* env);
int unregisterAudioSystem(JNIEnv* env);

#endif /* __RM_AUDIO_SYSTEM_H_ */
