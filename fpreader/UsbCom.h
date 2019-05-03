// Com.h: interface for the CCom class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

class CUsb1Com  
{
public:
	CUsb1Com();
	virtual ~CUsb1Com();
	int SendData(unsigned char *buf, int Size);
	int ReadData(unsigned char *buf, int Size );
	void CloseComm();
	BOOL OpenComm();
	
private:
	BOOL m_bConnect;
	OVERLAPPED syncRead,syncWrite;
	byte m_bufInput[3000];
	byte m_bufOutput[3000];
	byte m_bufOutputEx[3000];
	DWORD m_dwWaitThreadID;
	HANDLE m_hWaitThread;
    HANDLE hUDisk;
	ULONG mIndex;

	int  OpenUDiskEx(HANDLE* pHandle,int nDevNum);
	int  CloseUDiskEx(HANDLE hHandle);
	BOOL JudgeDisk(HANDLE hUDev);
	DWORD GetUDiskNums();	//use for UDisk	; UDisk  Éè±¸Êý
	BOOL UDiskSendData(HANDLE hHandle, BYTE* pData,ULONG nLength,INT nTimeOut/*=2000*/);
	BOOL UDiskRevData(HANDLE hHandle, BYTE* pData,ULONG* pnLength,INT nTimeout/*=2000*/);
};
