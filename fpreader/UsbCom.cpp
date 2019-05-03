// Com.cpp: implementation of the CUsb1Com class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "_Globals.h"
#include "UsbCom.h"
#include <io.h>
#include <fcntl.h>
#include <winioctl.h>
#include <math.h>


#include <objbase.h>
#include <initguid.h>	
#include <setupapi.h>
// requires to link with setupapi.lib
// Link with SetupAPI.Lib.
#pragma comment (lib, "setupapi.lib")

//无驱设备.............................
#include "ntddscsi.h"
#include "spti.h" 



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define CMD_READ 0x85
#define CMD_WRITE 0x86
#define CMD_JUDGEDISK 0x87


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUsb1Com::CUsb1Com()
{
	memset(&syncRead,0,sizeof(syncRead));
	memset(&syncWrite,0,sizeof(syncWrite));
	m_bConnect = FALSE;m_dwWaitThreadID = 0;
}
CUsb1Com::~CUsb1Com()
{
	if(m_bConnect) CloseComm();
	CloseHandle(syncRead.hEvent);
	CloseHandle(syncWrite.hEvent);
}

BOOL CUsb1Com::OpenComm( )
{
	if(OpenUDiskEx(&hUDisk,0)==0)
	{
	   m_bConnect = TRUE;
       return TRUE;
	 }
	else 
	 return FALSE;
}

void CUsb1Com::CloseComm()
{
    if ( m_bConnect && hUDisk != NULL && hUDisk != INVALID_HANDLE_VALUE ){
		
		CloseUDiskEx(hUDisk);
		hUDisk = INVALID_HANDLE_VALUE;
		m_bConnect = FALSE ;
		
		mIndex=-1;
   }
  
}
int CUsb1Com::SendData(unsigned char *buf, int Size )
{
	int ReadLen;
	ULONG mLen=0;
	int dwBytesWritten ;
	
	
	if ( m_bConnect == FALSE ) return -1;
	unsigned char*	pvBuff = buf;

	//OpenComm();
	
	dwBytesWritten = 0;
	ReadLen=Size;
	////////////////////////////
	while(1)
	{
		if(ReadLen < 4096)
		{
			mLen = ReadLen;	
			if ( UDiskSendData(hUDisk, pvBuff,mLen,2000) )  
			{
				
				dwBytesWritten+=mLen;
				Sleep(2);
				break;
			}
			else
			{
				break;
			}
		}
		else
		{
			mLen = 4096;	
			if ( UDiskSendData(hUDisk, pvBuff,mLen,2000))  
			{
				ReadLen-=mLen;
				dwBytesWritten+=mLen;
				pvBuff+=mLen;
				Sleep(2);
			}
			else
			{
				break;
			}
		}
	}
	if ( dwBytesWritten == 0 )
	{
		return -1;
	}
	return dwBytesWritten;
}


int CUsb1Com::ReadData(unsigned char *buf, int Size )
{	
	if ( m_bConnect == FALSE ) return -1;
	
	if ( UDiskRevData(hUDisk, buf,(ULONG *)&Size,2000) )
	{
	   return Size;
	}
	else
	{
		return -1;
	}
}

//无驱设备序列号 DEVICE_UDisk

