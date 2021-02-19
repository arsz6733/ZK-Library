#ifndef __ZKTIMEATTENDANCE_H
#define __ZKTIMEATTENDANCE_H
#include <stdint.h>

#define SENDER_PORT_NUM 6000
#define SENDER_IP_ADDRESS "192.168.1.1"

/* Change This to your ZK Time Attendance Ip and port address*/
#define ZK_DEVICE_PORT_NUM 1
#define ZK_DEVICE_IP_ADDRESS "192.168.1.201"

typedef enum{
	ZK_ERROR=0,
	ZK_OK=1,
	ZK_TIMEOUT
}ZK_ErrorCode;



ZK_ErrorCode ZK_Connect(void);
ZK_ErrorCode ZK_DisConnect(void);
ZK_ErrorCode ZK_DisableDevice(void);
ZK_ErrorCode ZK_EnableDevice(void);
ZK_ErrorCode ZK_GetAttRecordSize(uint32_t * RecordQuantity);
ZK_ErrorCode ZK_GetAttRecords(char **  AttRecordBuff,uint16_t * AttRecoordBuffSize,char * ZK_Buffer,int ZK_BUFF_SIZE,uint32_t StartRec, uint32_t StopRec);
ZK_ErrorCode ZK_EnableLiveCapture(void);
ZK_ErrorCode ZK_GetLiveCaptureRec(char ** DataLocation,char * ZK_Buffer,int ZK_BUFFER_SIZE,int *ZK_ReceivedDataSize);

#endif

