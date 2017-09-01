#ifndef RDSTask_H_
#define RDSTask_H_

#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/threads.h>

#include "ICompletionListener.h"
#include "IFrameListener.h"

namespace android
{
    class CommandObject;
    class RDSTask : public IFrameListener
    {
        private:
            CommandObject * outList[255];
            List<CommandObject *> inList;
        public:
            RDSTask();
            virtual ~RDSTask();

            static RDSTask * instance;
            static RDSTask * Instance();
            static void Destroy();
			static bool motorDisabled;
        private:
            void checkTimeout(int timeSincelastFrame);
        public:
            virtual int update(int timeSincelastFrame);
            int addCommand(CommandObject * obj);
            int sendCommand(int dest, const unsigned char * streams, int len);
            void stopMotion(int clsid, int dest, const unsigned char * streams, int len);
            void cancelCommand(int session_id);
            void onCommandCompletion(int id, int result, int flags = 0);
    };
};

#endif // RDSTask_H_
