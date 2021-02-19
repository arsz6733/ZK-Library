#include "zkTimeAttendance.h"
#include <lwip/sockets.h>


#define ZK_MAX_PACKET_SIZE  36


#define CMD_CONNECT          1000  // Connections requests
#define CMD_EXIT             1001  // Disconnection requests
#define CMD_ENABLEDEVICE     1002  // Ensure the machine to be at the normal work condition
#define CMD_DISABLEDEVICE    1003  // Make the machine to be at the shut-down condition
#define CMD_RESTART          1004  // Restart the machine.
#define CMD_POWEROFF         1005  // Shut-down power source
#define CMD_SLEEP            1006  // Ensure the machine to be at the idle state.
#define CMD_RESUME           1007  // Awakens the sleep machine (temporarily not to support)
#define CMD_CAPTUREFINGER    1009  // Captures fingerprints picture
#define CMD_TEST_TEMP        1011  // Test some fingerprint exists or does not
#define CMD_CAPTUREIMAGE     1012  // Capture the entire image
#define CMD_REFRESHDATA      1013  // Refresh the machine interior data
#define CMD_REFRESHOPTION    1014  // Refresh the configuration parameter
#define CMD_TESTVOICE        1017  // Play voice
#define CMD_GET_VERSION      1100  // Obtain the firmware edition
#define CMD_CHANGE_SPEED     1101  // Change transmission speed
#define CMD_AUTH             1102  // Connections authorizations
#define CMD_PREPARE_DATA     1500  // Prepares to transmit the data
#define CMD_DATA             1501  // Transmit a data packet
#define CMD_FREE_DATA        1502  // Clear machines opened buffer
#define CMD_GET_FREE_SIZES   0x0032
#define CMD_DATA_WRRQ        1503
#define CMD_DATA_RDY				 1504
#define CMD_REG_EVENT				 500

#define CMD_ACK_OK           2000  // Return value for order perform successfully
#define CMD_ACK_ERROR        2001  // Return value for order perform failed
#define CMD_ACK_DATA         2002  // Return data
#define CMD_ACK_RETRY        2003  // * Regstered event occorred */
#define CMD_ACK_REPEAT       2004  // Not available
#define CMD_ACK_UNAUTH       2005  // Connection unauthorized

#define MACHINE_PREPARE_DATA_1  20560 // 0x5050
#define MACHINE_PREPARE_DATA_2  32130 // 0x7282
//const uint8_t ZK_ConnectCommand[]={0x50,0x50,0x82,0x7d,0x08,00,00,00,0xe8,0x03,0x17,0xfc,00,00,00,00};
uint8_t ZK_Command[64]={0x50,0x50,0x82,0x7d};
void CreateHeader(uint16_t command , uint16_t replyId , uint32_t commandSize);
ZK_ErrorCode SocketConnect(void);
ZK_ErrorCode SocketClose(void);
int ZK_send(void * data,unsigned int size);
int ZK_recv(void * data,unsigned int size,int timeout);
void CalculateChecksum(int32_t size);
void EncodeStartEnd(uint32_t StartingAddress,uint32_t EndingAddress);
void MakeCommKey(uint16_t SessionID,uint16_t Pass);

uint16_t SessionID = 0;
uint32_t ReplyNumber = 0;


int socket_fd;

