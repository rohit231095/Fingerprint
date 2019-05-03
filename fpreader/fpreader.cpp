// fpreader.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "fpreader.h"
#include "SBPC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma comment(dll,"FpMatch.dll") 

short __stdcall CharMatch(unsigned char* Src,unsigned char* Dst); //eAlgDLL.dll PC比对算法库 1000枚算法库

////////////////////////////////////////////////////////////////////////
//	Operation
//		Match two Templates.
//	Parameters
//		pbQuery[IN]			:	Query Template
//		pbRegX[IN]			:	Registered Template
//		nLevel[IN]			:	Security level (1 ~ 5)
//		pnSimilarity[OUT]	:	matched if 1, unmatched if 0
//	Return
//		if 0 ok, otherwise fail.
////////////////////////////////////////////////////////////////////////
//int FPMatch(unsigned char*	pbQuery,unsigned char*	pbRegX,	int	nLevel,	int	*pnSimilarity);


//////////////////////////////////////////3000
////////////////////////////////////////////////////////////////////////
//	Operation
//		Match two Templates.
//	Parameters
//		pbQuery[IN]			:	Query Template
//		pbRegX[IN]			:	Registered Template
//		nLevel[IN]			:	Security level (1 ~ 5)
//		pnSimilarity[OUT]	:	matched if 1, unmatched if 0
//	Return
//		if 0 ok, otherwise fail.
////////////////////////////////////////////////////////////////////////
/*
   int
FPMatch(
	unsigned char*	pbQuery
,	unsigned char*	pbRegX
,	int				nLevel
,	int				*pnSimilarity
);

*/
///////////////////////////
//int WINAPI FPMatch(unsigned char*	pbQuery,unsigned char*	pbRegX,	int	nLevel,	int	*pnSimilarity); //Fpmatch.dll PC比对算法库 3000枚算法库

HINSTANCE hDllInst;

typedef int ( *FpFun)(unsigned char*	pbQuery,unsigned char*	pbRegX,int nLevel,int *pnSimilarity);

FpFun mFPMatch = NULL;

CSBPC gDev;



#define FP_SIZE				1420

#define MB_SIZE             512

#define TH30_MB_SIZE        818

#define EXE_RES_TRUE		1
#define EXE_RES_FALSE		0
#define EXE_RES_NONE		2

long  gLastError;

long version=0;

enum 
{
	ERR_SUCCESS = 0,
	ERR_COMPORT_ERROR,
	ERR_WRITE_FAIL,
	ERR_READ_FAIL,
	ERR_NO_FP,
	ERR_NO_CARD
};
enum COMMAND_NDX{
    CMD_OnOffLed=0x108,
	CMD_GetVersion,
    CMD_DetectFinger,
	CMD_GetTemplet,
	CMD_MergeChar,	
	CMD_GetImage,
	CMD_DetectCard,
	CMD_SETMONITORMODE
};

/////////////////////////////////////////////////////////////////////////////
// CFPDEMODlg dialog
typedef struct tagENR_DATA  
{
	BYTE	Valid;
	BYTE	Manager;
	BYTE    Threshold;
	BYTE	FingerNum;
	DWORD	ID;
} ENR_DATA;		/* 8byte */

typedef struct _FP_ENROLL_STRUCT
{
	BYTE	    cCharInfoTab[244];	  /*特征信息表*/
	ENR_DATA    UserInfo;
	DWORD	    Reserve;		/*备用*/
}FP_CHAR_ENROLL, *pFP_CHAR_ENROLL;

typedef struct _FP_MB_ENROL_STRUCT
{
  FP_CHAR_ENROLL CharFile1;
  FP_CHAR_ENROLL CharFile2;
}FP_MB_ENROLL,*pFP_MB_ENROLL;


typedef struct _TH30FP_MB_ENROL_STRUCT
{
  ENR_DATA    UserInfo;
  BYTE CharFile1[810];
}TH30FP_MB_ENROLL,*pTH30FP_MB_ENROLL;

FP_MB_ENROLL gTHFPBuf;

TH30FP_MB_ENROLL gTH30FPBuf;

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CFpreaderApp

BEGIN_MESSAGE_MAP(CFpreaderApp, CWinApp)
	//{{AFX_MSG_MAP(CFpreaderApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFpreaderApp construction

CFpreaderApp::CFpreaderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	 hDllInst= LoadLibrary("FpMatch.dll");
	if(hDllInst)
	{
	   mFPMatch = (FpFun)GetProcAddress(hDllInst,"FPMatch");
	  
	}
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CFpreaderApp object