GUID  GUIDUsbDevice = {0x53f56307L, 0xb6bf, 0x11D0, {0x94, 0xF2, 0x00, \
0xa0, 0xc9, 0x1e, 0xfb, 0x8b}};
int  CUsb1Com::OpenUDiskEx(HANDLE* pHandle,int nDevNum)
{ 	
	if(nDevNum<0)
		return -1;
	//-----------第一步,尝试通过查找可移动盘符来判断是否存在设备----------------------
    CHAR strDriver[25]; 
	int Driver,iDev=0;
	BYTE SN[13]={0};
	CString strPath;

	//当打开第一个设备时，把打开的设备序列号清空
	iDev = 0;//设备计数从0开始
	for (Driver='C';Driver<='Z';Driver++) 
	{//循环检测A～Z 
		strPath.Format("%c:",Driver);
		int type = GetDriveType(strPath); 
		if(type==DRIVE_REMOVABLE || type==DRIVE_CDROM)//可移动类型磁盘
		{ 
			sprintf(strDriver,"\\\\.\\%c:",Driver);	
			hUDisk = CreateFile(strDriver,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);	
			if(hUDisk==INVALID_HANDLE_VALUE)
				continue;
			if(JudgeDisk(hUDisk)==TRUE)
			{
				if(nDevNum==iDev)//............成功! 保存信息，然后返回
				{

					return (hUDisk==INVALID_HANDLE_VALUE ? -1 : 0);
				}
				else
					iDev++;
			}
			CloseHandle(hUDisk);
			hUDisk= INVALID_HANDLE_VALUE;
		} 	
	}
	
	//----------第二步,如果程序所在盘符不是我们的U盘,则检查是否有U盘插上-----------
	//---下面的代码会导致不能正常删除U盘-----------
	iDev = 0;
	//得到所有属于u盘设备类的设备列表
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUIDUsbDevice, NULL, NULL,(DIGCF_PRESENT | DIGCF_INTERFACEDEVICE)); 
	if(!hDevInfo)
		return -1;
    SP_INTERFACE_DEVICE_DATA DevInfoData;
    DevInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);
    int DeviceNo = 0;
    SetLastError(NO_ERROR);
    while(GetLastError() != ERROR_NO_MORE_ITEMS)
    {
		// 列举成员设备的驱动程序信息
        if(SetupDiEnumInterfaceDevice (hDevInfo,0, &GUIDUsbDevice,DeviceNo,&DevInfoData))
        {
            // get details about this device
            ULONG  DevDetailLen = 0;
            SetupDiGetInterfaceDeviceDetail(hDevInfo,&DevInfoData,NULL, 0,&DevDetailLen,NULL); // not interested in the specific dev-node
            PSP_INTERFACE_DEVICE_DETAIL_DATA pDevDetail = (SP_INTERFACE_DEVICE_DETAIL_DATA*)malloc(DevDetailLen);
            pDevDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
            if(! SetupDiGetInterfaceDeviceDetail(hDevInfo,&DevInfoData,pDevDetail,DevDetailLen,NULL,NULL)) 
			{
			//	HidD_GetAttributes()
				free(pDevDetail);
				SetupDiDestroyDeviceInfoList(hDevInfo);
				return -1;
			}
            // open a file to this device		
			hUDisk = CreateFile(pDevDetail->DevicePath,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, // no SECURITY_ATTRIBUTES structure
				OPEN_EXISTING, // No special create flags
				0, // to allow access to USB interrupt reports
				NULL); // No template file
            free(pDevDetail);
			++DeviceNo;//检查下一个设备.
			if(JudgeDisk(hUDisk)==TRUE)//是我们的U盘.
			{
				if(nDevNum==iDev)//............成功! 保存信息，然后返回
				{

					SetupDiDestroyDeviceInfoList(hDevInfo);
					return (hUDisk==INVALID_HANDLE_VALUE ? -1 : 0);
				}
				else
					iDev++;
			}				 
			CloseHandle(hUDisk);
			hUDisk=INVALID_HANDLE_VALUE;
        }
	} 
    SetupDiDestroyDeviceInfoList(hDevInfo);//*/
	return -1; 	
}
int  CUsb1Com::CloseUDiskEx(HANDLE hHandle)
{
	if(hHandle==NULL || hHandle==INVALID_HANDLE_VALUE)
		return 1;

	if(hHandle!=NULL && hHandle!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hHandle);
		hHandle = INVALID_HANDLE_VALUE;
	} 
	return 0;
}
BOOL CUsb1Com::JudgeDisk(HANDLE hUDev)
{
    BYTE pData[32];
	BOOL status = 0;  
	
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    ULONG length = 0,           
		returned = 0,
		sectorSize = 512;
	if (hUDev == INVALID_HANDLE_VALUE) 
		return -1;
	ULONG TransLen=0;
    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));     
	
    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.SenseInfoLength = 0;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.DataTransferLength = 2;
    sptdwb.sptd.TimeOutValue = 2000;
    sptdwb.sptd.DataBuffer = pData;
    sptdwb.sptd.SenseInfoOffset =
		offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    sptdwb.sptd.Cdb[0] = CMD_JUDGEDISK;    

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(hUDev,
		IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&sptdwb,
		length,
		&sptdwb,
		length,
		&returned,
		FALSE);	 
	if(!status)
	  return FALSE;
	if(pData[0]!=0x55 || pData[1]!=0x33)
		return FALSE;
	return TRUE;
}

