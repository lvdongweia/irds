/*
*/
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <utils/String8.h>

#include "ICompletionListener.h"
#include "IMotionPlayer.h"
#include "RDSLog.h"
namespace android 
{
    enum
    {
        RELEASE = IBinder::FIRST_CALL_TRANSACTION,
        GET_IS_SAFELY,
        SET_LISTENER,
        SET_DATASOURCE,
        SET_DATASOURCEBYSTREAM,
        RUN_PREPARE,
        RUN_START,
        RUN_PAUSE,
        RUN_RESUME,
        RUN_STOP,
        GET_DURATION,
        GET_POSITION,
        SET_POSITION,
        SET_ENABLERUN,
        GET_RUNSTATUS,
    };

    class BpMotionPlayer : public BpInterface<IMotionPlayer>
    {
        public:
            BpMotionPlayer(const sp<IBinder>& impl)
                : BpInterface<IMotionPlayer>(impl)
            {
            }
            virtual ~BpMotionPlayer()
            {
                release();
            }
            void release()
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                remote()->transact(RELEASE, data, &reply);
            }
            int isSafely()
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());

                remote()->transact(GET_IS_SAFELY, data, &reply);
                return reply.readInt32();
            }
            int setCompletionListener(const sp<ICompletionListener> & onCompletionListener)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                data.writeStrongBinder(onCompletionListener->asBinder());
                remote()->transact(SET_LISTENER, data, &reply);
                return reply.readInt32();
            }
            int setDataSource(const char * path, int offset, int size)
            {
                Parcel data, reply;
                int len = strlen(path) + 1;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                data.writeInt32(len);
                data.write((const void * )path, len);
                data.writeInt32(offset);
                data.writeInt32(size);
                remote()->transact(SET_DATASOURCE, data, &reply);
                return reply.readInt32();
            }
            int setDataSourceByStream(const unsigned char * bytes, int size)
            {
                 Parcel data, reply;
                 data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                 data.writeInt32(size);
                 data.write((const void * )bytes, size);
                 remote()->transact(SET_DATASOURCEBYSTREAM, data, &reply);
                 return reply.readInt32();
            }
            int prepare()
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                remote()->transact(RUN_PREPARE, data, &reply);
                return reply.readInt32();
            }
            int start()
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                remote()->transact(RUN_START, data, &reply);
                return reply.readInt32();
            }
            int pause()
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                remote()->transact(RUN_PAUSE, data, &reply);
                return reply.readInt32();
            }
            int resume()
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                remote()->transact(RUN_RESUME, data, &reply);
                return reply.readInt32();
            }
            int stop()
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                remote()->transact(RUN_STOP, data, &reply);
                return reply.readInt32(); 
            }
            int getDuration()
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                remote()->transact(GET_DURATION, data, &reply);
                return reply.readInt32();
            }
            int getPosition()
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                remote()->transact(GET_POSITION, data, &reply);
                return reply.readInt32();
            }
            int setPosition(int pos)
            {
                Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                data.writeInt32(pos);
                remote()->transact(SET_POSITION, data, &reply);
                return reply.readInt32();
            }
            int setEnableRun(int enable)
            {
				Parcel data, reply;
				data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
				data.writeInt32(enable);
                remote()->transact(SET_ENABLERUN, data, &reply);
                return reply.readInt32();
			}
			int getRunStatus()
			{
				Parcel data, reply;
                data.writeInterfaceToken(IMotionPlayer::getInterfaceDescriptor());
                remote()->transact(GET_RUNSTATUS, data, &reply);
                return reply.readInt32();
			}
    };

    IMPLEMENT_META_INTERFACE(MotionPlayer, "inf.rds.motion.player");

    /**********************************************************************/
    status_t BnMotionPlayer::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        switch (code) {
            case RELEASE:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    release();
                    return NO_ERROR;
                }
                break;
            case GET_IS_SAFELY:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    reply->writeInt32(isSafely());
                    return NO_ERROR;
                }
                break;
            case SET_LISTENER:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    const sp<ICompletionListener> listener = interface_cast<ICompletionListener>(data.readStrongBinder());
                    reply->writeInt32(setCompletionListener(listener));
                    return NO_ERROR;
                }
                break;
            case SET_DATASOURCE:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    int len = data.readInt32();
                    const void *pdata = data.readInplace(len);                  
                    int offset = data.readInt32();
                    int size = data.readInt32();
                    reply->writeInt32(setDataSource((const char*)pdata, offset, size));
                    return NO_ERROR;
                }
                break;
            case SET_DATASOURCEBYSTREAM:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    int size = data.readInt32();
                    const void *bytes = data.readInplace(size);
                    reply->writeInt32(setDataSourceByStream((const unsigned char *)bytes, size));
                    return NO_ERROR;
                }
            case RUN_PREPARE:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    reply->writeInt32(prepare());
                    return NO_ERROR;    
                }
                break;
            case RUN_START:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    reply->writeInt32(start());
                    return NO_ERROR;
               }
                break;
            case RUN_PAUSE:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    reply->writeInt32(pause());
                    return NO_ERROR;                    
                }
                break;
            case RUN_RESUME:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    reply->writeInt32(resume());
                    return NO_ERROR;                    
                }
                break;
            case RUN_STOP:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    reply->writeInt32(stop());
                    return NO_ERROR;                    
                }
                break;          
            case GET_DURATION:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    reply->writeInt32(getDuration());
                    return NO_ERROR;                    
                }
                break;
            case GET_POSITION:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    reply->writeInt32(getPosition());
                    return NO_ERROR;                    
                }
                break;
            case SET_POSITION:
                {
                    CHECK_INTERFACE(IMotionPlayer, data, reply);
                    int pos = data.readInt32();
                    reply->writeInt32(setPosition(pos));
                    return NO_ERROR;                    
                }
                break;
             case SET_ENABLERUN:
				{
					CHECK_INTERFACE(IMotionPlayer, data, reply);
                    int enable = data.readInt32();
                    reply->writeInt32(setEnableRun(enable));
                    return NO_ERROR;
				}
				break;
			 case GET_RUNSTATUS:
				{
					CHECK_INTERFACE(IMotionPlayer, data, reply);
                    reply->writeInt32(getRunStatus());
                    return NO_ERROR;
				}
				break;
        }
        return BBinder::onTransact(code, data, reply, flags);
    }

    /***************************************************************************/

}; /* namespace android */
