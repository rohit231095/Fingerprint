#pragma once

#define SEND_BLOCK_SIZE (1020)

#define DEFAULT_PORT 627
#define DEFAULT_PROTO SOCK_STREAM // SOCK_DGRAM // TCP

#define BLOCK_SIZE			1020

#define COMM_MODE_TCP	0
#define COMM_MODE_COM1	1
#define COMM_MODE_COM2	2
#define COMM_MODE_USB   3
#define COMM_MODE_SERVER 4

//////////////////////////////////////////////////////////////////////////

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

//////////////////////////////////////////////////////////////////////////

#define CMDSIZE	        16
#define ACKSIZE	        8
#define EXERETSIZE	    14

//--- for Transfer Protocol ---
#define STX1				0x55
#define STX2				0xAA
#define STX3				0x5A
#define STX4				0xA5

#define CMD_ANS_ACK			1
#define CMD_ANS_NAK			0

#define EXE_RES_TRUE		1
#define EXE_RES_FALSE		0

#pragma pack(1)
typedef struct tagCOMMAND {
	BYTE 	Head1;		// 55=STX1
	BYTE 	Head2;		// AA=STX2
	WORD    MachineID;
	WORD    Reserved;
	WORD 	Command;
	DWORD	Length;		//SettingValue
    WORD	InParam;
	WORD 	ChkSum;
} CMDPKT;               // 14BYTE  

typedef struct tagCOMMAND_ACK {
	BYTE 	Head1;		// 5A=STX1
	BYTE 	Head2;		// A5=STX2
	WORD    MachineID;
    WORD	Response;
	WORD 	ChkSum;
} ACKPKT;               // 8Byte  

//Response = 0x02  (NAK)
//           0x03  (ACK)

typedef struct tagCOMMAND_RES {
	BYTE 	Head1;		// AA=STX1
	BYTE 	Head2;		// 55=STX2
	WORD    MachineID;
	WORD    Reserved;
    WORD	Ret;		// 1: OK 0: Error
	DWORD	OutParam;	// Result or ErrorCode
	WORD 	ChkSum;
} RESULTPKT;            // 12Byte
#pragma pack()

//////////////////////////////////////////////////////////////////////////
enum
{
	CMD_INTERNAL_CHECK_LIVE = 0x51,
	CMD_INTERNAL_CHECK_PWD
};
