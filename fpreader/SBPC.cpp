// SBPC.cpp: implementation of the CSBPC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SBPC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BYTE	bRemoteEnroll = 0;

CSBPC::CSBPC()
{
	 m_MachineID=1;
}

CSBPC::~CSBPC()
{

}

BOOL CSBPC::StartComm()
{

	if ( m_Usb1Port.OpenComm() == TRUE )
		{
			return TRUE;
		}
	return FALSE;
	
}

void CSBPC::ExitComm()
{

   m_Usb1Port.CloseComm();

}

void CSBPC::ExitCommRst()
{

m_Usb1Port.CloseComm();

}

int CSBPC::SendCommand(WORD aCmd, WORD aInParam, DWORD aTransSize)
{

	DWORD vSum;
	CMDPKT	vCMD;
	int i;
	int vRet = -1;

	vCMD.Head1 = STX1;
	vSum = vCMD.Head1;

	vCMD.Head2 = STX2;
	vSum += (WORD)vCMD.Head2;

	vCMD.MachineID = m_MachineID;
	vSum += HIBYTE(vCMD.MachineID); vSum += LOBYTE(vCMD.MachineID);

	vCMD.Reserved = 0x1979;	
	vSum += LOBYTE(vCMD.Reserved); vSum += HIBYTE(vCMD.Reserved);
	
	vCMD.Command = aCmd;
	vSum += HIBYTE(vCMD.Command); vSum += LOBYTE(vCMD.Command);

	vCMD.Length = aTransSize;
	vSum += HIBYTE(HIWORD(vCMD.Length)); vSum += LOBYTE(HIWORD(vCMD.Length));
	vSum += HIBYTE(LOWORD(vCMD.Length)); vSum += LOBYTE(LOWORD(vCMD.Length));

	vCMD.InParam = aInParam;
	vSum += HIBYTE(vCMD.InParam); vSum += LOBYTE(vCMD.InParam);

	vCMD.ChkSum = (WORD)vSum;

	i = 1;
	

	while( i-- > 0 )
	{

		vRet = m_Usb1Port.SendData( (LPBYTE)&vCMD, CMDSIZE );
		if( vRet != sizeof(vCMD) ){
			vRet = -1;
			continue;
			}
		vRet = RecCommandAckNak();
		if ( vRet == 1 ) break;
	}

	return vRet;
}

int CSBPC::RecCommandAckNak(void)
{
	DWORD vSum;
	ACKPKT	vACK;
	int vRet;
	DWORD ST = GetTickCount();

	while( GetTickCount() - ST <500)
	{
		vRet = m_Usb1Port.ReadData( (LPBYTE)&vACK, sizeof(vACK) );	

		if( vRet != sizeof(vACK) )
		{
			vRet = -1;
			break;
		}

		if( vACK.Head1 != STX3 )
		{
			vRet = -1;
			break;
		}
		vSum = vACK.Head1;

		if( vACK.Head2 != STX4 )
		{
			vRet = -1;
			break;
		}
		vSum += vACK.Head2;

		if( vACK.MachineID != m_MachineID )
		{
			vRet = -1;
			break;
		}
		vSum += HIBYTE(vACK.MachineID); vSum += LOBYTE(vACK.MachineID);
		
		vSum += vACK.Response;

		if( vACK.ChkSum != vSum )
		{
			vRet = -1;
			break;
		}

		if( vACK.Response == CMD_ANS_ACK )
			vRet = 1;
		else
			vRet = 0;

		break;
	}

	return vRet;
}