ZK_ErrorCode ZK_Connect(void)
{
	int recv_data;
	uint8_t ZK_Data_Buffer[20];
	uint16_t ResponseCmd=0;
		SessionID = 0;
		ReplyNumber =0;
	
		if(SocketConnect() == ZK_ERROR)
			return ZK_ERROR;
		CreateHeader(CMD_CONNECT,0,8);
		CalculateChecksum(8);
		ZK_send((uint8_t *)ZK_Command,16);
		recv_data = ZK_recv(ZK_Data_Buffer,sizeof(ZK_Data_Buffer),15);
		if(recv_data != 16)
		{
			SocketClose();
			return ZK_ERROR;
		}
	//	printf("ConnectResponse\n");
//		for(int i =0; i<recv_data;i++){
//				printf("%x ",ZK_Data_Buffer[i]);
//		}
		ResponseCmd=(ZK_Data_Buffer[9]<<8) | ZK_Data_Buffer[8];
		if(ResponseCmd != CMD_ACK_OK && ResponseCmd != CMD_ACK_UNAUTH ){
			SocketClose();
			return ZK_ERROR;
		}
		SessionID = (ZK_Data_Buffer[13]<<8) | ZK_Data_Buffer[12];
		if(ResponseCmd == CMD_ACK_UNAUTH){
			ReplyNumber++;
			CreateHeader(CMD_AUTH,ReplyNumber,12);
			MakeCommKey(SessionID,0); /* pass is zero */ 
			CalculateChecksum(12);
		  ZK_send((uint8_t *)ZK_Command,20);
			recv_data = ZK_recv(ZK_Data_Buffer,sizeof(ZK_Data_Buffer),0);
			if(recv_data != 16)
			{
				SocketClose();
				return ZK_ERROR;
			}
			ResponseCmd=(ZK_Data_Buffer[9]<<8) | ZK_Data_Buffer[8];
			if(ResponseCmd != CMD_ACK_OK ){
				SocketClose();
				return ZK_ERROR;
			}			
		}
	return ZK_OK;
}

ZK_ErrorCode ZK_DisConnect(void)
{
	int recv_data;
	uint8_t ZK_Data_Buffer[20];
	uint16_t ResponseCmd=0;
		ReplyNumber++;
		CreateHeader(CMD_EXIT,ReplyNumber,8);
		CalculateChecksum(8);
		ZK_send((uint8_t *)ZK_Command,16);
		recv_data = ZK_recv(ZK_Data_Buffer,sizeof(ZK_Data_Buffer),0);
	//	printf("DisConnect\n");
//		for(int i =0; i<recv_data;i++){
//				printf("%x ",ZK_Data_Buffer[i]);
//		}
		ResponseCmd=(ZK_Data_Buffer[9]<<8) | ZK_Data_Buffer[8];
		if(ResponseCmd != CMD_ACK_OK){
			SocketClose();
			return ZK_ERROR;
		}
		SocketClose();
	return ZK_OK;
}

ZK_ErrorCode ZK_DisableDevice(void)
{
	int recv_data;
	uint8_t ZK_Data_Buffer[20];
	uint16_t ResponseCmd=0;
		ReplyNumber ++;
	
		CreateHeader(CMD_DISABLEDEVICE,ReplyNumber,8);
	  
		CalculateChecksum(8);
		ZK_send((uint8_t *)ZK_Command,16);
		recv_data = ZK_recv(ZK_Data_Buffer,sizeof(ZK_Data_Buffer),0);
		if(recv_data != 16)
		{
			return ZK_ERROR;
		}
	//	printf("DisableDevice\n");
//		for(int i =0; i<recv_data;i++){
//				printf("%x ",ZK_Data_Buffer[i]);
//		}
		ResponseCmd=(ZK_Data_Buffer[9]<<8) | ZK_Data_Buffer[8];
		if(ResponseCmd != CMD_ACK_OK){
			return ZK_ERROR;
		}
	return ZK_OK;
}

ZK_ErrorCode ZK_EnableDevice(void)
{
	int recv_data;
	uint8_t ZK_Data_Buffer[20];
	uint16_t ResponseCmd=0;
		ReplyNumber ++;
	
		CreateHeader(CMD_ENABLEDEVICE,ReplyNumber,8);
		CalculateChecksum(8);
		ZK_send((uint8_t *)ZK_Command,16);
		recv_data = ZK_recv(ZK_Data_Buffer,sizeof(ZK_Data_Buffer),0);
		if(recv_data != 16)
		{
			return ZK_ERROR;
		}
//printf("EnableDevice\n");
//		for(int i =0; i<recv_data;i++){
//				printf("%x ",ZK_Data_Buffer[i]);
//		}
		ResponseCmd=(ZK_Data_Buffer[9]<<8) | ZK_Data_Buffer[8];
		if(ResponseCmd != CMD_ACK_OK){
			return ZK_ERROR;
		}
	return ZK_OK;
}

