#include <utils/threads.h>

#include <stdio.h>
#include <stdlib.h>
#include <media/mediaplayer.h>
#include <binder/ProcessState.h>
#include <system/audio.h>
#include <android/log.h>
#include "ipal_music_bridge.h"

#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, "ipal_music_bridge", __VA_ARGS__)

using namespace android;

class jI_MusicPlayerBridgeImpl : public jI_MusicPlayerBridge {
public:
    jI_MusicPlayerBridgeImpl() : currentPos(0) {
        LOGI("jI_MusicPlayerBridgeImpl::jI_MusicPlayerBridgeImpl\n");
        ProcessState::self()->startThreadPool();
        j__mediaPlayer = new MediaPlayer();
    }

public:
    virtual int say(char*) {
        LOGI("jI_MusicPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return 1;
    }

    virtual int play(char *path) {
        LOGI("jI_MusicPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        int duration = 0;
        if (j__mediaPlayer->isPlaying()) {
            j__mediaPlayer->stop();
        }
        j__mediaPlayer->reset();
        j__mediaPlayer->setDataSource(path, NULL);
        j__mediaPlayer->setAudioStreamType((audio_stream_type_t)3);
        j__mediaPlayer->prepare();
        j__mediaPlayer->getDuration(&duration);
        j__mediaPlayer->start();
        return duration;
    }

    virtual int music_start(char *path) {
        LOGI("jI_MusicPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        if (j__mediaPlayer->isPlaying()) {
            j__mediaPlayer->stop();
        }
        j__mediaPlayer->reset();
        j__mediaPlayer->setDataSource(path, NULL);
        j__mediaPlayer->setAudioStreamType((audio_stream_type_t)3);
        j__mediaPlayer->setLooping(true);
        j__mediaPlayer->prepare();
        return j__mediaPlayer->start();
    }

    virtual int music_join() {
        LOGI("jI_MusicPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__mediaPlayer->stop();
    }

    virtual int pause() {
        LOGI("jI_MusicPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        j__mediaPlayer->getCurrentPosition(&currentPos);
        return j__mediaPlayer->pause();
    }

    virtual int resume() {
        LOGI("jI_MusicPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__mediaPlayer->seekTo(currentPos);
    }

    virtual int stop() {
        LOGI("jI_MusicPlayerBridgeImpl IN %s, %d\n", __FUNCTION__, __LINE__);
        return j__mediaPlayer->stop();
    }

private:
    sp<MediaPlayer> j__mediaPlayer;
    int currentPos;
};

jI__EXPORT jI_MusicPlayerBridge*  jI_MusicPlayerBridge__New(void* reserved) {
    (void)reserved;

    static jI_MusicPlayerBridgeImpl sigleInstance;
    return &sigleInstance;
}

jI__EXPORT void  jI_MusicPlayerBridge__Destroy(jI_MusicPlayerBridge* j__bridge) {
    (void)j__bridge;
}
