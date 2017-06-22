//Name : Server.h
//Author:  ÏÄ×¿·²
//Date : 2017-03-20
//Description : Header of CServer
//Copyright : All by author
#ifndef _SERVER_H
#define _SERVER_H
#include <Windows.h>
#include <vector>
#pragma comment(lib,"Ws2_32.lib")
#define DONTWAIT 1
#define WAITALL 0
class CServer
{
protected:
	//Core connection
	SOCKET m_socketKernel;
	std::vector<SOCKET> m_socketClients;
	int nMode;
public:
	//Init a socket and setup a server
	int Init(int port, const char * address);
	//Data exchange
	unsigned int Run(int quantity);
	//End procedure
	void Shut();
	//Send Message
	int SendMsg(const char *msg, int len,int seq);
	//Recieve Message
	int RecvMsg(char *msg, int len,int seq);
	//Constructor
	CServer(int);	//mode: 1 = DONTWAIT 0 = WAITALL
	//Manipulator
	void Iomanip(int);
};
#endif
