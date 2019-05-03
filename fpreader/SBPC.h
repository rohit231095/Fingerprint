// SBPC.h: interface for the CSBPC class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "_Globals.h"
#include "UsbCom.h"

class CSBPC  
{
public:
	WORD m_MachineID;
	CSBPC();
	virtual ~CSBPC();
	CUsb1Com m_Usb1Port;
	DWORD	m_ComBuadrate;
	void SetCommMode ( BOOL bMode );
	BOOL StartComm ();
	void ExitComm ();
	void ExitCommRst ();
	int RecExeResult( WORD * apResult, DWORD * apOutParam );
	int RecCommandAckNak( void );
	int SendCommand( WORD aCmd, WORD aInParam, DWORD aTransSize );
	int RecData(void* pData, DWORD dwSize );
	int SendData(void* pData, DWORD dwSize );
	int RecBigData(void* pData, DWORD dwSize );
	int SendBigData(void* pData, DWORD dwSize );
	int RecBmpData(void* pData, DWORD dwSize );
};
