//Implementation of CServer
#include"Server.h"
#include<cstdio>
#include<cstring>
int CServer::Init(int port, const char * address)
{
	int nStatus = 0;
	WSAData wsadata;
	//Launch Winsock
	int nErrMsg = WSAStartup(MAKEWORD(1, 1), &wsadata);
	if (nErrMsg != NO_ERROR)
	{
		
		nStatus = 1;
		return nStatus;
	}
	//Generate socket
	m_socketKernel = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socketKernel == INVALID_SOCKET)
	{
		
		nStatus = 2;
		return nStatus;
	}
	//Initial Server
	sockaddr_in sockaddrServer;
	sockaddrServer.sin_family = AF_INET;
	sockaddrServer.sin_port = port;
	sockaddrServer.sin_addr.S_un.S_addr = inet_addr(address);
	//Bind
	nErrMsg = bind(m_socketKernel, (sockaddr*)(&sockaddrServer), sizeof(sockaddrServer));
	if (nErrMsg < 0)
	{
		
		nStatus = 3;
		return nStatus;
	}
	return nStatus;
}
unsigned int CServer::Run(int quantity)
{
	listen(m_socketKernel, quantity);
	sockaddr_in sockaddrTCP;
	int len = sizeof(sockaddr);
	SOCKET socketClient;
	while(m_socketClients.size() != quantity)
	{
		socketClient = accept(m_socketKernel, (sockaddr*)(&sockaddrTCP), &len);
		if (socketClient == INVALID_SOCKET)
		{
			std::printf("Sequence over.\n");
			break;
		}
		else
		{
			m_socketClients.push_back(socketClient);
			ioctlsocket(socketClient, FIONBIO, (u_long*)&nMode);
		}
	}
	return m_socketClients.size();
}

void CServer::Shut()
{
	for (auto k : m_socketClients)
	{
		closesocket(k);
	}
	closesocket(m_socketKernel);
	int nRet;
	do
	{
		nRet = WSACleanup();
	} while (nRet != WSANOTINITIALISED);
}

int CServer::SendMsg(const char * msg, int len, int seq)
{
	int nStatus = 0;
	int nErrMsg = send(m_socketClients[seq], msg, len, 0);
	if (nErrMsg < 0)
	{
		
		nStatus = 1;
	}
	return nStatus;
}

int CServer::RecvMsg(char * msg, int len, int seq)
{
	int nStatus = 0;
	int nErrMsg = recv(m_socketClients[seq], msg, len, 0);
	if (nErrMsg < 0)
	{
		nStatus = 1;
	}
	return nStatus;
}

CServer::CServer(int mode)
{
	nMode = mode;
}
