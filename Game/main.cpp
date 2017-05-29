#include "Server.h"
#include "gameinfo.h"

#include <iostream>
#include <fstream>

#define DEFAULT_IP "127.0.0.1"  
#define MAX_PATH 260  
#define PORT_NUM 8080

using namespace std;

int MainLogic(const char*, int);
string GetLocalIP();

int main()
{
	string strAddr(GetLocalIP());
	//int nResult = MainLogic(strAddr.c_str(), PORT_NUM);
	//cout << nResult << endl;
	CServer server(WAITALL);
	server.Init(PORT_NUM, "183.172.218.23");
	server.Run(1);
	char msg[11] = "Connected.";
	server.SendMsg(msg, 11, 0);
	system("pause");
	server.Shut();
	return 0;
}


string GetLocalIP()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(1, 1);
	WSAStartup(wVersionRequested, &wsaData);
	char hostname[MAX_PATH] = { 0 };
	gethostname(hostname, MAX_PATH);
	LPHOSTENT lpHostEnt = gethostbyname(hostname);
	if (lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];
	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);
	WSACleanup();
	return string(inet_ntoa(inAddr));
}