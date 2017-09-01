#include <stdio.h>
#include <string.h>

#include "RobotSession.h"
#include "MotorID.h"
#include "MessageBlock.h"

using namespace RobotNetwork;

class onComplete : public MessageBlock::OnCompleted
{
	public:
		virtual void run(RobotMessage * msg)
		{
			printf("onComplete run\n");
		}
};


int main()
{
	RobotSession & robot = RobotSession::Instance();
	robot.Start("192.168.100.117");
	robot.MotorRun(MotorID::Walk, 1000);
    printf("press any key to reset\n");
    getchar();
	robot.MotorRun(MotorID::Turn, 60);
    printf("press any key to exit\n");
    getchar();
	robot.Close();
	return 0;
}
