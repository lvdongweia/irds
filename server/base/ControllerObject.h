#ifndef ControllerObject_H_
#define ControllerObject_H_ 

#include <utils/List.h>
#include "ICompletionListener.h"

namespace android
{
    class ControllerObject : public CommandObject
    {
        public:
            int target;
            int arg1;
            int arg2;
        public:
            ControllerObject(int sender_id = 0);
            ~ControllerObject();
        private:
            int RmSerialiaztion();
            int RbSerialization();
            int RcSerialization();
            int RfSerialization();
        public:
            virtual int  Send();
            virtual bool Equal(const CommandObject * obj);
        protected:
            void Clear();
        public:
            int Run(int to, int command, int arg1, int arg2, const sp<ICompletionListener> & onCompletionListener = 0);
    };
};

#endif // ControllerObject_H_

