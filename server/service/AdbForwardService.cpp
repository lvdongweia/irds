/*

*/

#include <stdint.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "RDSLog.h"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include "AdbForwardService.h"
#include "ServiceObject.h"
#include "DeviceTable.h"
#include "robot_common_data.h"
#include "DeviceMacros.h"
#include "CANSession.h"

namespace android
{
    static unsigned char streams[8];
    static bool bClose = false;

    static void motionReset()
    {
        int act_id, board_id;

        DeviceTable::getMotorAndBoardID(0x3E/*reset*/, act_id, board_id);
        streams[0] = act_id;
        streams[1] = 5;
        streams[2] = 0xFF;
        streams[3] = 0; 
        streams[4] = 1;    
        CANSession::Instance()->Send(board_id, streams, 5);
    }
    static void motionCommand(unsigned char * pData)
    {
        int joint_id = 0;
        int motor_id = 0;
        int board_id = 0;
        int duration = 0;

        joint_id = pData[3];
        DeviceTable::getMotorAndBoardID(joint_id, motor_id, board_id);

        if (motor_id == WHEEL_LEFT || motor_id == WHEEL_RIGHT)
        {
            return;
        }
        if (board_id != RB_R_ARM_MOTOR_CTRL_ID && 
                board_id != RB_L_ARM_MOTOR_CTRL_ID &&
                board_id != RB_BODY_MOTOR_CTRL_ID)
        {
            return;
        }
        streams[0] = RB_MOTOR_CTRL_RUN; //type
        streams[1] = 8;//length
        streams[2] = 0xFF;
        streams[3] = motor_id;
        streams[4] = pData[4]; //angle
        streams[5] = pData[5]; //angle
        streams[6] = pData[6]; //flags   
        duration = pData[7]; // duration 100ms

        if (duration <= 0 || duration > 100)
        {
            duration = 2;
        }
        streams[7] = (unsigned char)duration;
        CANSession::Instance()->Send(board_id, streams, 8);
    }
    static void * run(void * args )
    {
        int sockfd,new_fd;
        struct sockaddr_in my_addr;
        struct sockaddr_in their_addr;
        unsigned int size, port = 20528, lisnum = 1;

        unsigned char buffer[10];
        if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) { 
            perror("socket"); 
            return 0;
        } 
        my_addr.sin_family=PF_INET; 
        my_addr.sin_port=htons(port); 
        my_addr.sin_addr.s_addr = INADDR_ANY; 
        bzero(&(my_addr.sin_zero), 0); 
        if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) { 
            perror("bind"); 
            return 0;
        } 
        printf("bind ok \n");

        if (listen(sockfd, lisnum) == -1) { 
            perror("listen"); 
            return 0;
        }
        printf("listen ok \n");

        while(bClose == false) { 
            socklen_t sin_size = sizeof(struct sockaddr_in); 
            LOGE("### adb service", "wait for connect.....");
            if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) { 
                perror("accept"); 
                continue; 
            }
            LOGE("### adb service", "new client:%s", inet_ntoa(their_addr.sin_addr));
            do
            {
                size = read(new_fd, buffer, 10);
                LOGE("### adb service", "recv data size:%d [0]:0x%x [1]:0x%x", size, buffer[0], buffer[1]);
                if (size == 10 && buffer[0] == 0xF1 && buffer[1] == 0x8E)
                {
                    motionCommand(buffer);

                    LOGE("### adb service", "server: got connection from %s datasize:%d\n",inet_ntoa(their_addr.sin_addr), size); 
                }
                else
                {
                    motionReset();
                    LOGE("### adb service", "motion reset");
                }
            } while (size > 0);
            LOGE("### adb service", " client close");
        }
        close(sockfd);
        //return motorMotion.Run((const unsigned char *)pdata, len, duration);
        return 0;
    }

    AdbForwardService::Looper::Looper()
    {
        bStop = false;
    }
    AdbForwardService::Looper::~Looper()
    {
        bStop = true;
    }
    bool AdbForwardService::Looper::threadLoop()
    {
        if (bStop == true)
        {
            return false;
        }
        //todo
        return (bStop == false);
    }
    void AdbForwardService::Looper::stop()
    {
        bStop = true;
        requestExitAndWait();
    }

    AdbForwardService::AdbForwardService()
    {
    }
    AdbForwardService::~AdbForwardService()
    {
    }
    int AdbForwardService::Run()
    {
        pthread_t tid = 0;
        int ret = pthread_create(&tid, NULL, run, NULL);
        //looper = new Looper();
        //looper->run("rds main looper");
        return 0;
    }
    int AdbForwardService::Stop()
    {
        bClose = true;
        //looper->stop();
        //looper.clear();

        return 0;
    }
};
