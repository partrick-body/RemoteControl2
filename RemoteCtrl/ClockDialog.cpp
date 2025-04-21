// ClockDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "lockDialog.h"
#include "afxdialogex.h"


// ClockDialog 对话框

IMPLEMENT_DYNAMIC(ClockDialog, CDialog)

ClockDialog::ClockDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_INFO, pParent)
{

}

ClockDialog::~ClockDialog()
{
}

void ClockDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ClockDialog, CDialog)
END_MESSAGE_MAP()


// ClockDialog 消息处理程序