int CSBPC::RecExeResult(WORD *apResult, DWORD *apOutParam)
{
	DWORD vSum;
	RESULTPKT	vEXE;
	int vRet;
	unsigned int WaitTime;
	DWORD ST = GetTickCount();

	WaitTime=20000;
	while( GetTickCount() - ST < WaitTime )
	{
		if( apResult != NULL )
			*apResult = 0;
		if( apOutParam != NULL )
			*apOutParam = 0;
		
	    vRet = m_Usb1Port.ReadData( (LPBYTE)&vEXE, sizeof(vEXE) );
		
		if( vRet != sizeof(vEXE) )
			return -1;

		if( vEXE.Head1 != STX2 )
			return -1;
		vSum = vEXE.Head1;

		if( vEXE.Head2 != STX1 )
			return -1;
		vSum += vEXE.Head2;

		if( vEXE.MachineID != m_MachineID )
			return -1;
		
		vSum += HIBYTE(vEXE.MachineID); vSum += LOBYTE(vEXE.MachineID);
		vSum += HIBYTE(vEXE.Reserved); vSum += LOBYTE(vEXE.Reserved);
		
		if( apResult != NULL )
			*apResult = vEXE.Ret;
		vSum += HIBYTE(vEXE.Ret); vSum += LOBYTE(vEXE.Ret);

		if( apOutParam != NULL )
			*apOutParam = vEXE.OutParam;
		vSum += HIBYTE(HIWORD(vEXE.OutParam)); vSum += LOBYTE(HIWORD(vEXE.OutParam));
		vSum += HIBYTE(LOWORD(vEXE.OutParam)); vSum += LOBYTE(LOWORD(vEXE.OutParam));

		if( vEXE.ChkSum != vSum )
			return -1;
		else
			return 1;
	}
	return -1;
}

int CSBPC::RecData(void* pData, DWORD dwSize )
{
	int nRet, i;
	WORD CheckSum;
	int RecSize = dwSize + 6;
	BYTE*	pRecBuff = new BYTE[RecSize];
	DWORD ST = GetTickCount();

	if ( pRecBuff == NULL ) return -1;

	while( GetTickCount() - ST <5000 )
	{

		nRet = m_Usb1Port.ReadData( pRecBuff, RecSize );
		
		if ( nRet != RecSize )
			break;

		if( pRecBuff[0] != STX4 ) break;
		CheckSum = pRecBuff[0];

		if( pRecBuff[1] != STX3 ) break;
		CheckSum += pRecBuff[1];

		if( m_MachineID != *(WORD*)&pRecBuff[2] ) break;
		CheckSum += pRecBuff[2];
		CheckSum += pRecBuff[3];

		for ( i = 4; i < RecSize - 2; i++ ){
			CheckSum += pRecBuff[i];
		}

		if( CheckSum != *(WORD*)&pRecBuff[RecSize - 2] )
			break;

		memcpy ( pData, pRecBuff + 4, dwSize );
		delete pRecBuff;
		return 1;
	}
	delete pRecBuff;
	return -1;
}

int CSBPC::SendData(void* pData, DWORD dwSize )
{
	int nRet;
	DWORD i;
	WORD CheckSum;
	int SendSize = dwSize + 6;
	BYTE*	pSendBuff = new BYTE[SendSize];

	if ( pSendBuff == NULL ) return -1;

	pSendBuff[0] = STX3;
	CheckSum = pSendBuff[0];

	pSendBuff[1] = STX4;
	CheckSum += pSendBuff[1];

	pSendBuff[2] = LOBYTE(m_MachineID);
	CheckSum += pSendBuff[2];
	pSendBuff[3] = HIBYTE(m_MachineID);
	CheckSum += pSendBuff[3];

	memcpy( pSendBuff + 4, pData, dwSize );
	for ( i = 0; i < dwSize; i++ ) CheckSum += pSendBuff[i+4];
	*(WORD*)&pSendBuff[4+dwSize] = CheckSum;

	nRet = m_Usb1Port.SendData( pSendBuff, SendSize );
	if ( nRet != SendSize ){
		delete pSendBuff;
		return -1;
	}
	delete pSendBuff;
	return 1;
}

