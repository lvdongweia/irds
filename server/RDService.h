/*
*/

#ifndef RDService_H__
#define RDService_H__

#include <utils/Log.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <utils/String8.h>

#include "IRDService.h"
#include "IFrameListener.h"

namespace android
{
    class ITopic;
    class RawCommandService;
    class SystemController;
    class MotionService;
    class MotionPlayer;
    class AdbForwardService;

    class RDService : public BnRDService
    {
        private:
            class Looper : public Thread
            {
                public:
                    Looper();
                    virtual  ~Looper();
                    virtual bool threadLoop(); //
                public:
                    void stop();
                    void addFrameListener(IFrameListener * listener);
                    void removeFrameListener(IFrameListener * listener);
                private:
                    bool bStop;
                    int frameTime;
                    List<IFrameListener *> listenerList;
                    Mutex listLock;
            };
        public:
            RDService();
        private:
            ~RDService();
        public:
            static RDService * instance;
            static RDService * Instance();
        public:
            void update();
            void addFrameListener(IFrameListener * listener);
            void removeFrameListener(IFrameListener * listener);
            void notifyCommandCompleted(int sender_id, int session_id, int result, int errorcode);
            void notifyMessage(int from, int what, int arg1, int arg2);
        public:
            virtual sp<IRawCommandService> getRawCommandService(const void * token, int len);
            virtual sp<ISystemController> getSystemController(const void * token, int len);
            virtual sp<IMotionService> getMotionService(const void * token, int len);
            virtual sp<IMotionPlayer> getMotionPlayer(const void * token, int len);
            virtual int subscribe(const sp<ITopic> & topic, const void * token, int len);
        private:
            void Main();
        private:
            Mutex serviceLock;
            sp<Looper> looper;
            //device
            sp<RawCommandService> rawCommandService;
            List< sp<SystemController> > systemControllers;
            List< sp<MotionService> > motionServices;
            List< sp<MotionPlayer> > motionPlayers;
            AdbForwardService * adbForwardService;
    };

    /**********************************************************************************/

}; /* namespace android */

#endif /* RDService_H___ */
