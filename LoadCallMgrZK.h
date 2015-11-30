// LoadCallMgrZK.h: interface for the CLoadCallMgrZK class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOADCALLMGRZK_H__1C791232_DAF2_4023_BE25_647FAEE0EEBD__INCLUDED_)
#define AFX_LOADCALLMGRZK_H__1C791232_DAF2_4023_BE25_647FAEE0EEBD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GApiCallMgr.h"

#define _USE_CALLMANAGER_DLL

class CLoadCallMgrZK  
{
public:
    typedef nuPVOID(*GCM_InitModuleProc)(nuPVOID);
    typedef nuVOID(*GCM_FreeModuleProc)();

public:
	CLoadCallMgrZK();
	virtual ~CLoadCallMgrZK();

	nuBOOL  OnInitDialog(CDialog* pDlg,
		                 nuPVOID  pFunc        = NULL,
						 nuPVOID  pGet_ComData = NULL,
						 nuPVOID  pSend_ComData= NULL,
						 nuPVOID  pUI_ComData  = NULL,
						 nuPVOID  pSend_NaviDat  = NULL);
    nuVOID  OnDestroyDialog();
	
	nuUINT  OnPaint();
	nuUINT  MouseDown(nuLONG x, nuLONG y);
    nuUINT  MouseMove(nuLONG x, nuLONG y);
    nuUINT  MouseUp(nuLONG x, nuLONG y);
    nuUINT  KeyDown(nuUINT nKey);
    nuUINT  KeyUp(nuUINT nKey);
    nuUINT  Message(NURO_UI_MESSAGE& uiMsge);
	nuUINT  ResetScreen(nuINT nWidth, nuINT nHeight, nuBYTE nType);
	nuUINT  SetNaviThreadCallBackFunc(nuVOID *pfFunc);
protected:
    nuBOOL  LoadCallMgr();
    nuVOID  FreeCallMgr();
	
protected:
    HINSTANCE       m_hInst;
    CGApiCallMgr*   m_piCallMgr;
 
    NURO_SCREEN     m_screen;
    nuBOOL          m_bLoaden;
    nuBOOL          m_bFirstPaint;

public:
	nuUINT          m_nUIType;
	nuLONG			m_lWidth;
	nuLONG          m_lHeight;
};

#endif // !defined(AFX_LOADCALLMGRZK_H__1C791232_DAF2_4023_BE25_647FAEE0EEBD__INCLUDED_)
