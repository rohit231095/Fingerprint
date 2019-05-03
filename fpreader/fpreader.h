// fpreader.h : main header file for the FPREADER DLL
//

#if !defined(AFX_FPREADER_H__49E11EE2_F086_4049_8DBD_034B3A7132E8__INCLUDED_)
#define AFX_FPREADER_H__49E11EE2_F086_4049_8DBD_034B3A7132E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFpreaderApp
// See fpreader.cpp for the implementation of this class
//

class CFpreaderApp : public CWinApp
{
public:
	CFpreaderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFpreaderApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CFpreaderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FPREADER_H__49E11EE2_F086_4049_8DBD_034B3A7132E8__INCLUDED_)
