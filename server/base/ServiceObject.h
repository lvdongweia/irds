#ifndef ServiceObject_H_
#define ServiceObject_H_

#include "CommandObject.h"

namespace android
{
    class ServiceObject
    {
        public:
            ServiceObject(const void * token, int len);
            virtual ~ServiceObject();
        public:
            void onChanged(int status);
            void onCompletion(int session_id, int result, int errorcode);
            void clear();
        public:
            int uid;
            bool isUnset;
        protected:
            char * token;
            sp<ICompletionListener> onCompletionListener;
    };
};

#endif //ServiceObject_H_
