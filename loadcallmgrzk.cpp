// LoadCallMgrZK.cpp: implementation of the CLoadCallMgrZK class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NuroGpsUI.h"
#include "LoadCallMgrZK.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoadCallMgrZK::CLoadCallMgrZK()
{
	m_hInst                     = NULL;
    m_piCallMgr                 = NULL;
	m_nUIType                   = 0;
    m_screen.sysDiff.hScreenDC  = NULL;
    m_screen.sysDiff.hWnd       = NULL;
    m_bLoaden                   = nuFALSE;
    m_bFirstPaint               = nuTRUE;
	long width, height;
	if( !nuReadConfigValue("SCREENWIDTH", &width) )
	{
		width = 800;
	}
	if( !nuReadConfigValue("SCREENHEIGHT", &height) )
	{
		height = 480;
	}
	m_screen.nScreenWidth  = width;
	m_screen.nScreenHeight = height;
	m_screen.nScreenLeft   = 0;
	m_screen.nScreenTop    = 0;
}

CLoadCallMgrZK::~CLoadCallMgrZK()
{

}

nuBOOL CLoadCallMgrZK::LoadCallMgr()
{
#ifdef _USE_CALLMANAGER_DLL
    TCHAR tsPath[256];
    GetModuleFileName(NULL, tsPath, 256);
    for( int i = _tcslen(tsPath) - 1; i >= 0; i-- )
    {
        if( tsPath[i] == '\\' )
        {
            tsPath[i+1] = '\0';
            break;
        }
    }
	//
	_tcscat(tsPath, TEXT("Setting\\"));
	//
    _tcscat(tsPath, TEXT("CallManager.dll"));

    m_hInst = LoadLibrary(tsPath);
    if( NULL == m_hInst )
    {
        return nuFALSE;
    }
    GCM_InitModuleProc pfInitModule;
    pfInitModule = (GCM_InitModuleProc)GetProcAddress(m_hInst, (LPCSTR)("GCM_InitModule"));
    if( NULL == pfInitModule )
    {
        return nuFALSE;
    }
    m_piCallMgr = (CGApiCallMgr*)pfInitModule(NULL);
#else
    m_piCallMgr = (CGApiCallMgr*)GCM_InitModule(NULL);
#endif
    if( NULL == m_piCallMgr )
    {
        return nuFALSE;
    }
    return nuTRUE;
}

nuVOID CLoadCallMgrZK::FreeCallMgr()
{
#ifdef _USE_CALLMANAGER_DLL
    if( NULL != m_hInst )
    {
        GCM_FreeModuleProc pfFreeModule = (GCM_FreeModuleProc)GetProcAddress(m_hInst, (LPCSTR)("GCM_FreeModule"));
        if( NULL != pfFreeModule )
        {
            pfFreeModule();
        }
        FreeLibrary(m_hInst);
        m_piCallMgr = NULL;
        m_hInst = NULL;
    }
#else
    if( NULL != m_piCallMgr )
    {
        GCM_FreeModule();
        m_piCallMgr = NULL;
    }
#endif
}