CFpreaderApp theApp;

//////////////////////////////
//生成图象文件
BOOL MakeBMP(LPCTSTR LPSZFileName,unsigned char* pImage,int X,int Y)
{
	unsigned int num;
	int i,j;
	CFile f;	
	unsigned char head[1078]={
							/***************************/
							//file header
							0x42,0x4d,//file type 
							//0x36,0x6c,0x01,0x00, //file size***
							0x0,0x0,0x0,0x00, //file size***
							0x00,0x00, //reserved
							0x00,0x00,//reserved
							0x36,0x4,0x00,0x00,//head byte***
							/***************************/
							//infoheader
							0x28,0x00,0x00,0x00,//struct size

							//0x00,0x01,0x00,0x00,//map width*** 
							0x00,0x00,0x00,0x00,//map width*** 
							//0x68,0x01,0x00,0x00,//map height***
							0x00,0x00,0x00,0x00,//map height***

							0x01,0x00,//must be 1
							0x08,0x00,//color count***
							0x00,0x00,0x00,0x00, //compression
							//0x00,0x68,0x01,0x00,//data size***
							0x00,0x00,0x00,0x00,//data size***
							0xE5,0x4C,0x00,0x00,//dpix
							0xE5,0x4C,0x00,0x00, //dpiy
							0x00,0x00,0x00,0x00,//color used
							0x00,0x00,0x00,0x00,//color important
											
						};
    if(!f.Open(_T(LPSZFileName),
		CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		return 0;
	}
	//确定图象宽度数值
	num=X; head[18]= num & 0xFF;
	num=num>>8;  head[19]= num & 0xFF;
	num=num>>8;  head[20]= num & 0xFF;
	num=num>>8;  head[21]= num & 0xFF;
	//确定图象高度数值
	num=Y; head[22]= num & 0xFF;
	num=num>>8;  head[23]= num & 0xFF;
	num=num>>8;  head[24]= num & 0xFF;
	num=num>>8;  head[25]= num & 0xFF;
	
	//确定调色板数值
	j=0;
	for (i=54;i<1078;i=i+4)
	{
		head[i]=head[i+1]=head[i+2]=j; 
		head[i+3]=0;
		j++;
	}	
	
	//写入文件头
	f.Write(head,1078*sizeof(char));
	
	//写入图象数据
	for(  i=0;i<Y; i++ )
	{
		//f.Seek(0,1078*sizeof(char)+(Y-1-i)*X);
		//f.Write(pImage+i*X,X*sizeof(char));
		f.Write(pImage+(Y-1-i)*X,X*sizeof(char));
	}
	f.Close();	
	return TRUE;
}
inline   char*   ConvertBSTRToString(BSTR   pSrc) 
{ 
	if(!pSrc)   return   NULL; 
	
	//convert   even   embeded   NULL 
	DWORD   cb,cwch   =   ::SysStringLen(pSrc); 
	
	char   *szOut   =   NULL; 
	
	if(cb   =   ::WideCharToMultiByte(CP_ACP,   0,   
		pSrc,   cwch   +   1,   NULL,   0,   0,   0)) 
	{ 
		szOut   =   new   char[cb]; 
		if(szOut) 
		{ 
			szOut[cb   -   1]     =   '\0 '; 
			
			if(!::WideCharToMultiByte(CP_ACP,   0,   
				pSrc,   cwch   +   1,   szOut,   cb,   0,   0)) 
			{ 
				delete   []szOut;//clean   up   if   failed; 
				szOut   =   NULL; 
			} 
		} 
	} 
	
	return   szOut; 
}; 

BOOL _stdcall GetLastError(long* dwErrorCode) 
{
	*dwErrorCode = gLastError;
	return TRUE;
}

BOOL  _stdcall  ConnectUsb()
{
    gDev.ExitComm();
	gDev.m_MachineID = 1;
	return gDev.StartComm();

}
BOOL WINAPI  DisConnectUsb()
{
   gDev.ExitComm();
   return TRUE;
}
BOOL WINAPI Getalgorithm(long * versionindex, char * versionchar)
{
   WORD Result;
   DWORD OutData;
   char versionstr[30];

   memset(versionstr,0,30);
   
  if ( !gDev.SendCommand( CMD_GetVersion, 0, 0) ||
		 !gDev.RecExeResult( &Result, (DWORD*)&OutData ) ||
		 Result == EXE_RES_FALSE )
	{
		return FALSE;
	}  
	 if( !gDev.RecData( (void*)versionstr, 16))
 	{
		return FALSE;
 	}	 
  version=(int)OutData;
  *versionindex=(int)OutData;  

 // CString str(versionstr);	
  //*versionchar = str.AllocSysString();
  strcpy(versionchar,versionstr);
  return TRUE;
}

BOOL  WINAPI OnoffLED(long onoff)
{
    WORD Result;
	DWORD OutData;
   if ( !gDev.SendCommand( CMD_OnOffLed, 0, onoff) ||
		 !gDev.RecExeResult( &Result, (DWORD*)&OutData ) ||
		 Result == EXE_RES_FALSE )
	{
		return FALSE;
	}
    gLastError=ERR_SUCCESS;
    return TRUE;
   
}
BOOL  WINAPI  DetectFinger()
{
    WORD Result;
	DWORD OutData;
	if ( !gDev.SendCommand( CMD_DetectFinger, 0, 0))
		{
		   gLastError=ERR_READ_FAIL;
		   return FALSE;
		}
   if(!gDev.RecExeResult( &Result, (DWORD*)&OutData ))
		{
		   gLastError=ERR_READ_FAIL;
		   return FALSE;
		} 	
    if(Result == EXE_RES_FALSE )
	{
	    gLastError=ERR_NO_FP;
		return FALSE;
	}
    gLastError=ERR_SUCCESS;
    return TRUE;
}

BOOL  WINAPI  DetectCard(long* CardNum)
{
    WORD Result;
	DWORD CARDID=0;
	if ( !gDev.SendCommand(CMD_DetectCard, 0, 0))
		{
		   gLastError=ERR_READ_FAIL;
		   *CardNum=CARDID;
		   return FALSE;
		}
   if(!gDev.RecExeResult( &Result, (DWORD*)&CARDID ))
		{
		   gLastError=ERR_READ_FAIL;
		   *CardNum=CARDID;
		   return FALSE;
		} 	
    if(Result == EXE_RES_FALSE )
	{
	    gLastError=ERR_NO_CARD;
		*CardNum=CARDID;
		return FALSE;
	}

	*CardNum=CARDID;
    gLastError=ERR_SUCCESS;
    return TRUE;
}
BOOL   WINAPI GetTemplet(long step)
{
  WORD Result;
  DWORD OutData;
  int score=0;
  FP_CHAR_ENROLL *fpchar;

  if(version==0)
  	{
	  if(step==0)
	  	fpchar=&gTHFPBuf.CharFile1;
	  else if(step==1)
	  	fpchar=&gTHFPBuf.CharFile2;
	  
		if ( !gDev.SendCommand( CMD_GetTemplet, 0, 0) ||
			 !gDev.RecExeResult( &Result, (DWORD*)&OutData ) ||
			 Result == EXE_RES_FALSE )
		{
		    gLastError=ERR_READ_FAIL;
			return FALSE;
		}
		 if( !gDev.RecBigData( (void*)fpchar, 256))
	 	{
	 	    gLastError=ERR_READ_FAIL;
			return FALSE;
	 	}	

	  if(step==1)
	  	{
	  	   score=CharMatch((BYTE *)&gTHFPBuf.CharFile1,(BYTE *)&gTHFPBuf.CharFile2);
		   if(score<45)
			return FALSE;
	  	}
  	}
  else if(version==1)
  	{
  	    if ( !gDev.SendCommand( CMD_GetTemplet, 0, step) ||
			 !gDev.RecExeResult( &Result, (DWORD*)&OutData ) ||
			 Result == EXE_RES_FALSE )
		{
		    gLastError=ERR_READ_FAIL;
			return FALSE;
		}		
		 if( !gDev.RecBigData( (void*)&gTH30FPBuf, sizeof(TH30FP_MB_ENROLL)))
	 	{
	 	    gLastError=ERR_READ_FAIL;
			return FALSE;
	 	}	
  	}

  gLastError=ERR_SUCCESS;
  return TRUE;
}
BOOL  WINAPI GetImage(LPCTSTR LPSZFileName)
{
    unsigned char pimage[288][256];
	unsigned char pimage2[354][316];
	
	if ( !gDev.SendCommand( CMD_GetImage, 0, 0))
	{
	    gLastError=ERR_READ_FAIL;
		return FALSE;
	}
    if( !gDev.RecBmpData( (void*)&pimage[0][0], 256*288))
		 	{
		 	    gLastError=ERR_READ_FAIL;
				return FALSE;
		 	}

	int i;int j;
	memset(pimage2,0xff,316*354);
	for(j=0;j<288;j++)
		{
		  for(i=0;i<256;i++)
		  	{
		  	  pimage2[j+33][i+30]=pimage[j][i];
		  	}
		}

	if(!MakeBMP(LPSZFileName, &pimage2[0][0],316,354))
	 {
		return FALSE;
     }
   gLastError=ERR_SUCCESS;
    return TRUE;	
}
BOOL WINAPI GetMergeChar(char* dwEnrollData)
{
   char hexchar[6];
   char fpstring[2000];
   memset(fpstring,0,2000);
   int i=0;
   
   if(version==0)
  	{
	  BYTE *fpchar=(BYTE *)&gTHFPBuf;
	  for(i=0;i<512;i++)
	  	{
	  	    if(fpchar[i]==0)// code
	  	    {
	  	       BYTE *ptr=&fpchar[i+1];
			   int j=0;
			   while(*ptr==0&&i+j<511)
			   	{
			   	   ptr++;
				   j++;
			   	}
			   if(j>0)
			   	{
			   	  sprintf(hexchar,"(%d)",j+1);
				  strcat(fpstring,hexchar);
				  i+=(j);
			   	}
			    else
		    	{
		   	      sprintf(hexchar,"%02x",fpchar[i]);	
			      strcat(fpstring,hexchar);
		    	  
		    	}
	  	    }
	  	    else
	  	    {
		   	sprintf(hexchar,"%02x",fpchar[i]);	
			strcat(fpstring,hexchar);
	  	    }
	  	}	  
   	}
   else
   	{
   	   WORD Result;
	   DWORD OutData;
   	   if ( !gDev.SendCommand( CMD_MergeChar, 0, 0) ||
		 !gDev.RecExeResult( &Result, (DWORD*)&OutData ) ||
		 Result == EXE_RES_FALSE )
			{
			    gLastError=ERR_READ_FAIL;
				return FALSE;
			}
		 if( !gDev.RecBigData( (void*)&gTH30FPBuf, sizeof(TH30FP_MB_ENROLL)))
	 	{
	 	    gLastError=ERR_READ_FAIL;
			return FALSE;
	 	}	
		 ///////////////////////////////////////////////
		 for(i=0;i<810;i++)
		   {
		  	    if(gTH30FPBuf.CharFile1[i]==0)// code
		  	    {
		  	       BYTE *ptr=&gTH30FPBuf.CharFile1[i+1];
				   int j=0;
				   while(*ptr==0&&i+j<809)
				   	{
				   	   ptr++;
					   j++;
				   	}
				   if(j>0)
				   	{
				   	  sprintf(hexchar,"(%d)",j+1);
					  strcat(fpstring,hexchar);
					  i+=(j);
				   	}
				    else
			    	{
			   	      sprintf(hexchar,"%02x",gTH30FPBuf.CharFile1[i]);	
				      strcat(fpstring,hexchar);
			    	  
			    	}
		  	    }
		  	    else
		  	    {
			   	sprintf(hexchar,"%02x",gTH30FPBuf.CharFile1[i]);	
				strcat(fpstring,hexchar);
		  	    }
		  	} 
   	      
   	}
    //CString str(fpstring);	
	//*dwEnrollData = str.AllocSysString();
	strcpy(dwEnrollData,fpstring);
      return TRUE;
}
BOOL   WINAPI VerifyFinger(char* dwEnrollData)
{
    pFP_MB_ENROLL TH_MB_BUF;
	
    byte  fpchar[2000];
    char*  fpstr;
	int score=0; 
	int nRET=-1;
	int i,k,j;
	int stringlen;
	BOOL check=TRUE;
	BYTE tmp;
	BYTE num;
	fpstr=dwEnrollData;
	memset(fpchar,0,2000);
	stringlen=strlen(fpstr);
	k=0;
	j=0;
   if(version==0)
   	{		
		for(i=0;i<stringlen;i++) 
	  	{				  	   
	  	     if(k>520)
			 	break;
			 
	  	     if(fpstr[i]=='(') //decode
	  	     	{
					int count=0;
					j=0;
					i++;
					while(j<4&&i<stringlen)
						{
						  if(fpstr[i]==')')
						  	break;
						  count=count*10+(fpstr[i]-'0');
						  i++;
						  j++;
						}
					if(j>3||i>stringlen)
						{ 
						  check=FALSE;
						  	break;
						}
					int a=0;
					for(a=0;a<count;a++)
						{
						   fpchar[k++]=0;
						}
					j=0;
					
	  	     	}
			 else
			 	{
				     if((fpstr[i]>='0')&&(fpstr[i]<='9'))
					  tmp = fpstr[i]-'0';
					 else if((fpstr[i]>='A')&&(fpstr[i]<='F'))
					  tmp = fpstr[i]-'A'+10;
					 else if((fpstr[i]>='a')&&(fpstr[i]<='f'))
					  tmp = fpstr[i]-'a'+10;
					 else
					 { check=FALSE;
					    break;
					 }							   
				   if(j==0)
				   	{
				   	  num=tmp;
					  j=1;
				   	}
				   else
				   	{
				   	  num=num*16+tmp;	
			  	      fpchar[k++]=num;							  
					  j=0;
				   	}
			 	}

	  	}
		if(check!=TRUE||k!=512)
	  	{	  	 
	  	  
		  return FALSE;
	  	}			
		TH_MB_BUF=(pFP_MB_ENROLL)fpchar;
		
		if(TH_MB_BUF->CharFile1.Reserve!=0x24406083)
			{
			  return FALSE;
			}
		
		score=CharMatch((BYTE *)&TH_MB_BUF->CharFile1,(BYTE *)&gTHFPBuf.CharFile1);
		if(score>45)
			{
			return TRUE;
			}

		score=CharMatch((BYTE *)&TH_MB_BUF->CharFile2,(BYTE *)&gTHFPBuf.CharFile1);
		if(score>45)
			{
			return TRUE;
			}
		
   	}
   else
   	{
   	    for(i=0;i<stringlen;i++) 
	  	{				  	   
	  	     if(k>820)
			 	break;
			 
	  	     if(fpstr[i]=='(') //decode
	  	     	{
					int count=0;
					j=0;
					i++;
					while(j<4&&i<stringlen)
						{
						  if(fpstr[i]==')')
						  	break;
						  count=count*10+(fpstr[i]-'0');
						  i++;
						  j++;
						}
					if(j>3||i>stringlen)
						{ 
						  check=FALSE;
						  	break;
						}
					int a=0;
					for(a=0;a<count;a++)
						{
						   fpchar[k++]=0;
						}
					j=0;
					
	  	     	}
			 else
			 	{
				     if((fpstr[i]>='0')&&(fpstr[i]<='9'))
					  tmp = fpstr[i]-'0';
					 else if((fpstr[i]>='A')&&(fpstr[i]<='F'))
					  tmp = fpstr[i]-'A'+10;
					 else if((fpstr[i]>='a')&&(fpstr[i]<='f'))
					  tmp = fpstr[i]-'a'+10;
					 else
					 { check=FALSE;
					    break;
					 }							   
				   if(j==0)
				   	{
				   	  num=tmp;
					  j=1;
				   	}
				   else
				   	{
				   	  num=num*16+tmp;	
			  	      fpchar[k++]=num;							  
					  j=0;
				   	}
			 	}

	  	}
	  if(check!=TRUE||k!=810)
	  	{
	  	  return FALSE;
	  	}
	  
		  nRET=mFPMatch((BYTE *)&gTH30FPBuf.CharFile1[0],(BYTE *)fpchar,3,&score);	
		  if(nRET==0&&score==1)
		  	{
	       return TRUE;
		  	}
		
   	}
   gLastError=ERR_SUCCESS;
   return FALSE;
}
BOOL  WINAPI MatchTwoFinger(char* dwEnrollData1,char* dwEnrollData2)
{
   pFP_MB_ENROLL TH_MB_BUF1;
   pFP_MB_ENROLL TH_MB_BUF2;
	
    byte  fpchar1[2000];
	byte  fpchar2[2000];
    char*  fpstr;
	int score=0; 
	int nRET=-1;
	int i,j;
	int stringlen;
	BOOL check=TRUE;
	BYTE tmp;
	BYTE num;
	int len1=0;
	int len2=0;
	///////////////////////fp1
	fpstr=dwEnrollData1;
	memset(fpchar1,0,2000);
	stringlen=strlen(fpstr);
	check=TRUE;
	j=0;	
    for(i=0;i<stringlen;i++) 
	  	{				  	   
	  	     if(len1>820)
			 	break;
			 
	  	     if(fpstr[i]=='(') //decode
	  	     	{
					int count=0;
					j=0;
					i++;
					while(j<4&&i<stringlen)
						{
						  if(fpstr[i]==')')
						  	break;
						  count=count*10+(fpstr[i]-'0');
						  i++;
						  j++;
						}
					if(j>3||i>stringlen)
						{ 
						  check=FALSE;
						  	break;
						}
					int a=0;
					for(a=0;a<count;a++)
						{
						   fpchar1[len1++]=0;
						}
					j=0;
					
	  	     	}
			 else
			 	{
				     if((fpstr[i]>='0')&&(fpstr[i]<='9'))
					  tmp = fpstr[i]-'0';
					 else if((fpstr[i]>='A')&&(fpstr[i]<='F'))
					  tmp = fpstr[i]-'A'+10;
					 else if((fpstr[i]>='a')&&(fpstr[i]<='f'))
					  tmp = fpstr[i]-'a'+10;
					 else
					 { check=FALSE;
					    break;
					 }							   
				   if(j==0)
				   	{
				   	  num=tmp;
					  j=1;
				   	}
				   else
				   	{
				   	  num=num*16+tmp;	
			  	      fpchar1[len1++]=num;							  
					  j=0;
				   	}
			 	}

	  	}
	if(check!=TRUE||!(len1==512||len1==810))
	  	{	  	 
	  	  
		  return FALSE;
	  	}	
	///////////////////////////////////fp2
	fpstr=dwEnrollData2;
	memset(fpchar2,0,2000);
	stringlen=strlen(fpstr);
	check=TRUE;
	j=0;
    for(i=0;i<stringlen;i++) 
	  	{				  	   
	  	     if(len2>820)
			 	break;
			 
	  	     if(fpstr[i]=='(') //decode
	  	     	{
					int count=0;
					j=0;
					i++;
					while(j<4&&i<stringlen)
						{
						  if(fpstr[i]==')')
						  	break;
						  count=count*10+(fpstr[i]-'0');
						  i++;
						  j++;
						}
					if(j>3||i>stringlen)
						{ 
						  check=FALSE;
						  	break;
						}
					int a=0;
					for(a=0;a<count;a++)
						{
						   fpchar2[len2++]=0;
						}
					j=0;
					
	  	     	}
			 else
			 	{
				     if((fpstr[i]>='0')&&(fpstr[i]<='9'))
					  tmp = fpstr[i]-'0';
					 else if((fpstr[i]>='A')&&(fpstr[i]<='F'))
					  tmp = fpstr[i]-'A'+10;
					 else if((fpstr[i]>='a')&&(fpstr[i]<='f'))
					  tmp = fpstr[i]-'a'+10;
					 else
					 { check=FALSE;
					    break;
					 }							   
				   if(j==0)
				   	{
				   	  num=tmp;
					  j=1;
				   	}
				   else
				   	{
				   	  num=num*16+tmp;	
			  	      fpchar2[len2++]=num;							  
					  j=0;
				   	}
			 	}

	  	}
	if(check!=TRUE||!(len2==512||len2==810))
	  	{	  	 
	  	  
		  return FALSE;
	  	}	
	////////////////////////////
	if(len1!=len2)
		return FALSE;
	
   if(len1==512)
   	{		
   	    TH_MB_BUF1=(pFP_MB_ENROLL)fpchar1;
		TH_MB_BUF2=(pFP_MB_ENROLL)fpchar2;

		if(TH_MB_BUF1->CharFile1.Reserve!=0x24406083)
			{
			gLastError=ERR_SUCCESS;
			return FALSE;
			}
		if(TH_MB_BUF2->CharFile1.Reserve!=0x24406083)
			{
			gLastError=ERR_SUCCESS;
			return FALSE;
			}
		
		score=CharMatch((BYTE *)&TH_MB_BUF1->CharFile1,(BYTE *)&TH_MB_BUF2->CharFile1);
		if(score>45)
			{
			return TRUE;
			}

		score=CharMatch((BYTE *)&TH_MB_BUF1->CharFile2,(BYTE *)&TH_MB_BUF2->CharFile2);
		if(score>45)
			{
			return TRUE;
			}
		
   	}
   else if(len1==810)
   	{

		  nRET=mFPMatch((BYTE *)fpchar1,(BYTE *)fpchar2,3,&score);	
		  if(nRET==0&&score==1)
		  	{
	       return TRUE;
		  	}		
   	}
   gLastError=ERR_SUCCESS;
   return FALSE;
}

