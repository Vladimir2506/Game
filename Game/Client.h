//Name : Client.cpp
//Author:  ÏÄ×¿·²
//Date : 2017-03-20
//Description : Header of CClient
//Copyright : All by author
#ifndef _CLIENT_H
#define _CLIENT_H
#include <Windows.h>
#pragma comment(lib,"Ws2_32.lib")
#define DONTWAIT 1
#define WAITALL 0
class CClient
{
protected:
	//Core connection
	SOCKET m_socketKernel;
	int nMode;
public:
	//Connector
	int Connect(int port, const char * address);
	//Send Message
	int SendMsg(const char *msg, int len);
	//Recieve Message
	int RecvMsg(char *msg, int len);
	//Shut down
	void Close();
	//Constructor
	CClient(int);
	//Manipulator
	void Iomanip(int);
};
#endif
