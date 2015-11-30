#include "StdAfx.h"
#include "NetworkThread.h"
#ifdef NURO_HS_TMC_GD
#include <afxinet.h>
#include <WinInet.h>
#endif

CNetworkThread::CNetworkThread(void)
{
	m_LoadCallmgr = NULL;
	m_pCarData    = NULL;
	m_pTMCUsed	  = NULL;
	m_nCarDataCount = 0;
	m_loadpngokchang = nuFALSE;

	/////////////////////////////
	m_hWnd = NULL;
	m_bNetWorkState = nuFALSE;
	m_bDownLoadCitylist = nuFALSE;
	m_nIdentificationstate = nuFALSE;
	m_bDialUpState = nuFALSE;

}
CNetworkThread::~CNetworkThread(void)
{
	if (m_pCarData)
	{
		delete[] m_pCarData;
		m_pCarData = NULL;
	}
}

nuUINT  CNetworkThread::InitThread(CLoadCallMgrZK*	LoadCallmgr,HWND wnd,nuUINT nPriority /* = NURO_THREAD_PRIORITY_LOW */)
{
	if (LoadCallmgr == NULL)
	{
		return 0;
	}
	if( !m_thdCode.CreateCodeList(_WND_THD_CODE_NODE_COUNT) )
	{
		return 0;
	}

	m_DownLoadTraffic.Init();
	m_hWnd = wnd;

	m_pCarData = new TMCCARDATA[_TMC_DATA_COUNT_];
	if (NULL == m_pCarData)
	{
		return 0;
	}
	nuMemset(m_pCarData,0,sizeof(TMCCARDATA)*_TMC_DATA_COUNT_);

	m_LoadCallmgr = LoadCallmgr;
	m_bThrWork = nuTRUE;

	//edit by chang;2014-11-27;
	m_thdHandle = nuCreateThread(NULL, 0, ThdAction, this, NURO_THREAD_CREATE_DEFAULT, NULL);

	if (NULL == m_thdHandle)
	{
		m_bThrWork   = nuFALSE;
		return 0;
	}

	nuSetThreadPriority(m_thdHandle, THREAD_PRIORITY_BELOW_NORMAL);
	/*if (NURO_THREAD_CREATE_DEFAULT != nPriority)
	{
		nuSetThreadPriority(m_thdHandle, nPriority);
	}*/
	return 1;
}

nuDWORD CNetworkThread::ThdAction(nuPVOID lpVoid)
{

	CNetworkThread* pThis = (CNetworkThread*)lpVoid;
	//pThis->m_bOutThread = nuFALSE;
	nuDWORD nRes = pThis->ThreadAction();
	//pThis->m_bOutThread = nuTRUE;
	//nuThreadWaitReturn(pThis->m_thdHandle);
	return nRes;
}
nuVOID  CNetworkThread::FreeThread()
{
	if( m_bThrWork )
	{
		m_bThrWork = nuFALSE;
		nuDelThread(m_thdHandle);
		nuMemset(&m_thdHandle, 0, sizeof(m_thdHandle));
	}

	m_thdCode.DeleteCodeList();
}