ZK_ErrorCode ZK_EnableLiveCapture(void)
{
	int recv_data;
	uint8_t ZK_Data_Buffer[20];
	uint16_t ResponseCmd=0;
		ReplyNumber ++;
	
		CreateHeader(CMD_REG_EVENT,ReplyNumber,12);
		ZK_Command[16] =0xff;   // Create a Function
		ZK_Command[17] =0xff;
		ZK_Command[18] =0;
		ZK_Command[19] =0;
		CalculateChecksum(12);
		ZK_send((uint8_t *)ZK_Command,20);
		recv_data = ZK_recv(ZK_Data_Buffer,sizeof(ZK_Data_Buffer),0);
		if(recv_data != 16)
		{
			return ZK_ERROR;
		}
//rintf("EnableLiveCapture\n");
//		for(int i =0; i<recv_data;i++){
//				printf("%x ",ZK_Data_Buffer[i]);
//		}
		ResponseCmd=(ZK_Data_Buffer[9]<<8) | ZK_Data_Buffer[8];
		if(ResponseCmd != CMD_ACK_OK){
			return ZK_ERROR;
		}
	return ZK_OK;
}

ZK_ErrorCode ZK_GetLiveCaptureRec(char ** DataLocation,char * ZK_Buffer,int ZK_BUFFER_SIZE,int *ZK_ReceivedDataSize)
{
		int recv_data;
		uint8_t ZK_Data_Buffer[60];
		uint16_t ResponseCmd=0;
		ReplyNumber ++;
		*ZK_ReceivedDataSize = 0;
		recv_data = ZK_recv(ZK_Data_Buffer,sizeof(ZK_Data_Buffer),0);
		if(recv_data <0){
			return ZK_TIMEOUT;
		}
		if(recv_data == 16 ){
	//	printf("GetLiveCaptureData\n");
//		for(int i =0; i<recv_data;i++){
//			printf("%x ",ZK_Data_Buffer[i]);
//		}
		ResponseCmd=(ZK_Data_Buffer[9]<<8) | ZK_Data_Buffer[8];
		if(ResponseCmd == CMD_REG_EVENT){	
			CreateHeader(CMD_ACK_OK,0,8);
			CalculateChecksum(8);
			ZK_send((uint8_t *)ZK_Command,16);
			recv_data = ZK_recv(ZK_Buffer,ZK_BUFFER_SIZE,2);
			if(recv_data < 0)
				return ZK_ERROR;
			if(recv_data == (52)){   /*  16 Header + 36 Data */
			//	printf("LiveCaptureData\n");
//				for(int i =0; i<recv_data;i++){
//					printf("%x ",ZK_Buffer[i]);
//				}		
				*DataLocation = &ZK_Buffer[16];
				*ZK_ReceivedDataSize = recv_data - 16;
			}
			else{
				return ZK_ERROR;
			}
		}
	return ZK_OK;
	}
	else if(recv_data > 16){
		for (int i=16; i<52 ; i++){
			ZK_Buffer[i]=ZK_Data_Buffer[i];
		}
		*ZK_ReceivedDataSize = recv_data - 16;
		return ZK_OK;
	}
	else{
		return ZK_ERROR;
	}
}

