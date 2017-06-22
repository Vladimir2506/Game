//Name : main.cpp
//Author:  ÏÄ×¿·²
//Date : 2017-05-21
//Description : The entry point of the application
//Copyright : All by author
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
	cout << "SERVER IP = " << strAddr << endl;
	cout << "PORT = " << PORT_NUM << endl;
	int nResult = MainLogic(strAddr.c_str(), PORT_NUM);
	cout << nResult << endl;
	return 0;
}

//Get server ip form the system
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