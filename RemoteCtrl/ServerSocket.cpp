#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;
CServerSocket* CServerSocket::m_instance = NULL;
CServerSocket::CHelper CServerSocket::m_helper;//��������������ͷŵ�������
CServerSocket* pserver = CServerSocket::getInstance();
//********************//
//����ʵ�ֵ���ģʽ�Ĺؼ�Ҫ��
//
//˽�й��캯������ֹ�ⲿͨ�� new ��������������
//��̬���������ԣ��ṩȫ�ַ��ʵ��Ի�ȡʵ����
//�̰߳�ȫ�����̻߳�������ȷ���ڲ�������²��ᴴ�����ʵ����
//********************//