ZK_ErrorCode ZK_GetAttRecordSize(uint32_t * RecordQuantity)
{
	int recv_data;
	uint8_t ZK_Data_Buffer[200];
	uint16_t ResponseCmd=0;
		ReplyNumber++;
		CreateHeader(CMD_GET_FREE_SIZES,ReplyNumber,8);
		CalculateChecksum(8);
		ZK_send((uint8_t *)ZK_Command,16);
		recv_data = ZK_recv(ZK_Data_Buffer,sizeof(ZK_Data_Buffer),0);
//		printf("recv_data-Size %d\n ", recv_data);
//		printf("ReadSize\n");
//		for(int i =0; i<recv_data;i++){
//				printf("%x ",ZK_Data_Buffer[i]);
//		}
		ResponseCmd=(ZK_Data_Buffer[9]<<8) | ZK_Data_Buffer[8];
		if(ResponseCmd != CMD_ACK_OK){
			return ZK_ERROR;
		}
		*RecordQuantity =ZK_Data_Buffer[51] << 24 | ZK_Data_Buffer[50] << 16 | ZK_Data_Buffer[49] << 8 |  ZK_Data_Buffer[48] ;
	return ZK_OK;
	
}
ZK_ErrorCode ZK_GetAttRecords(char **  AttRecordBuff,uint16_t * AttRecoordBuffSize,char * ZK_Buffer,int ZK_BUFF_SIZE,uint32_t StartRec, uint32_t StopRec)
{
		int recv_data ,recv_data1, recv_data2 ;
		uint16_t ResponseCmd=0;
		uint32_t ZK_DeviceBufferSize = 0 ;
		uint32_t StartingAddress=0 , DataSizeToRecv;
		uint8_t ZK_Resp_Buffer[30];
		uint8_t ZK_Ack_Buffer[30];
		uint32_t ReceveidDataSize = 0;	
	
		ReplyNumber++;
		CreateHeader(CMD_DATA_WRRQ,ReplyNumber,19);  // 8 + 11   8-> Command+seessionID+ReplyID+Checksum  11-> Data
		ZK_Command[16] =1;   // Create Function and add zeros
		ZK_Command[17] =0x0d;
		CalculateChecksum(19);
		ZK_send((uint8_t *)ZK_Command,27);   // CMD_DATA_WRRQ Data = 010d000000000000000000
		recv_data = ZK_recv(ZK_Buffer,ZK_BUFF_SIZE,0);
//		printf("recv_data-Size %d\n ", recv_data);
//		printf("GetAttRecords\n");
//		for(int i =0; i<recv_data;i++){
//				printf("%x ",ZK_Buffer[i]);
//		}
		ResponseCmd=(ZK_Buffer[9]<<8) | ZK_Buffer[8];
		if(ResponseCmd == CMD_DATA){
			printf("ZKError : ResponseCmd == CMD_DATA\n");
			return 	ZK_ERROR;	
		}
		else if(ResponseCmd == CMD_ACK_OK){
						ZK_DeviceBufferSize =(ZK_Buffer[20]<<16) | (ZK_Buffer[19]<<16) | (ZK_Buffer[18]<<8) | ZK_Buffer[17];
					//printf("ZK_DeviceBufferSize  = %d \n" , ZK_DeviceBufferSize);
						if((StopRec+1 - StartRec)>ZK_MAX_PACKET_SIZE){
							printf("ZKError : StopRec+1 - StartRec)>ZK_MAX_PACKET_SIZE\n");
							return ZK_ERROR;
						}
						StartingAddress = (StartRec * 40) /*+ 4*/ ; 
						if((StopRec - StartRec ) >= ZK_MAX_PACKET_SIZE ){
							DataSizeToRecv = ZK_MAX_PACKET_SIZE  * 40 ;
							StartRec += ZK_MAX_PACKET_SIZE;
						}
						else{
							DataSizeToRecv = (StopRec + 1 - StartRec ) * 40 ;
							StartRec += StopRec + 1 - StartRec ;
						}
						ReplyNumber++;						
						CreateHeader(CMD_DATA_RDY,ReplyNumber,16);			
						EncodeStartEnd(StartingAddress,DataSizeToRecv);
						CalculateChecksum(16);
						ZK_send((uint8_t *)ZK_Command,24);			
						recv_data = ZK_recv(ZK_Resp_Buffer,sizeof(ZK_Resp_Buffer),0);
						recv_data1 = ZK_recv(ZK_Buffer,DataSizeToRecv+16,0);
						recv_data2 = ZK_recv(ZK_Ack_Buffer,sizeof(ZK_Ack_Buffer),0);
//						printf("\nrecv_data-Size %d\n ", recv_data);
//						for(int i =0; i<recv_data;i++){
//								printf("%x ",ZK_Resp_Buffer[i]);
//						}		
						if(StartingAddress == 0){
//							for(int i =20; i<recv_data1 + 4 ;i++){
//									if(((i - 20 )%40 )== 0){
//										printf("\n %d -> " ,(StartingAddress  )/40 +  i/40);
//									}
//									printf("%x ",ZK_Buffer[i]);
//							 }	
							*AttRecordBuff = &ZK_Buffer[20];
							*AttRecoordBuffSize = recv_data1 - 20 + 4;
							 
						 }				
							else{
//							for(int i =16; i<recv_data1;i++){
//									if(((i - 16 )%40 )== 0){
//										printf("\n %d -> " ,(StartingAddress  )/40 +  i/40);
//									}
//									printf("%x ",ZK_Buffer[i]);
//							 }		
							*AttRecordBuff = &ZK_Buffer[16];
							*AttRecoordBuffSize = recv_data1 - 16;
						}
					return 	ZK_OK;		
			}
		else{
				printf("ZKError : ResponseCmd is not as expected\n");
			return 	ZK_ERROR;	
		}
	return 	ZK_ERROR;		
}
/* Tested Succefully*/
void EncodeStartEnd(uint32_t StartingAddress,uint32_t EndingAddress)
{
			ZK_Command[16] = StartingAddress & 0xff;
			ZK_Command[17] = (StartingAddress & 0xff00) >> 8;
			ZK_Command[18] = (StartingAddress & 0xff0000) >> 16;
			ZK_Command[19] = (StartingAddress & 0xff000000) >> 24 ;
			ZK_Command[20] = EndingAddress & 0xff;
			ZK_Command[21] = (EndingAddress & 0xff00) >> 8;
			ZK_Command[22] = (EndingAddress & 0xff0000) >> 16;
			ZK_Command[23] = (EndingAddress & 0xff000000) >> 24 ;
}
/* Tested Succefully*/
void CreateHeader(uint16_t command , uint16_t replyId , uint32_t commandSize)
{
		memset((uint8_t *)&ZK_Command[4],0,60);
	
		ZK_Command[4] = commandSize & 0xff;
		ZK_Command[5] = (commandSize & 0xff00) >> 8;
		ZK_Command[6] = (commandSize & 0xff0000) >> 16;
		ZK_Command[7] = (commandSize & 0xff000000) >> 24 ;
		ZK_Command[8] = (command & 0xff);
		ZK_Command[9] = (command & 0xff00)>> 8;
		ZK_Command[12] = SessionID & 0xff;
		ZK_Command[13] = (SessionID & 0xff00) >> 8;
		ZK_Command[14] = replyId & 0xff;
		ZK_Command[15] = (replyId & 0xff00) >> 8;
}
/* Tested Succefully*/
void MakeCommKey(uint16_t SessionID,uint16_t Pass)
{
	uint32_t Key=0;
	uint8_t KeyString[4];
	uint8_t KeyStringReversed[4];
	    for(int i=0;i<32;i++){
        if (Pass & (1 << i))
            Key = (Key << 1 | 1);
        else
            Key = Key << 1;
			}
		Key += SessionID;
		/*Xor each byte to the corresponding Ascii num*/
		KeyString[0]=(Key & 0xFF) ^ 'Z';
		KeyString[1]=((Key & 0xFF00)>>8) ^ 'K';
		KeyString[2]=((Key & 0xFF0000)>>16) ^ 'S';
		KeyString[3]=((Key & 0xFF000000)>>24) ^ 'O'; 
			
		/* substitute second short and first short    */
		KeyStringReversed[0] = KeyString[2];
		KeyStringReversed[1] = KeyString[3];    
		KeyStringReversed[2] = KeyString[0];
		KeyStringReversed[3] = KeyString[1];
		/* Fill the array			*/
		ZK_Command[16] = KeyStringReversed[0] ^ 50;   // 50 is a random number called ticks in python lib
		ZK_Command[17] = KeyStringReversed[1] ^ 50;
		ZK_Command[18] = 50;
		ZK_Command[19] = KeyStringReversed[3] ^ 50;
}
/* Tested Succefully*/
void CalculateChecksum(int32_t size)
{
		int32_t chk_32b=0;
		uint16_t chk_16b=0;
		int j = 1;
		uint16_t num_16b=0;
		for(j=1;j<size;j+=2){
			num_16b = ZK_Command[8+j-1] | (ZK_Command[8+j]<<8);
			chk_32b = chk_32b + num_16b;
		}
		chk_32b = (chk_32b & 0xffff) + ((chk_32b & 0xffff0000)>>16);
		chk_16b = chk_32b ^ 0xFFFF;
		
		ZK_Command[10] = chk_16b & 0xff;
		ZK_Command[11] = (chk_16b & 0xff00)>> 8;	
}

