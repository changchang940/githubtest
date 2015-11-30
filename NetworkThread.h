#pragma once
#include "NuroClasses.h"
#include "LoadCallMgrZK.h"
#include "DownLoadTrafficFile.h"

#define _WND_THD_CODE_NODE_COUNT                128

class CNetworkThread
{
public:
	CNetworkThread(void);
	~CNetworkThread(void);

	nuUINT  InitThread(CLoadCallMgrZK*	LoadCallmgr,HWND wnd,nuUINT nPriority  = NURO_THREAD_PRIORITY_LOW);
	nuVOID  FreeThread();
	static nuDWORD ThdAction(nuPVOID lpVoid);
	nuBOOL  PushMsgCode(const NURO_CODE_NODE& codeNode);

	nuUINT MessageProc(const NURO_CODE_NODE& code);
	nuDWORD ThreadAction();

	nuBOOL GetCarDataBuffer(nuLONG gpsstate,nuLONG datacount,nuPVOID pBuf);
	nuBOOL Identification();
	nuBOOL DownLoadDTI();
	nuBOOL DownLoadRoadSec();
	nuBOOL DownLoadCityList();
#ifdef NURO_HS_TMC_GD_DEBUG_LOG
	void   printfile(WCHAR *pstr,WCHAR *filename);
#endif
	nuBOOL testNetworkState();
	nuBOOL getNetworkState();
	nuBOOL setNetworkState(nuBOOL parmstate);
	nuINT getIdentificationState();
	nuINT setIdentificationState(nuINT parmstate);
	nuBOOL getCitylistState();
	nuBOOL setCitylistState(nuBOOL parmstate);
	nuBOOL getDialUpState();
	nuBOOL setDialUpState(nuBOOL parmstate);
	nuINT  urlTestNetwork(WCHAR *url);

private:
	nuHANDLE        m_thdHandle;
	nuBOOL          m_bThrWork;
	CNuroCodeListZK     m_thdCode;
	CLoadCallMgrZK*	m_LoadCallmgr;

	//add by chang;for tmc;2014-11-11;
public:
	CDownLoadTrafficFile m_DownLoadTraffic;
	PTMCCARDATA   m_pCarData;
	int m_gpsStatus;
	int m_nCarDataCount;
	
	bool m_loadpngokchang;
	unsigned char *m_pTMCUsed;
	HWND m_hWnd;
private:
	//add by chang;2015-06-20;
	//true: network is good;
	//false: network is not good;
	nuBOOL m_bNetWorkState;
	nuINT  m_nIdentificationstate;
	nuBOOL m_bDownLoadCitylist;
	nuBOOL m_bDialUpState;
};
