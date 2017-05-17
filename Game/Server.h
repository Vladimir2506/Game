//Header of CServer
#ifndef _SERVER_H
#define _SERVER_H
#include <Windows.h>
#include <vector>
#pragma comment(lib,"Ws2_32.lib")
class CServer
{
protected:
	//Core connection
	SOCKET m_socketKernel;
	std::vector<SOCKET> m_socketClients;
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
};
#endif
