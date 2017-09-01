#ifndef RDS_MACROS__H_
#define RDS_MACROS__H__
/************************************
************************************/

enum RDSTopicEvent
{
    RDS_EVENT_ALL                   = 0xFFFF,
    RDS_EVENT_TOUCH                 = 1 << 0,
    RDS_EVENT_SENSOR                = 1 << 1,
    RDS_EVENT_SYSTEM                = 1 << 2,
    RDS_EVENT_REPORT                = 1 << 3,
    RDS_EVENT_SOUND_LOCALIZATION    = 1 << 4,
    RDS_EVENT_SOUND_TEXT            = 1 << 5,
    RDS_EVENT_COMPLETION            = 1 << 6,
    RDS_EVENT_LONG_TOUCH            = 1 << 7,
    RDS_EVENT_RELEASE_TOUCH         = 1 << 8,

    //if not enough, modify RDS_EVENT_ALL value
    RDS_EVENT_MAX                   = 1 << 15,
};

#endif

