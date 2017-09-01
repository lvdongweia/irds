
#ifndef __INCL_IPOL_MUSIC_BRIDGE_H__
#define __INCL_IPOL_MUSIC_BRIDGE_H__


class jI_MusicPlayerBridge {
public:
    virtual ~jI_MusicPlayerBridge() {}

public:
    virtual int say(char *txt) = 0;
    virtual int play(char *path) = 0;
    virtual int music_start(char *path) = 0;
    virtual int music_join() = 0;
    virtual int pause() = 0;
    virtual int resume() = 0;
    virtual int stop() = 0;
};

#ifdef _WIN32
#define jI__EXPORT __declspec(dllexport)
#else
#define jI__EXPORT __attribute__((visibility ("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

jI__EXPORT jI_MusicPlayerBridge*  jI_MusicPlayerBridge__New(void* reserved);
jI__EXPORT void  jI_MusicPlayerBridge__Destroy(jI_MusicPlayerBridge* j__bridge);

#ifdef __cplusplus
};
#endif
#endif /* __INCL_IPOL_MUSIC_BRIDGE_H__ */