DWORD CUsb1Com::GetUDiskNums()	//use for UDisk	; UDisk  设备数
{ 	
	HANDLE hUDisk = INVALID_HANDLE_VALUE;
	DWORD numDevices = 0;

	//-----------第一步,尝试通过查找可移动盘符来判断是否存在设备----------------------
    CHAR strDriver[25]; 
	int Driver,iDev=0;
	BYTE SN[13]={0};
	CString strPath;

	//当打开第一个设备时，把打开的设备序列号清空
	for (Driver='C';Driver<='Z';Driver++) 
	{//循环检测A～Z 
		strPath.Format("%c:",Driver);
		int type = GetDriveType(strPath); 
		if(type==DRIVE_REMOVABLE || type==DRIVE_CDROM)//可移动类型磁盘
		{ 
			sprintf(strDriver,"\\\\.\\%c:",Driver);	
			hUDisk = CreateFile(strDriver,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);	
			if(hUDisk==INVALID_HANDLE_VALUE)
				continue;
			if(JudgeDisk(hUDisk)==TRUE)
				numDevices++;//设备数加1
			CloseHandle(hUDisk);
			hUDisk= INVALID_HANDLE_VALUE;
		} 	
	}
	
	//----------第二步,如果程序所在盘符不是我们的U盘,则检查是否有U盘插上-----------
	//---下面的代码会导致不能正常删除U盘-----------
	//得到所有属于u盘设备类的设备列表
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUIDUsbDevice, NULL, NULL,(DIGCF_PRESENT | DIGCF_INTERFACEDEVICE)); 
	if(!hDevInfo)
		return numDevices;
    SP_INTERFACE_DEVICE_DATA DevInfoData;
    DevInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);
    int DeviceNo = 0;
    SetLastError(NO_ERROR);
    while(GetLastError() != ERROR_NO_MORE_ITEMS)
    {
		// 列举成员设备的驱动程序信息
        if(SetupDiEnumInterfaceDevice (hDevInfo,0, &GUIDUsbDevice,DeviceNo,&DevInfoData))
        {
            // get details about this device
            ULONG  DevDetailLen = 0;
            SetupDiGetInterfaceDeviceDetail(hDevInfo,&DevInfoData,NULL, 0,&DevDetailLen,NULL); // not interested in the specific dev-node
            PSP_INTERFACE_DEVICE_DETAIL_DATA pDevDetail = (SP_INTERFACE_DEVICE_DETAIL_DATA*)malloc(DevDetailLen);
            pDevDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
            if(! SetupDiGetInterfaceDeviceDetail(hDevInfo,&DevInfoData,pDevDetail,DevDetailLen,NULL,NULL)) 
			{
			//	HidD_GetAttributes()
				free(pDevDetail);
				SetupDiDestroyDeviceInfoList(hDevInfo);
				return numDevices;
			}
            // open a file to this device		
			hUDisk = CreateFile(pDevDetail->DevicePath,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, // no SECURITY_ATTRIBUTES structure
				OPEN_EXISTING, // No special create flags
				0, // to allow access to USB interrupt reports
				NULL); // No template file
            free(pDevDetail);
			++DeviceNo;//检查下一个设备.
			if(JudgeDisk(hUDisk)==TRUE)//是我们的U盘.
				numDevices++;				 
			CloseHandle(hUDisk);
			hUDisk=INVALID_HANDLE_VALUE;
        }
	} 
    SetupDiDestroyDeviceInfoList(hDevInfo);//*/
	return numDevices; 	
}

//UDisk通讯
BOOL CUsb1Com::UDiskSendData(HANDLE hHandle, BYTE* pData,ULONG nLength,INT nTimeOut/*=2000*/)
{//PS_COMM_ERR	PS_OK
	BOOL status = 0; 
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    ULONG length = 0,           
		returned = 0,
		sectorSize = 512;
	if (hHandle == INVALID_HANDLE_VALUE) 
		return FALSE;

	if(nTimeOut<2000)
		nTimeOut = 2000;

	ULONG TransLen=nLength;
    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));     
    sptdwb.sptd.Length  = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId  = 0;
    sptdwb.sptd.TargetId= 1;
    sptdwb.sptd.Lun	    = 0;
    sptdwb.sptd.CdbLength	= CDB10GENERIC_LENGTH;
    sptdwb.sptd.SenseInfoLength = 0;
    sptdwb.sptd.DataIn	= SCSI_IOCTL_DATA_OUT;
    sptdwb.sptd.DataTransferLength = TransLen;
    sptdwb.sptd.TimeOutValue	= nTimeOut;
    sptdwb.sptd.DataBuffer		= pData;
    sptdwb.sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    sptdwb.sptd.Cdb[0] = CMD_WRITE;    

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(hUDisk,
		IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&sptdwb,
		length,
		&sptdwb,
		length,
		&returned,
		FALSE);//1-OK	 
	if(status)
		return TRUE;
	return FALSE;
}
BOOL CUsb1Com::UDiskRevData(HANDLE hHandle, BYTE* pData,ULONG* pnLength,INT nTimeout/*=2000*/)
{
	BOOL status = 0;  
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    ULONG length = 0,           
		returned = 0,
		sectorSize = 512;
	ULONG TransLen;
	if(nTimeout<2000)
		nTimeout = 2000;
	if (hHandle == INVALID_HANDLE_VALUE) 
		return FALSE;
	
    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));      	
	TransLen=*pnLength;	
    if(TransLen==1) 
		TransLen=2;
    sptdwb.sptd.Length		= sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId		= 0;
    sptdwb.sptd.TargetId	= 1;
    sptdwb.sptd.Lun			= 0;
    sptdwb.sptd.CdbLength	= CDB10GENERIC_LENGTH;
    sptdwb.sptd.SenseInfoLength = 0;
    sptdwb.sptd.DataIn			= SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.DataTransferLength =TransLen;	
    sptdwb.sptd.TimeOutValue	= nTimeout;
    sptdwb.sptd.DataBuffer		= pData;
    sptdwb.sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    sptdwb.sptd.Cdb[0]			= CMD_READ;
	
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(hUDisk,
		IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&sptdwb,
		length,
		&sptdwb,
		length,
		(ULONG *)pnLength,
		FALSE);//1-OK
	*pnLength=sptdwb.sptd.DataTransferLength;
	
	if(status)//OK
		return TRUE;
	return FALSE;
}