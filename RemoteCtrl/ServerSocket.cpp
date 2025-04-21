#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;
CServerSocket* CServerSocket::m_instance = NULL;
CServerSocket::CHelper CServerSocket::m_helper;//创建单例对象和释放单例对象
CServerSocket* pserver = CServerSocket::getInstance();
//********************//
//二、实现单例模式的关键要素
//
//私有构造函数：防止外部通过 new 操作符创建对象。
//静态方法或属性：提供全局访问点以获取实例。
//线程安全（多线程环境）：确保在并发情况下不会创建多个实例。
//********************//