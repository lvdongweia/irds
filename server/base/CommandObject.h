#ifndef CommandObject_H_
#define CommandObject_H_ 

#include <utils/List.h>
#include "ICompletionListener.h"
#include "IResultListener.h"

namespace android
{
    class CommandObject
    {
        public:
            int cls_id;
            int sender_id;
            int timeout;
            int flags;
            int repeat;
            sp<ICompletionListener> callback;
            sp<IResultListener> onResult;
        public:
            int co_id; //command_id
            int task_id;
            int board_id;
            int motor_id;
            int cmd_count; // comand count
            float startTime;
            int task_session_id;
            unsigned char * streams;
        public:
            CommandObject(int sender_id, int cls_id);
            virtual ~CommandObject();
        protected:
            int  startSend();
        public:
            virtual int Stop();
            virtual int Finish(int result, int errorcode);
            virtual int Send();
            virtual bool Blocked(const CommandObject * obj);
            virtual bool Equal(const CommandObject * obj);
    };
    class SettingObject : public CommandObject
    {
        public:
            unsigned char idset[32];
            unsigned char val;
            bool enabled; 
        public:
            SettingObject(int sender_id = 0);
            virtual ~SettingObject();
        public:
            virtual int Send();
            virtual int Finish(int result, int errorcode);
            virtual bool Equal(const CommandObject * obj);
        public:
            int Run(int id, int cmd, int arg1, int arg2, const sp<IResultListener> & onResultListener);
        private:
            void Clear();

    };
    class MotorMotion : public CommandObject
    {
        public:
            int angle;
            int duration;
            int dataSize;
            int dataCap;
            unsigned char * pData;

        public:
            MotorMotion(int sender_id = 0);
            virtual ~MotorMotion();
        public:
            virtual int  Send();
            virtual bool Equal(const CommandObject * obj);
        public:
            int Run(int joint_id, int angle, int duration, int flags);
            int Run(const unsigned char * pdata, int len, int duration);
        private:
            void Clear();
    };
    class ActionMotion : public CommandObject
    {
        public:
            int task_type_id;
            int action_id;
            int duration;
        public:
            ActionMotion(int sender_id = 0);
            virtual ~ActionMotion();
        public:
            virtual int  Send();
        public:
            int Run(int action_id, int repeat, int duration);
    };

    class PositionMotion : public CommandObject
    {
        public:
            union 
            {
                int distance;
                int angle;
            };
            int speed;
        public:
            PositionMotion(int sender_id = 0);
            virtual ~PositionMotion();
        public:
            virtual int Send();
            virtual int Stop();
        public:
            static int Send(int task_id, int act_id, unsigned char * pdata, int len);
            int Walk(int distance, int speed, int flag);
            int WalkTo(int target, int speed, int flag);
            int Turn(int angle, int speed);
        private:
            int newStopCommand();
    };
    class EmojiMotion : public CommandObject
    {
        public:
            int id;
            int duration;
        public:
            EmojiMotion(int sender_id = 0);
            ~EmojiMotion();
        public:
            virtual int  Send();
        protected:
            void Clear();
        public:
            int Run(int id, int duration, int repeat);
    };
    class StopMotion : public CommandObject
    {
        public:
            StopMotion(int sender_id = 0);
            ~StopMotion();
        public:
            virtual int Send();
        public:
            static int Run(int board_id, unsigned char * data);
            static int Run(int id = 0, int bodyPart = 0, int arg1 = 0);
    };
    class ResetMotion : public CommandObject
    {
        public:
            ResetMotion(int sender_id = 0);
            ~ResetMotion();
        public:
            int id;
        public:
            virtual int Send();
        public:
            static int Run(int id);
            static int Run();
    };
};

#endif // CommandObject_H_