int CSBPC::RecBigData(void* pData, DWORD dwSize )
{
	DWORD nRet, i;
	WORD CheckSum;
	BYTE*	pRecBuff = new BYTE[BLOCK_SIZE+6];
//	DWORD ST = GetTickCount();
	BYTE*	pPoint = (BYTE*)pData;
	DWORD Bytes, Remain = dwSize;

//	if ( m_bCommMode == COMM_MODE_TCP ) return RecData( pData, dwSize );
	if ( pRecBuff == NULL ) return -1;

	Bytes = dwSize;
	while( Remain > 0 )
	{
		if ( Remain > BLOCK_SIZE )
			Bytes = BLOCK_SIZE;
		else
			Bytes = Remain;
		
		nRet = m_Usb1Port.ReadData( pRecBuff, Bytes + 6);
		
		if ( nRet != Bytes + 6){
			delete pRecBuff;
			return -1;
		}

		if( pRecBuff[0] != STX3 ) {
			delete pRecBuff;
			return -1;
		}
		CheckSum = pRecBuff[0];

		if( pRecBuff[1] != STX4 ) {
			delete pRecBuff;
			return -1;
		}
		CheckSum += pRecBuff[1];

		if( m_MachineID != *(WORD*)&pRecBuff[2] ) break;
		CheckSum += pRecBuff[2];
		CheckSum += pRecBuff[3];

		for ( i = 0; i < Bytes; i++ ){
			CheckSum += pRecBuff[i+4];
		}

		if( CheckSum != *(WORD*)&pRecBuff[Bytes + 4] ){
			delete pRecBuff;
			return -1;
		}

		memcpy ( pPoint, pRecBuff + 4, Bytes );
		pPoint += Bytes;
		Remain -= Bytes;
	}
	delete pRecBuff;
	return 1;
}

int CSBPC::RecBmpData(void* pData, DWORD dwSize )
{
	DWORD nRet;
	BYTE*	pPoint = (BYTE*)pData;
	DWORD Bytes, Remain = dwSize;

	Bytes = dwSize;
	while( Remain > 0 )
	{
		if ( Remain > 32768 )
			Bytes = 32768;
		else
			Bytes = Remain;
		
		nRet = m_Usb1Port.ReadData(pPoint,Bytes);		
		pPoint += Bytes;		
		Remain -= Bytes;
	}	
	return 1;
}

int CSBPC::SendBigData(void* pData, DWORD dwSize )
{
	int nRet;
	DWORD i;
	WORD CheckSum;
	BYTE*	pSendBuff = new BYTE[BLOCK_SIZE + 6];
	BYTE*	pPoint = (BYTE*)pData;
	DWORD	Remain = dwSize, Bytes;

//	if ( m_bCommMode == COMM_MODE_TCP ) return SendData( pData, dwSize );
	if ( pSendBuff == NULL ) return -1;
	while ( Remain > 0 ){
		if ( Remain > BLOCK_SIZE )
			Bytes = BLOCK_SIZE;
		else
			Bytes = Remain;
		
		pSendBuff[0] = STX3;
		CheckSum = pSendBuff[0];

		pSendBuff[1] = STX4;
		CheckSum += pSendBuff[1];

		pSendBuff[2] = LOBYTE(m_MachineID);
		CheckSum += pSendBuff[2];
		pSendBuff[3] = HIBYTE(m_MachineID);
		CheckSum += pSendBuff[3];

		memcpy( pSendBuff + 4, pPoint, Bytes );
		for ( i = 0; i < Bytes; i++ ) CheckSum += pSendBuff[i+4];
		*(WORD*)&pSendBuff[4+Bytes] = CheckSum;		
	    nRet = m_Usb1Port.SendData( pSendBuff, Bytes + 6 );
		if ( nRet != (int)Bytes + 6 ){
			delete pSendBuff;
			return -1;
		}
		pPoint += Bytes;
		Remain -= Bytes;
	}
	delete pSendBuff;
	return 1;
}