#include "pch.h"
#include "ClientController.h"
#include "ClientSocket.h"
#include "resource.h"

std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;//两个静态变量需要实现
CClientController::CHelper CClientController::m_helper;
CClientController* CClientController::getInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new CClientController();
		struct {
			UINT nMsg;
			MSGFUNC func;
		}MsgFuncs[] =
		{
			//{WM_SEND_PACK, &CClientController::OnSendPack},
			//{WM_SEND_DATA, &CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH, &CClientController::OnShowwatcher},
		{(UINT)-1,NULL}
		};
		for (int i = 0; MsgFuncs[i].func != NULL; i++)
		{
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(MsgFuncs[i].nMsg,MsgFuncs[i].func));
		}
	}
	return m_instance;
}

int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(
		NULL, 0, &CClientController::threadEntry,
		this, 0, &m_nThreadID);
	m_statusDlg.Create(IDD_DLG_STATUS, &m_remoteDlg);
	return 0;
}

int CClientController::Invoke(CWnd*& pMinWnd)
{
	pMinWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}

//LRESULT CClientController::SendMessage(MSG msg)
//{  
//	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//	if (hEvent == NULL)return-2;
//	MSGINFO info(msg);
//	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)hEvent);
//	WaitForSingleObject(hEvent, INFINITE);//等待时间
//	CloseHandle(hEvent);//回收事件句柄，防止资源耗尽
//	return info.result;
//}

/*数据包收到后，需要应答的窗口*/
bool CClientController::SendCommandPacket(HWND hWnd, int nCmd, bool bAutoClose, BYTE* pData, size_t nLength, WPARAM wParam)
{
	CClientSocket* pClient = CClientSocket::getInstance();
	
    bool ret = pClient->SendPacket(hWnd,CPacket(nCmd, pData, nLength), bAutoClose,wParam);
	return  ret;
	
}
void CClientController::DownloadEnd()
{
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("下载完成！！"), _T("完成"));
}
int CClientController::DownFile(CString strPath)
{
	CFileDialog dlg(FALSE, NULL, strPath, OFN_HIDEREADONLY
		| OFN_OVERWRITEPROMPT, NULL, &m_remoteDlg);

	if (dlg.DoModal() == IDOK)
	{
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();

		FILE* pFile = fopen(m_strLocal, "wb+");
		if (pFile == NULL) {
			AfxMessageBox(_T("本地没有权限保存该文件，或者文件无法创建！！！"));
			return -1;
		}
		SendCommandPacket(m_remoteDlg, 4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength(), (WPARAM)pFile);

		m_remoteDlg.BeginWaitCursor();//光标设置成等待沙漏
		m_statusDlg.m_info.SetWindowText(_T("命令正在执行中！"));
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(&m_remoteDlg);//在remoteDlg中创建窗口
		m_statusDlg.SetActiveWindow();//激活，置到前台
		//TODO:大文件传输需要额外的处理
	}

	return 0;
}

void CClientController::StartWatchScreen()
{
	m_isClosed = false;
	//m_watchDlg.SetParent(&m_remoteDlg);
	CWatchDialog dlg(&m_remoteDlg);
	m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadWatchScreen, 0, this);
	m_watchDlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);
}

#define Delay 500 //监控延迟

void CClientController::threadWatchScreen()
{
	Sleep(50);
	ULONGLONG nTick = GetTickCount64();
	while (!m_isClosed) {
		if (m_watchDlg.isFull() == false) {
			if (GetTickCount64() - nTick < Delay) {
				Sleep(Delay - DWORD(GetTickCount64() - nTick));
			}
			nTick = GetTickCount64();
			int ret = SendCommandPacket(m_watchDlg.GetSafeHwnd(), 6, true, NULL, 0);
			
			if (ret == 1) {
				//TRACE("成功发送请求图片命令\r\n");
			}
			else {
				TRACE("获取图片失败！ret = %d\r\n", ret);
			}
		}
		Sleep(1);
	}
	TRACE("thread end %d\r\n", m_isClosed);
}

void CClientController::threadWatchScreen(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadWatchScreen();
	_endthread();
}

//void CClientController::threadDownloadFile()
//{
//	FILE* pFile = fopen(m_strLocal, "wb+");
//	if (pFile == NULL)
//	{
//		AfxMessageBox(_T("本地没有权限保存该文件，或者文件无法创建！！！"));
//		m_statusDlg.ShowWindow(SW_HIDE);
//		m_remoteDlg.EndWaitCursor();
//		return;
//	}
//	CClientSocket* pClient = CClientSocket::getInstance();
//	do
//	{
//		int ret = CClientController::getInstance()->SendCommandPacket(m_remoteDlg,4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength());
//		long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
//		if (nLength == 0)
//		{
//			AfxMessageBox("文件长度为零或者无法读取文件！！！");
//			return;
//		}
//		long long nCount = 0;
//		while (nCount < nLength)
//		{
//			ret = pClient->DealCommand();//正常应该等于download命令4
//			if (ret < 0)
//			{
//				AfxMessageBox("传输失败！！");
//				TRACE("传输失败：ret=%d\r\n", ret);
//				break;
//			}
//			fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
//			nCount += pClient->GetPacket().strData.size();
//		}
//	} while (false);
//	fclose(pFile);
//	pClient->CloseSocket();
//	m_statusDlg.ShowWindow(SW_HIDE);
//	m_remoteDlg.EndWaitCursor();
//	m_remoteDlg.MessageBox(_T("下载完成！！"), _T("完成"));
//
//}

//void CClientController::threadDownloadEntry(void* arg)
//{
//	CClientController* thiz = (CClientController*)arg;
//	thiz->threadDownloadFile();
//	_endthread();
//}

void CClientController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message==WM_SEND_MESSAGE)
		{
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end())
			{
				pmsg->result=(this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
				 
			}
			else
			{
				pmsg->result = -1;
			}
			SetEvent(hEvent);
		}
		else
		{
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end())
			{
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
			}
		}
		
	}
}

unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}

//LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	CPacket* pPacket = (CPacket*)wParam;
//	return pClient->Send(*pPacket);
//}
//
//LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	char* pBuffer = (char*)lParam;
//	return pClient->Send(pBuffer,(int)lParam);
//}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowwatcher(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}
