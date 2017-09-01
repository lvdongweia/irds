#ifndef DEVICE_TABLE_H_
#define DEVICE_TABLE_H_


namespace DeviceTable
{

    int getMotorAndBoardID(int joint_id, int & motor_id, int & board_id);
    int getSystemMotion(int id);
    int getEmoji(int id);
    int getRobotBodyPart(int id);
    int getSensorId(uint8_t id);
    int getSysCtrlCmd(int respid);
    int getSettingCommand(int cmd);
    int getSettingIDList(int ids, unsigned char * idList, int len);

};

#endif //DEVICE_TABLE_H_
