#pragma once
#include"ClientSocket.h"
#include"CWatchDialog.h"
#include"RemoteClientDlg.h"
#include"StatusDlg.h"
#include"map"
#include "resource.h"
#include"EdoyunTool.h"

//#define WM_SEND_PACK (WM_USER+1)//发送包数据
//#define WM_SEND_DATA (WM_USER+2)//发送数据
#define WM_SHOW_STATUS (WM_USER+3)//展示状态
#define WM_SHOW_WATCH (WM_USER+4)//远程监控
#define WM_SEND_MESSAGE (WM_USER+0x1000)//自定义消息处理
class CClientController
{
public:
	//获取全局唯一对象
	static CClientController* getInstance();
	//初始化操作
	int InitController();
	//启动
	int Invoke(CWnd*&pMinWnd);
	//发送消息
	//LRESULT SendMessage(MSG msg);
	//更新网络服务器的地址
	void UpdateAddress(int nIP, int nPort)
	{
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}
	int DealCommand()
	{
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket()
	{
		CClientSocket::getInstance()->CloseSocket();
	}

	//1查看磁盘分区
	//2查看指定目录下的文件
	//3打开文件
	//4下载文件
	//5鼠标操作
	//6发送屏幕内容
	//7锁机
	//8解锁
	//9删除文件
	// 1981测试连接
	//返回值是状态，true是成功，false是失败
	bool SendCommandPacket(HWND hWnd/*数据包收到后，需要应答的窗口*/,int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0, WPARAM wParam=0);

	int GetImage(CImage& image)
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		return CEdoyunTool::Bytes2Image(image, pClient->GetPacket().strData);
		
	}
	void DownloadEnd();
	int DownFile(CString strPath);
	void StartWatchScreen();
protected:
	void threadWatchScreen();
	static void threadWatchScreen(void* arg);
	void threadDownloadFile();
	//static void threadDownloadEntry(void* arg);
	CClientController():
		m_statusDlg(&m_remoteDlg),
		m_watchDlg(&m_remoteDlg)//指定父窗口
	{
		m_isClosed = true;
		HANDLE m_hThreadWatch = INVALID_HANDLE_VALUE;
		
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController()
	{
		WaitForSingleObject(m_hThread, 100);
	}
	void threadFunc();
	static unsigned __stdcall threadEntry(void* arg);
	static void releaseInstance()
	{
		if (m_instance != NULL)
		{
			delete m_instance;
			m_instance = NULL;
		}
		
	}
	//LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowwatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo {
		MSG msg;
		LRESULT result;
		
		MsgInfo(MSG m)
		{
			result = 0;
			memset(&msg, 0, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m)
		{
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m)
		{
			if (this != &m)
			{
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			
			return *this;
		}


	}MSGINFO;
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
    static std::map<UINT, MSGFUNC> m_mapFunc;
	CWatchDialog m_watchDlg;//消息包，在对话框关闭后，可能导致内存泄漏
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;//三个对话框

	HANDLE m_hThread;
	HANDLE m_hThreadWatch;
	CString m_strRemote;//下载文件的远程路径
	CString m_strLocal;//下载文件的本地保存路径
	bool m_isClosed;//监视是否关闭
	unsigned int m_nThreadID;
	static CClientController* m_instance;
	class CHelper {
	public:
		CHelper() {
			//CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}
	};
	static CHelper m_helper;
};