nuUINT CNetworkThread::MessageProc(const NURO_CODE_NODE& code)
{
#ifdef NURO_HS_TMC_GD
	try
	{
		if (_NR_MSG_HTTP_NETWORKSTATETEST_ == code.nCodeID)
		{

			if (getDialUpState()&&testNetworkState())
			{

					//network state is good;
					NURO_UI_MESSAGE nrMsg = {0};
					nrMsg.nMsgID  = _NR_MSG_NETWORK_STATE_;
					nrMsg.nParam1 = 1;
					m_LoadCallmgr->Message(nrMsg); 

					//send message to identification;
					//因为网络原因没鉴权成功，则可以多次自动鉴权，不需要用户参与
					//非网络原因;如果机器序列号不对，电话号码不对等,则需等待用户手动鉴权;
					if (0 == getIdentificationState()||5 == getIdentificationState())
					{
						NURO_CODE_NODE  code1 = {0};
						code1.nParamX    = 0;
						code1.nParamY    = 0;
						code1.pExtend    = NULL;
						code1.nCodeID    = _NR_MSG_HTTP_IDENTIFICATION_;
						PushMsgCode(code1);
					}
					else if(1 == getIdentificationState())//鉴权成功,可下载数据;
					{
						NURO_CODE_NODE  code2 = {0};
						code2.nParamX    = 0;
						code2.nParamY    = 0;
						code2.pExtend    = NULL;
						if (code.nParamY == 1)//only road;
						{
							code2.nCodeID    = _NR_MSG_HTTP_DOWNLOAD_ROADSEC_;
							PushMsgCode(code2);
						}
						if (code.nParamX == 1)//all
						{
							if (getCitylistState())
							{
								//push citylist msg;
								code2.nCodeID    = _NR_MSG_HTTP_DOWNLOAD_DTI_;
								PushMsgCode(code2);
							}
							else
							{
								code2.nCodeID    = _NR_MSG_HTTP_DOWNLOAD_CITYLIST_;
								PushMsgCode(code2);
							}
						}
					}//undo to tell user to input phonenumber;
		
			}
			else
			{
				NURO_UI_MESSAGE nrMsg = {0};
				nrMsg.nMsgID  = _NR_MSG_NETWORK_STATE_;
				nrMsg.nParam1 = 0;
				m_LoadCallmgr->Message(nrMsg); 
				//ask for dial up again;
				HWND hMainCtrl = NULL;
				hMainCtrl = ::FindWindow(NULL, TEXT("HSAEMainControl"));

				if (hMainCtrl != NULL)
				{
					::PostMessage(hMainCtrl,0x950B,0,0);
				}
			}
			return 0;
		}
		if (_NR_MSG_HTTP_IDENTIFICATION_ == code.nCodeID)
		{
			Identification();
			if (1 == getIdentificationState())
			{
				NURO_CODE_NODE  code3 = {0};
				code3.nParamX    = 0;
				code3.nParamY    = 0;
				code3.pExtend    = NULL;
				code3.nCodeID    = _NR_MSG_HTTP_DOWNLOAD_CITYLIST_;
				PushMsgCode(code3);
			}
			return 1;
		}
		if (1 != getIdentificationState())
		{
			return 1;
		}
		if (_NR_MSG_HTTP_DOWNLOAD_DTI_ == code.nCodeID)
		{
			DownLoadDTI();

			return 1;
		}
		if (_NR_MSG_HTTP_DOWNLOAD_ROADSEC_ == code.nCodeID)
		{
			DownLoadRoadSec();
			return 1;
		}
		if (_NR_MSG_HTTP_DOWNLOAD_CITYLIST_ == code.nCodeID)
		{
			DownLoadCityList();
			return 1;
		}
	}
	catch(CInternetException *e)
	{
		TCHAR   szCause[255] = {0};
		CString strFormatted;
		e->GetErrorMessage(szCause, 255);

#ifdef NURO_HS_TMC_GD_DEBUG_LOG
		SYSTEMTIME st ;
		GetLocalTime(&st);
		WCHAR timestr[256] = {0};
		swprintf(timestr,TEXT("\n %d%d%d%d%d%d:CInternetException==%s \n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,szCause);
		WCHAR  file[] = TEXT("exception.txt");
		printfile(timestr,file);
#endif
	}
#endif 
	return 1;
}
#ifdef NURO_HS_TMC_GD_DEBUG_LOG
void CNetworkThread::printfile(WCHAR *pstr,WCHAR *filename)
{
	if (NULL == pstr || NULL == filename)
	{
		return;
	}
	FILE* fp;
	TCHAR tsPath[256] = {0};
	GetModuleFileName(NULL, tsPath, 256);
	for( int i = _tcslen(tsPath) - 1; i >= 0; i-- )
	{
		if( tsPath[i] == '\\' )
		{
			tsPath[i+1] = '\0';
			break;
		}
	}
	wcscat(tsPath,filename);
	fp = _wfopen(tsPath,TEXT("a+"));

	if (fp)
	{
		fwprintf(fp,pstr);
		fclose(fp);
		//undo: translate message to nuroUI,tell user;
	}
}
#endif

nuDWORD CNetworkThread::ThreadAction()
{
	NURO_CODE_NODE code = {0};
	while( m_bThrWork )
	{
		if( m_thdCode.PopNode(code) )
		{
			MessageProc(code);		
		}
		else
		{
			nuSleep(50);
		}
	}
	return 1;
}

nuBOOL CNetworkThread::PushMsgCode(const NURO_CODE_NODE &codeNode)
{
	return m_thdCode.PushNode(codeNode);
}

nuBOOL CNetworkThread::GetCarDataBuffer(nuLONG gpsstate,nuLONG datacount,nuPVOID pBuf)
{
	m_gpsStatus = gpsstate;
	m_nCarDataCount = datacount;
	if (pBuf)
	{
		PTMCCARDATA pSrc = (PTMCCARDATA)pBuf;
		if (m_pCarData)
		{
			nuMemcpy(m_pCarData,pSrc,sizeof(TMCCARDATA)*m_nCarDataCount);
		}
	}
	return true;
}

nuBOOL CNetworkThread::Identification()
{
	DWORD identiflag = 0 ;
	identiflag = m_DownLoadTraffic.Identification();

	//identiflag = 200;

	//identification;
	if (identiflag == 200)
	{
		setIdentificationState(1);
	}
	else
	{
		setIdentificationState(identiflag);
	}
	nuWCHAR *pPalmHttp = NULL;
	pPalmHttp = m_DownLoadTraffic.ReadIdentiResult();
	if (pPalmHttp != NULL)
	{
		nuWCHAR *p = NULL; 
		p = wcsstr(pPalmHttp,TEXT("/..."));

		if (p != NULL)
		{
			*p = NULL;
			wcscpy(m_DownLoadTraffic.m_palmCityServer,pPalmHttp);
		}

		delete[] pPalmHttp;
		pPalmHttp = NULL;
	}

	//tell nuroui;
	NURO_UI_MESSAGE nrMsg = {0};
	nrMsg.nMsgID  = _NR_MSG_HTTP_IDENTIFICATION_;
	nrMsg.nParam1 = identiflag;
	nrMsg.nParam2 = 0;
	m_LoadCallmgr->Message(nrMsg);

	//for test;
	{
		NURO_UI_MESSAGE nrMsg = {0};
		nrMsg.nMsgID = _NR_MSG_TMC_FRONT_SOUND_;
		nrMsg.nParam1 = 0;
		nrMsg.nParam2 = 0;
		if (0 != nrMsg.nMsgID)
		{
			m_LoadCallmgr->Message(nrMsg);
		}

	}

	//////////////////////////////

	return nuTRUE;
}
nuBOOL CNetworkThread::DownLoadDTI()
{
	//down load dti;
	if (m_nCarDataCount == _TMC_DATA_COUNT_)
	{
		while((*m_pTMCUsed) > 0)
		{
			nuSleep(50);
		}
		*m_pTMCUsed = true;
		m_DownLoadTraffic.resetCoorBuff();
		if (m_pCarData)
		{
			for (int i = _TMC_DATA_COUNT_-4 ;i < _TMC_DATA_COUNT_;i++)
			{
				nuWCHAR timebuff[256] = {0};
				float flon = 0.0,flat = 0.0;
				m_DownLoadTraffic.nuMeterToDegree(m_pCarData[i].nlong,flon);
				m_DownLoadTraffic.nuMeterToDegree(m_pCarData[i].nlat,flat);

				swprintf(timebuff, TEXT("%d-%d-%d %d:%d:%d"), 
					m_pCarData[i].nYear,
					m_pCarData[i].nMonth,
					m_pCarData[i].nDay,
					m_pCarData[i].nHours,
					m_pCarData[i].nMinutes,
					m_pCarData[i].nSeconds);

				m_DownLoadTraffic.setCoorData(flon,flat,timebuff,m_pCarData[i].nspeed,m_pCarData[i].ndirection);

				if (i == _TMC_DATA_COUNT_ - 1)
				{
					m_DownLoadTraffic.SetCarPos(flon,flat);
				}

			}
			m_DownLoadTraffic.urlCoorData();
			m_DownLoadTraffic.DownLoadDTI();
		}
		*m_pTMCUsed = false;

		if(m_DownLoadTraffic.m_bTMCUpdate)
		{
			m_DownLoadTraffic.m_bTMCUpdate = nuFALSE;
			NURO_UI_MESSAGE nrMsg = {0};
			nrMsg.nMsgID = _NR_MSG_TMC_DATA_UPDATE;
			m_LoadCallmgr->Message(nrMsg);
		}
	}
	return nuTRUE;
}
nuBOOL CNetworkThread::DownLoadRoadSec()
{
	{
		NURO_UI_MESSAGE nrMsg = {0};
		nrMsg.nMsgID  = _NR_MSG_SHOW_PNG_;
		nrMsg.nParam1 = 0; 
		nrMsg.nParam2 = 0; 
		m_LoadCallmgr->Message(nrMsg);
	}

	//down load dti;
	if (m_nCarDataCount == _TMC_DATA_COUNT_)
	{
		m_DownLoadTraffic.resetCoorBuff();
		if (m_pCarData)
		{
			//road secretary;
			m_DownLoadTraffic.setLocation(m_pCarData,m_nCarDataCount);
			m_DownLoadTraffic.DownLoadRoadSecretary();

			//tell mainctrl to play sound;
			if (m_DownLoadTraffic.m_roadSecurityFileLen)
			{
				{
					NURO_UI_MESSAGE nrMsg = {0};
					nrMsg.nMsgID = _NR_MSG_TMC_FRONT_SOUND_;
					nrMsg.nParam1 = 0;
					nrMsg.nParam2 = 0;

					if (0 != nrMsg.nMsgID)
					{
						m_LoadCallmgr->Message(nrMsg);
					}
				}
				{
					NURO_UI_MESSAGE nrMsg = {0};
					nrMsg.nMsgID = _NR_MSG_TMC_EVENT_SOUND_;
					nrMsg.nParam1 = 0;
					nrMsg.nParam2 = 0;
					if (0 != nrMsg.nMsgID)
					{
						m_LoadCallmgr->Message(nrMsg);
					}
				}
			}
		}
	}
	bool loadpngok = false;
	if (m_DownLoadTraffic.m_roadSecurityFileLen)
	{
		loadpngok = m_DownLoadTraffic.DownLoadPng(NULL);
	}

	if (loadpngok)
	{
		NURO_UI_MESSAGE nrMsg = {0};
		nrMsg.nMsgID  = _NR_MSG_SHOW_PNG_;
		nrMsg.nParam1 = 1; 
		nrMsg.nParam2 = 0;
		m_LoadCallmgr->Message(nrMsg);
	}
	else
	{
		NURO_UI_MESSAGE nrMsg = {0};
		nrMsg.nMsgID  = _NR_MSG_SHOW_PNG_;
		nrMsg.nParam1 = 0;
		nrMsg.nParam2 = 0;
		m_LoadCallmgr->Message(nrMsg);
	}
	return nuTRUE;
}

nuBOOL CNetworkThread::DownLoadCityList()
{
	if (m_DownLoadTraffic.DownLoadCityList())
	{
		if (m_DownLoadTraffic.LoadCityListFile())
		{
			setCitylistState(nuTRUE);
			NURO_CODE_NODE  code = {0};
			code.nParamX    = 0;
			code.nParamY    = 0;
			code.pExtend    = NULL;
			code.nCodeID    = _NR_MSG_HTTP_DOWNLOAD_DTI_;
			PushMsgCode(code);
		}
	}
	return nuTRUE;
}
nuBOOL CNetworkThread::testNetworkState()
{
	//wcscpy(baiduurl,TEXT("http://news.baidu.com"));

	WCHAR palmurl[512] = {0};
	WCHAR tempurl[512] = {0};
	wcscpy(tempurl,TEXT("http://cloud.palmgo.cn"));
	//wcscpy(palmurl,TEXT("http://cloud.palmgo.cn/"));
	nuBOOL restemp = urlTestNetwork(tempurl);
	//nuBOOL respalm = urlTestNetwork(palmurl);
	if (!restemp)
	{
		return nuFALSE;
	}
	else
	{
		return nuTRUE;
	}
}
nuBOOL CNetworkThread::getNetworkState()
{
	return m_bNetWorkState;
}
nuBOOL CNetworkThread::setNetworkState(nuBOOL parmstate)
{
	m_bNetWorkState = parmstate;
	return m_bNetWorkState;
}
nuINT CNetworkThread::getIdentificationState()
{
	return m_nIdentificationstate;
}
nuINT CNetworkThread::setIdentificationState(nuINT parmstate)
{
	m_nIdentificationstate = parmstate;
	return m_nIdentificationstate;
}
nuBOOL CNetworkThread::getCitylistState()
{
	return m_bDownLoadCitylist;
}
nuBOOL CNetworkThread::setCitylistState(nuBOOL parmstate)
{
	m_bDownLoadCitylist = parmstate;
	return m_bDownLoadCitylist;
}
nuBOOL CNetworkThread::getDialUpState()
{
	return m_bDialUpState;
}
nuBOOL CNetworkThread::setDialUpState(nuBOOL parmstate)
{
	m_bDialUpState = parmstate;
	return m_bDialUpState;
}
nuINT  CNetworkThread::urlTestNetwork(WCHAR *url)
{
	/*{
	SYSTEMTIME st ;
	GetLocalTime(&st);
	WCHAR timestr[256] = {0};
	swprintf(timestr,TEXT("\n %d-%d-%d-%d-%d-%d:bigen==%s \n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,url);
	WCHAR  file[] = TEXT("networkok.txt");
	printfile(timestr,file);
	}*/
	nuBOOL res = nuFALSE;
	WCHAR tempurl[512] = {0};
	wcscpy(tempurl,url);
#ifdef NURO_HS_TMC_GD
	DWORD dwStatecode;
	CInternetSession session;
	CHttpConnection* pHttpCon = NULL;
	CHttpFile*       pHttpFile = NULL;
	CString    strServer = _T("");
	CString    strObject = _T("");
	INTERNET_PORT   wPort = 0;
	DWORD  dwType = 0;
	try
	{
		AfxParseURL(tempurl,dwType,strServer,strObject,wPort);
		pHttpCon = session.GetHttpConnection(strServer,wPort,NULL,NULL);
		if (NULL != pHttpCon)
		{
			pHttpFile = pHttpCon->OpenRequest(CHttpConnection::HTTP_VERB_POST,strObject,NULL,1,NULL,TEXT("HTTP/1.1"),INTERNET_FLAG_EXISTING_CONNECT|INTERNET_FLAG_NO_AUTO_REDIRECT);
			if (pHttpFile)
			{
				pHttpFile->SendRequest();

				pHttpFile->QueryInfoStatusCode(dwStatecode);
				if (dwStatecode == 200)
				{
					res = nuTRUE;
				}
				else
				{
					res = nuFALSE;
				}
			}
			/*
			//for test file;
			SYSTEMTIME st ;
			GetLocalTime(&st);
			WCHAR timestr[256] = {0};
			swprintf(timestr,TEXT("\n %d-%d-%d-%d-%d-%d:urlTestNetwork==%d \n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,dwStatecode);
			WCHAR  file[] = TEXT("networkok.txt");
			printfile(timestr,file);*/
		}
	}
	catch (CInternetException* e)
	{
		TCHAR   szCause[255] = {0};
		CString strFormatted;
		e->GetErrorMessage(szCause, 255);
#ifdef NURO_HS_TMC_GD_DEBUG_LOG
		SYSTEMTIME st ;
		GetLocalTime(&st);
		WCHAR timestr[256] = {0};
		swprintf(timestr,TEXT("\n %d-%d-%d-%d-%d-%d:CInternetException==%s \n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,szCause);
		WCHAR  file[] = TEXT("exception.txt");
		printfile(timestr,file);
#endif
		res = nuFALSE;
	}
	if (pHttpFile)
	{
		pHttpFile->Close();
		delete pHttpFile;
	}
	if (pHttpCon)
	{
		pHttpCon->Close();
		delete pHttpCon;
	}
	session.Close();
#endif
	/*{
	SYSTEMTIME st ;
	GetLocalTime(&st);
	WCHAR timestr[256] = {0};
	swprintf(timestr,TEXT("\n %d-%d-%d-%d-%d-%d:end res==%d \n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,res);
	WCHAR  file[] = TEXT("networkok.txt");
	printfile(timestr,file);
	}*/

	return res;
}