ZK_ErrorCode SocketConnect(void)
{
		struct sockaddr_in sa,ra;

		socket_fd = socket(PF_INET, SOCK_STREAM, 0);

		if ( socket_fd < 0 )
		{
		printf("socket call failed");
			return ZK_ERROR;
		}

		memset(&sa, 0, sizeof(struct sockaddr_in));
		sa.sin_family = AF_INET;
		//sa.sin_addr.s_addr =gnetif.ip_addr.addr;
		sa.sin_port = htons(SENDER_PORT_NUM);


		/* Bind the TCP socket to the port SENDER_PORT_NUM and to the current
		* machines IP address (Its defined by SENDER_IP_ADDR).
		* Once bind is successful for UDP sockets application can operate
		* on the socket descriptor for sending or receiving data.
		*/
		if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == -1)
		{
			printf("Bind to Port Number %d failed\n",SENDER_PORT_NUM);
			close(socket_fd);
			return ZK_ERROR;
		}
		/* Receiver connects to server ip-address. */

		memset(&ra, 0, sizeof(struct sockaddr_in));
		ra.sin_family = AF_INET;
		ra.sin_addr.s_addr = inet_addr(ZK_DEVICE_IP_ADDRESS);
		ra.sin_port = htons(ZK_DEVICE_PORT_NUM);


		if(connect(socket_fd,(struct sockaddr*)&ra,sizeof( struct sockaddr)) < 0)
		{
			printf("connect failed \n");
			close(socket_fd);
			return ZK_ERROR;
		}
		return ZK_OK;
}
	
ZK_ErrorCode SocketClose(void)
{
	close(socket_fd);
	return ZK_OK;
}

int ZK_send(void * data,unsigned int size)
{
	return send(socket_fd,data,size,0);
}

int ZK_recv(void * data,unsigned int size,int timeout)
{
	int recv_data;
	struct timeval Socket_timeout;
	
	if(timeout == 0){
		Socket_timeout.tv_sec = 35 ;
		Socket_timeout.tv_usec = 0;
	}
	else{
		Socket_timeout.tv_sec = timeout ;
		Socket_timeout.tv_usec = 0;
	}
  lwip_setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &Socket_timeout, sizeof(Socket_timeout));
	recv_data=recv(socket_fd,data,size,0);
	
	return recv_data;
}