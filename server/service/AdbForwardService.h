/*
*/

#ifndef AdbForwardService_H__
#define AdbForwardService_H__

#include <utils/Log.h>
#include <utils/Log.h>
#include <utils/threads.h>

namespace android
{
    class AdbForwardService
    {
        class Looper : public Thread
        {
            public:
                Looper();
                virtual  ~Looper();
                virtual bool threadLoop(); //
            public:
                void stop();
            private:
                bool bStop;
        };
        public:
            AdbForwardService();
            ~AdbForwardService();
         public:
            int Run();
            int Stop();
         private:
            sp<Looper> looper;
    };
};

#endif //AdbForwardService_H__