nuBOOL CLoadCallMgrZK::OnInitDialog(CDialog* pDlg,
									nuPVOID  pFunc/* = NULL */,
						            nuPVOID  pGet_ComData/* = NULL*/,
									nuPVOID  pSend_ComData/*= NULL*/,
						            nuPVOID  pUI_ComData /* = NULL*/,
									nuPVOID  pSend_NaviDat /* = NULL*/)
{
    if( !LoadCallMgr() )
    {
        return nuFALSE;
    }
    if( NULL == pDlg )
    {
       return nuFALSE;
    }
    TCHAR tsPath[256];
    GetModuleFileName(NULL, tsPath, 256);
    for( int i = _tcslen(tsPath) - 1; i >= 0; i-- )
    {
        if( tsPath[i] == '\\' )
        {
            tsPath[i+1] = '\0';
            break;
        }
    }
    //LoadCallMgr
    m_screen.sysDiff.hWnd       = pDlg->m_hWnd;
    m_screen.sysDiff.hScreenDC  = GetDC(pDlg->m_hWnd);
    CGApiCallMgr::CM_INIT_PARAMETER cmParam;
    cmParam.ptzMainPath = tsPath;
    cmParam.cmpScreen   = m_screen;
	cmParam.nUIType     = m_nUIType;
    if( !m_piCallMgr->ICmOnInit(&cmParam) )
    {
		return nuFALSE;
    }
	if (!m_piCallMgr->ICmSetCallBack(pFunc, 0))
	{
		return nuFALSE;
	}
	if (!m_piCallMgr->ICmSetCallBack(pFunc, 1))//add by chang-Exit
	{
		return nuFALSE;
	}

    m_screen = cmParam.cmpScreen;

/*
    pDlg->SetWindowPos(&pDlg->wndTop,
                       0,
                       0,
		               m_screen.nScreenWidth,
		               m_screen.nScreenHeight,
		               SWP_SHOWWINDOW);*/

    m_bLoaden = nuTRUE;

	if (!m_piCallMgr->ICmSetComBuffer(pGet_ComData, pSend_ComData,pSend_NaviDat))
	{
		return nuFALSE;
	}
    return nuTRUE;
}

nuVOID CLoadCallMgrZK::OnDestroyDialog()
{
    m_bLoaden = nuFALSE;
    if( NULL != m_piCallMgr )
    {
        m_piCallMgr->ICmOnFree();
        m_piCallMgr = NULL;
    }
    if( m_screen.sysDiff.hScreenDC )
    {
        DeleteDC(m_screen.sysDiff.hScreenDC);
        m_screen.sysDiff.hScreenDC  = NULL;
        m_screen.sysDiff.hWnd       = NULL;
    }
    FreeCallMgr();
}

nuUINT CLoadCallMgrZK::OnPaint()
{
    if( m_bFirstPaint )
    {
        m_bFirstPaint = nuFALSE;
	}	
	if( NULL != m_piCallMgr )
	{
		m_piCallMgr->ICmOnPaint();
	}
    return 0;
}

nuUINT CLoadCallMgrZK::MouseDown(nuLONG x, nuLONG y)
{
	if( NULL != m_piCallMgr )
	{
		return m_piCallMgr->ICmMouseDown(x, y);
	}
	return 0;
}

nuUINT CLoadCallMgrZK::MouseMove(nuLONG x, nuLONG y)
{
	if( NULL != m_piCallMgr )
	{
		return m_piCallMgr->ICmMouseMove(x, y);
	}
	return 0;
}

nuUINT CLoadCallMgrZK::MouseUp(nuLONG x, nuLONG y)
{
	if( NULL != m_piCallMgr )
	{
		return m_piCallMgr->ICmMouseUp(x, y);
	}
	return 0;
}

nuUINT CLoadCallMgrZK::KeyDown(nuUINT nKey)
{
	if( NULL != m_piCallMgr )
	{
		return m_piCallMgr->ICmKeyDown(nKey);
	}
	return 0;
}

nuUINT CLoadCallMgrZK::KeyUp(nuUINT nKey)
{
	if( NULL != m_piCallMgr )
	{
		return m_piCallMgr->ICmKeyUp(nKey);
	}
	return 0;
}

nuUINT  CLoadCallMgrZK::Message(NURO_UI_MESSAGE& uiMsge)
{
	if( NULL != m_piCallMgr )
	{
		return m_piCallMgr->ICmMessage(uiMsge);
	}
	return 0;
}

nuUINT CLoadCallMgrZK::ResetScreen(nuINT nWidth, nuINT nHeight, nuBYTE nType)
{
	if( NULL != m_piCallMgr )
	{
		return m_piCallMgr->ICmResetScreen(nWidth, nHeight, nType);
	}
	return 0;

}
nuUINT CLoadCallMgrZK::SetNaviThreadCallBackFunc(nuVOID *pfFunc)
{
	if( NULL != m_piCallMgr )
	{
		return m_piCallMgr->ICmSetNaviThreadCallBackFunc(pfFunc);
	}
	return 0;
}