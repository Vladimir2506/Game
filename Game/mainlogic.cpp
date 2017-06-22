//Name : mainlogic.cpp
//Author:  夏卓凡
//Date : 2017-05-07
//Description : mainlogic of the game
//Copyright : All by author
#include "gameinfo.h"
#include "Server.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iostream>

using namespace std;

//A pre-func
#define LEN(s) 1+s.size()

//Some global constants
const int PLAYER_NUM = 8;	//Total players
const int REFUSE = 951;		//A constant to mark doing nothing
const int WOLF_NUM = 3;		//Total werewolf
const int VILL_NUM = 5;		//Total Human
const int MAX_SIZE = 512;	//Max size of buffer
const int MAX_NOTE = 4;		//Max size of notes
const int GOOD = 0;			//Good man
const int BAD = 1;			//Bad man
const int PUB = 0;			//Public msg
const int PRV = 1;			//Private msg
const int TIME_DIV = 150;	//Time Div
const int TIME_WAIT = 50;
//Response Message 
vector<string> vecResponseList
{
	"_K",	//RM_KILL = Werewolf choose someone to kill
	"_T",	//RM_TALK = Cross talk message
	"_A",	//RM_ANTIDOTE = Witch uses antidote
	"_B",	//RM_BADGE = Police election
	"_E",	//RM_EXILE = Exile vote
	"_I",	//RM_INDICATE = Prophet identify someone
	"_N",	//RM_NOTE = The dying man leaves note
	"_P",	//RM_POISON = Witch uses poison
	"_W",	//RM_WITCH = Witch does nothing
	"_X",	//RM_XOHTER = A police dies and turns his badge
	"_H",	//RM_HUNTER = A hunter dies and shoots
	"_G",	//RM_GROUP = Werewolves Grouptalk Ends
	"_ST"	//RM_STOP = Talk stopping
};

enum enResponseList
{
	RM_KILL, RM_TALK, RM_ANTIDOTE, RM_BADGE,
	RM_EXILE, RM_INDICATE, RM_NOTE, RM_POISON,
	RM_WITCH, RM_XOHTER, RM_HUNTER,
	RM_GROUP, RM_STOP
};

//Command Message
vector<string> vecCommandList
{
	"_S|",	//CM_SHOW = Show parametre string to process
	"_K|",	//CM_KILL = Want return the victim's ID
	"_B|",	//CM_BADGE = Want return ID vote for police
	"_E|",	//CM_EXILE = Want return ID vote for exile
	"_I|",	//CM_INDICATE = Want return ID to identify
	"_W|",	//CM_WITCH = witch
	"_H|",	//CM_HUNTER = Want return ID to shoot
	"_N|",	//CM_NOTE = Tell all players who will address and want return the note
	"_C|",	//CM_CHARACTER = Tell the result of pick character
	"_T|",	//CM_TALK = Want return talking
	"_X|",	//CM_XOTHER = Want to Move badge
	"_R|",	//CM_RESULT = End Game and Show Result	
	"_G|",	//CM_GROUP = Begin a Group talk
	"_YA|",	//CM_SYNCA = Sync alive
	"_YD|",	//CM_SYNCD = Sync dying
	"_IR|",	//CM_IDRES = Result of Indication
	"_V|",	//CM_VOTE = Vote case
	"_SB|",	//CM_SYNCBADGE = Badge state need to update
	"_ST|",	//CM_STOP = Stop talking
	"_PU|",	//CM_PUBLIC = Show string to public
	"_TE|"	//CM_TEAM = Show string to team
};

enum enCommandList
{
	CM_SHOW, CM_KILL, CM_BADGE, CM_EXILE,
	CM_INDICATE, CM_WITCH,
	CM_HUNTER, CM_NOTE, CM_CHARACTER, CM_TALK,
	CM_XOTHER, CM_RESULT, CM_GROUP, CM_SYNCA,
	CM_SYNCD, CM_IDRES, CM_VOTE,
	CM_SYNCBADGE, CM_STOP, CM_PUBLIC ,CM_TEAM
};

//Vectors need to delegate and get the maximum 
enum enDelegate
{
	//Three case to find most
	VD_KILL,VD_BADGE,VD_EXILE
};

void DoParametres(int, string &, vector<PlayerInfo>&, vector<int>&, CServer &);
void RandomPick(vector<PlayerInfo>&, CServer&);
string ParamGenerate(vector<PlayerInfo> &, int);
string ParamGenerate(vector<int>);
string BinaryGenerate(vector<PlayerInfo>&, int);
void GlobalRadio(CServer &, const string &, vector<PlayerInfo> &);
void GroupRadio(CServer &, const string &, vector<PlayerInfo> &, int);
void GroupGet(CServer &, vector<string> &, vector<PlayerInfo> &, int);
void GlobalGet(CServer &, vector<string> &, vector<PlayerInfo> &,bool);
void Parse(const string &, vector<PlayerInfo> &, vector<int> &, CServer &);
vector<int> FindMost(vector<int> &, vector<PlayerInfo> &, bool);
int EndGame(vector<PlayerInfo> &);
void ShowRound(bool, CServer &, int, vector<PlayerInfo> &);
void Sync(vector<PlayerInfo> &, CServer &);
void FreeSpeech(vector<PlayerInfo> &, CServer &);
void GroupComm(vector<PlayerInfo> &, CServer &, int);
void SingleCry(vector<PlayerInfo> &, CServer &, int);
int MainLogic(char *, int);
void MoveBadge(CServer & server, vector<PlayerInfo> &vecPlayers, int nId);
void HunterShoot(CServer & server, vector<PlayerInfo> & vecPlayers, int nId, bool bSpeech, int &note);
string Reveal(vector<PlayerInfo> &);
void Diedelay(vector<PlayerInfo> &players, int mch);

//Identity Decision by Random
void RandomPick(vector<PlayerInfo> & players, CServer & server)
{
	vector<int> vecIdentity = { villager, villager, werewolf, werewolf, werewolf, hunter, prophet, witch};
	int nLimit = players.size();
	while (nLimit > 0)
	{
		int nMark = rand() % nLimit;
		players[nLimit - 1].m_nch = vecIdentity[nMark];
		vecIdentity.erase(vecIdentity.begin() + nMark);
		--nLimit;
	}
	for (auto &p:players)
	{
		//Sendmsg of identity
		string msgId(vecCommandList[CM_CHARACTER]);
		msgId += to_string(p.GetID()) + " " + to_string(p.m_nch);
		server.SendMsg(msgId.c_str(), LEN(msgId), p.GetID());
	}
}

//Parametres generator
enum enGen
{
	PG_ALIVE, PG_DYING
};

//Generate a string of ids by alive/dying
string ParamGenerate(vector<PlayerInfo> & players, int nKey)
{
	string strParam;
	switch (nKey)
	{
	case PG_ALIVE:
		for (auto &p : players)
		{
			if (p.m_stateSelf.bAlive)
			{
				strParam += to_string(p.GetID()) + string(" ");
			}
		}
		break;
	case PG_DYING:
		for (auto &p : players)
		{
			if (p.m_stateSelf.bDying)
			{
				strParam += to_string(p.GetID()) + string(" ");
			}
		}
		break;
	default:
		break;
	}
	return strParam;
}

//Generate a string of ids
string ParamGenerate(vector<int> vec)
{
	string strParam;
	for (auto &i : vec)
	{
		strParam += to_string(i) + string(" ");
	}
	return strParam;
}

//Generate a string of status by 0/1
string BinaryGenerate(vector<PlayerInfo> & players,int nKey)
{
	string strParam;
	switch (nKey)
	{
	case PG_ALIVE:
		for (int i = 0; i < PLAYER_NUM; ++i)
		{
			if (players[i].m_stateSelf.bAlive)
			{
				strParam += "1";
			}
			else
			{
				strParam += "0";
			}
		}
		break;
	case PG_DYING:
		for (int i = 0; i < PLAYER_NUM; ++i)
		{
			if (players[i].m_stateSelf.bDying)
			{
				strParam += "1";
			}
			else
			{
				strParam += "0";
			}
		}
		break;
	default:
		break;
	}
	return strParam;
}

//Send message to a series of clients
void GlobalRadio(CServer & server, const string & msg, vector<PlayerInfo> & players)
{
	for (auto &p : players)
	{
		Sleep(TIME_DIV);
		server.SendMsg(msg.c_str(),LEN(msg), p.GetID());
	}
}

//Send message to a special group
void GroupRadio(CServer &server, const string &msg, vector<PlayerInfo> & players,int nChara)
{
	for (auto &p : players)
	{
		if (p.m_nch == nChara && p.m_stateSelf.bAlive)
		{
			Sleep(TIME_DIV);
			server.SendMsg(msg.c_str(), LEN(msg), p.GetID());
		}
	}
}

//Recieve messages from a special group
void GroupGet(CServer & server, vector<string> & vecResp, vector<PlayerInfo> & players, int nChara)
{
	for (auto &p : players)
	{
		if (p.m_nch == nChara && p.m_stateSelf.bAlive)
		{
			char szBuffer[MAX_SIZE];
			server.RecvMsg(szBuffer, MAX_SIZE, p.GetID());
			vecResp.push_back(szBuffer);
		}
	}
}

//Recieve messages globally
void GlobalGet(CServer & server, vector<string> & vecResp, vector<PlayerInfo> & players,bool bExile)
{
	for (unsigned int i = 0;i < players.size(); ++i)
	{
		if (players[i].m_stateSelf.bAlive)
		{
			char szBuffer[MAX_SIZE];
			server.RecvMsg(szBuffer, MAX_SIZE, i);
			vecResp.push_back(szBuffer);
		}
		else
		{
			if (bExile)
			{
				vecResp.push_back(vecCommandList[CM_EXILE] + to_string(1 + i) + "~"+ to_string(REFUSE + 1));
			}
			else
			{
				vecResp.push_back(vecCommandList[CM_BADGE] + to_string(1 + i) + "~"+ to_string(REFUSE + 1));
			}
		}
	}
}

//Parse string
void Parse(const string & str,vector<PlayerInfo> & vecPlayers,vector<int> & vecTogo, CServer &server)
{
	if (str.empty())
	{
		return;
	}
	int nResp = 0;
	string strResp, strPara;
	strResp = str.substr(0, str.find("|"));
	strPara = str.substr(1 + str.find("|"));
	for (unsigned int k = 0;k < vecResponseList.size(); ++k)
	{
		if (strResp == vecResponseList[k])
		{
			nResp = k;
		}
	}
	DoParametres(nResp, strPara, vecPlayers, vecTogo, server);
}

//Do something to proceed parametres
void DoParametres
(
	int nResp,	//Case of Response
	string & str,	//Parametre of Response
	vector<PlayerInfo> & vecPlayers,	//Players of the game
	vector<int> & vecTogo,		//Delegate
	CServer & server			//Send message
)
{
	switch (nResp) 
	{
	case RM_KILL:
		vecTogo.push_back(stoi(str));
		break;
	case RM_TALK:
	{
		switch (vecTogo[0])
		{
		case PUB:
			GlobalRadio(server, vecCommandList[CM_PUBLIC] + str, vecPlayers);
			break;
		case PRV:
			GlobalRadio(server, vecCommandList[CM_TEAM] + str, vecPlayers);
			break;
		default:
			break;
		}
		break;
	}
	case RM_ANTIDOTE:
		if (stoi(str) != REFUSE)
		{
			vecPlayers[stoi(str)].m_stateSelf.bDying = false;
		}
		break;
	case RM_BADGE:
	{
		GlobalRadio(server, vecCommandList[CM_VOTE] + str, vecPlayers);
		string strVotee = str.substr(1 + str.find("~"));
		vecTogo.push_back(stoi(strVotee) - 1);
		break;
	}
	case RM_EXILE:
	{
		GlobalRadio(server, vecCommandList[CM_VOTE] + str, vecPlayers);
		string strVotee = str.substr(1 + str.find("~"));
		vecTogo.push_back(stoi(strVotee) - 1);
		break;
	}
	case RM_INDICATE:
	{
		if (stoi(str) != REFUSE)
		{
			string msgProphet(vecCommandList[CM_IDRES]);
			msgProphet += str + " ";
			if (vecPlayers[stoi(str)].m_nch == werewolf)
			{
				msgProphet += to_string(BAD);
			}
			else
			{
				msgProphet += to_string(GOOD);
			}
			GroupRadio(server, msgProphet, vecPlayers, prophet);
		}
		break;
	}
	case RM_NOTE:
	{
		string strNote(vecCommandList[CM_PUBLIC]);
		GlobalRadio(server, strNote + str, vecPlayers);
		break;
	}
	case RM_POISON:
		if (stoi(str) != REFUSE)
		{
			vecPlayers[stoi(str)].m_stateSelf.bDying= true;
		}
		break;
	case RM_WITCH:	
		break;
	case RM_XOHTER:
	{
		string msg(vecCommandList[CM_SYNCBADGE]);
		if (stoi(str) != REFUSE)
		{
			for (auto &p : vecPlayers)
			{
				if (p.GetID() == stoi(str))
				{
					p.m_stateSelf.bBadged = true;
					msg += to_string(p.GetID());
					GlobalRadio(server, msg, vecPlayers);
				}
				else
				{
					p.m_stateSelf.bBadged = false;
				}
			}
		}
		else
		{
			msg += to_string(REFUSE);
			GlobalRadio(server, msg, vecPlayers);
		}
		break;
	}
	case RM_HUNTER:
		if (stoi(str) != REFUSE)
		{
			vecTogo.push_back(stoi(str));
		}
		break;
	default:
		break;
	}
}

//Find Most Occurance
//And a sheriff has 1.5 weights
vector<int> FindMost(vector<int> & set,vector<PlayerInfo> & pl,bool police)
{
	vector<double> base;
	base.resize(PLAYER_NUM,0.0);
	for (unsigned int i = 0; i < set.size(); ++i)
	{
		if (set[i] != REFUSE)
		{
			if (police)
			{
				if (pl[i].m_stateSelf.bBadged)
				{
					base[set[i]] += 1.5;
				}
				else
				{
					base[set[i]] += 1.0;
				}
			}
			else
			{
				base[set[i]] += 1.0;
			}
		}
	}
	double dMax = 0.0;
	for (auto s : base)
	{
		if (dMax <= s)
		{
			dMax = s;
		}
	}
	vector<int> vecResult;
	for (unsigned int i = 0; i < base.size(); ++i)
	{
		if (base[i] == dMax)
		{
			vecResult.push_back(i);
		}
	}
	return vecResult;
}


//Check the survivors
int EndGame(vector<PlayerInfo> &pl)
{
	int nEndCode = 0;
	int nWolfDead = 0;
	int nVillDead = 0;
	for (auto &k : pl)
	{
		if (k.m_nch == werewolf && k.m_stateSelf.bAlive == false)
		{
			++nWolfDead;
		}
		if (k.m_nch != werewolf && k.m_stateSelf.bAlive == false)
		{
			++nVillDead;
		}
	}
	if (nWolfDead == WOLF_NUM)
	{
		nEndCode = 1;
	}
	if (nVillDead == VILL_NUM)
	{
		nEndCode = 2;
	}
	return nEndCode;
}

//Show Round
void ShowRound(bool bNight,CServer & server,int nRound,vector<PlayerInfo> &vecPlayers)
{
	string strRound = string("_S|第");
	strRound += to_string(nRound) + "回合";
	strRound += (bNight ? string(" 夜晚") : string(" 白天"));
	GlobalRadio(server, strRound, vecPlayers);
}

//Data Syncronization
void Sync(vector<PlayerInfo> & players, CServer & server)
{
	string alv(vecCommandList[CM_SYNCA]), dyi(vecCommandList[CM_SYNCD]);
	alv += (BinaryGenerate(players, PG_ALIVE));
	dyi += (BinaryGenerate(players, PG_DYING));
	GlobalRadio(server, alv, players);
	GlobalRadio(server, dyi, players);
}

//Free Speech
void FreeSpeech(vector<PlayerInfo> & players, CServer & server)
{
	server.Iomanip(DONTWAIT);
	vector<int> vecSec;
	vecSec.push_back(PUB);
	bool bTalk = true;
	for (unsigned int k = 0;k < players.size(); ++k)
	{
		auto &d = players[k];
		if (d.m_stateSelf.bAlive)
		{
			string tmp = vecCommandList[CM_PUBLIC] + "现在是第" + to_string(d.GetID()+1) + "号玩家发言";
			GlobalRadio(server, tmp, players);
			string strAct(vecCommandList[CM_TALK]);
			Sleep(TIME_DIV);
			server.SendMsg(strAct.c_str(), LEN(strAct), k);
			bTalk = true;
			while (bTalk)
			{
				char szBuffer[MAX_SIZE];
				Sleep(TIME_WAIT);
				int nResult = server.RecvMsg(szBuffer, MAX_SIZE, k);
				if (nResult == 0)
				{
					string strDisp(szBuffer);
					string strCtrl = strDisp.substr(0, strDisp.find("|"));
					if (strCtrl != vecResponseList[RM_STOP])
					{
						Parse(strDisp, players, vecSec, server);
					}
					else
					{
						bTalk = false;
					}
				}
			}
		}
	}
	server.Iomanip(WAITALL);
}

//A Single Cry
void SingleCry(vector<PlayerInfo> & players, CServer &server, int nID)
{
	server.Iomanip(DONTWAIT);
	vector<int> vecSec;
	vecSec.push_back(PUB);
	bool bTalk = true;
	string tmp = vecCommandList[CM_PUBLIC] + "现在是第" + to_string(nID + 1) + "号玩家发言";
	GlobalRadio(server, tmp, players);
	string strAct(vecCommandList[CM_NOTE]);
	Sleep(TIME_DIV);
	server.SendMsg(strAct.c_str(), LEN(strAct), nID);
	while (bTalk)
	{
		char szBuffer[MAX_SIZE];
		Sleep(TIME_WAIT);
		int nResult = server.RecvMsg(szBuffer, MAX_SIZE, nID);
		if (nResult == 0)
		{
			string strDisp(szBuffer);
			string strCtrl = strDisp.substr(0, strDisp.find("|"));
			if (strCtrl != vecResponseList[RM_STOP])
			{
				Parse(strDisp, players, vecSec, server);
			}
			else
			{
				bTalk = false;
			}
		}
	}
	server.Iomanip(WAITALL);
}

//Group Communication
void GroupComm(vector<PlayerInfo> & players, CServer & server, int nch)
{
	server.Iomanip(DONTWAIT);
	vector<PlayerInfo> theGroup;
	vector<int> vecSec;
	vecSec.push_back(PRV);
	bool bTalk = true;
	int cnt = 0;
	int nLim = 0;
	for (auto &k : players)
	{
		if (k.m_nch == nch && k.m_stateSelf.bAlive)
		{
			++nLim;
			theGroup.push_back(k);
		}
	}
	GroupRadio(server, vecCommandList[CM_TEAM] + "交流开始", players, werewolf);
	GroupRadio(server, vecCommandList[CM_GROUP], players, werewolf);
	while (bTalk)
	{
		char szBuffer[MAX_SIZE];
		for (auto &d : players)
		{
			if (d.m_nch == nch)
			{
				int nResult = server.RecvMsg(szBuffer, MAX_SIZE, d.GetID());
				if (nResult == 0)
				{
					string strDisp(szBuffer);
					string strCtrl = strDisp.substr(0, strDisp.find("|"));
					if (strCtrl != vecResponseList[RM_STOP])
					{
						Parse(strDisp, theGroup, vecSec, server);
					}
					else
					{
						++cnt;
					}
				}
			}
			if (cnt == nLim)
			{
				bTalk = false;
				break;
			}
		}
	}
	server.Iomanip(WAITALL);
}

//Badge Movement
void MoveBadge(CServer & server, vector<PlayerInfo> &vecPlayers, int nId)
{
	string strXOther(vecCommandList[CM_XOTHER]);
	vector<int> vecDummy;
	strXOther += ParamGenerate(vecPlayers, PG_ALIVE);
	Sleep(TIME_DIV);
	server.SendMsg(strXOther.c_str(), LEN(strXOther), nId);
	char sz[MAX_SIZE];
	server.RecvMsg(sz, MAX_SIZE, nId);
	Parse(sz, vecPlayers, vecDummy, server);
}

//Hunter Shooting
void HunterShoot(CServer & server, vector<PlayerInfo> & vecPlayers, int nId,bool bSpeech,int &note)
{
	vector<int> vecKill;
	string msgHunter(vecCommandList[CM_HUNTER]);
	msgHunter += ParamGenerate(vecPlayers, PG_ALIVE);
	server.SendMsg(msgHunter.c_str(), LEN(msgHunter), nId);
	char szHunter[MAX_SIZE];
	server.RecvMsg(szHunter, MAX_SIZE, nId);
	Parse(szHunter, vecPlayers, vecKill, server);
	if (!vecKill.empty())
	{
		vecPlayers[vecKill[0]].m_stateSelf.bAlive = false;
		Sync(vecPlayers, server);
		if (bSpeech && note < MAX_NOTE)
		{
			SingleCry(vecPlayers, server, vecKill[0]);
			++note;
		}
		if (vecPlayers[vecKill[0]].m_stateSelf.bBadged)
		{
			MoveBadge(server, vecPlayers, vecKill[0]);
		}
	}
}

//The Main Procedure of the Game
int MainLogic(const char *szAddr,int nPort)
{
	//Game Init
	srand(static_cast<unsigned int>(time(nullptr)));
	CServer server(WAITALL);
	int nRound = 0;		//Counter of rounds
	bool bNight = false;
	int nGameEnd = 0;	//0 = Not End 1 = Villager Win 2 = Werewolf Win
	//PlayerInfo Init
	vector<TID> vecId;	//Distribute IDs 
	for (int k = 0; k < PLAYER_NUM; ++k)
	{
		vecId.push_back(k);
	}
	int status = server.Init(nPort ,szAddr);	//Open a server
	if (status == 0)
	{
		server.Run(PLAYER_NUM);		//Link to every clients
	}
	else
	{
		cout << "Init failed. CODE = " << status << endl;
		return status;		// Quit if connection failed
	}
	vector<PlayerInfo> vecPlayers;	//Initialize the players
	for (unsigned int k = 0; k < vecId.size(); ++k)
	{
		vecPlayers.push_back(PlayerInfo(vecId[k]));
	}
	//Game Loop
	int nNote = 0;
	while (nGameEnd == 0)	
	{
		if (nRound == 0)
		{
			RandomPick(vecPlayers,server);		//round 0 to pick characters
			cout << "游戏开始" << endl;
			GlobalRadio(server, string("_S|游戏开始"), vecPlayers);
			++nRound;	//Start from the first Night
		}
		Sync(vecPlayers, server);
		//Delegate to the mainlogic to calculate the on
		vector<vector<int>> vecSet;
		vector<int> vecDummy;
		for (int l = 0; l < 4; ++l)
		{
			vecSet.push_back(vecDummy);
		}
		//Night Phase
		bNight = true;
		//Show DayNight and round
		ShowRound(bNight, server,nRound, vecPlayers);
		string strAlive;	//Generate a param string of alive players' IDs
		strAlive = ParamGenerate(vecPlayers, PG_ALIVE);
		//WEREWOLF
		string strPhaseWerewolf(vecCommandList[CM_PUBLIC]);
		strPhaseWerewolf += "狼人行动阶段";
		GlobalRadio(server, strPhaseWerewolf, vecPlayers);
		//Grouptalk
		GroupComm(vecPlayers, server, werewolf);
		//End talk
		string strKill = strAlive;
		bool bKill = false;
		vector<int> vecKill;
		while (!bKill)
		{
			vecSet[VD_KILL].clear();
			string msgWolf = string(vecCommandList[CM_KILL]) + strKill;
			GroupRadio(server, msgWolf, vecPlayers, werewolf);
			vector<string> vecRespWolf;
			GroupGet(server, vecRespWolf, vecPlayers, werewolf);
			for (auto &r : vecRespWolf)
			{
				Parse(r, vecPlayers, vecSet[VD_KILL], server);
			}
			bool bValid = false;
			for (auto &i : vecSet[VD_KILL])
			{
				if (i != REFUSE)
				{
					bValid = true;
				}
			}
			if (!bValid)
			{
				continue;
			}
			vecKill = FindMost(vecSet[VD_KILL], vecPlayers, false);
			if (vecKill.size() == 1)
			{
				bKill = true;
			}
			else
			{
				strKill = ParamGenerate(vecKill);
				GroupRadio(server, vecCommandList[CM_TEAM] + "重新选择目标", vecPlayers, werewolf);
			}
		}
		int nKill = vecKill[0];
		vecPlayers[nKill].m_stateSelf.bDying = true;	//Sum of the wolf killing
		GroupRadio(server, vecCommandList[CM_TEAM] + "杀死了" + to_string( 1 + nKill), vecPlayers, werewolf);
		//PROPHET
		string strPhaseProphet(vecCommandList[CM_PUBLIC]);
		strPhaseProphet += "预言家行动阶段";
		GlobalRadio(server, strPhaseProphet, vecPlayers);
		string msgProp = string(vecCommandList[CM_INDICATE]);
		GroupRadio(server, msgProp, vecPlayers, prophet);
		vector<string> vecRespProphet;
		GroupGet(server, vecRespProphet, vecPlayers, prophet);
		for (auto &r : vecRespProphet)
		{
			Parse(r, vecPlayers, vecDummy, server);
		}
		Diedelay(vecPlayers, prophet);
		//WITCH
		string strPhaseWitch(vecCommandList[CM_PUBLIC]);
		strPhaseWitch += "女巫行动阶段";
		GlobalRadio(server, strPhaseWitch, vecPlayers);
		string msgWitch(vecCommandList[CM_WITCH]);
		Sync(vecPlayers,server);
		GroupRadio(server, msgWitch, vecPlayers, witch);
		vector<string> vecRespWitch;
		GroupGet(server, vecRespWitch, vecPlayers, witch);
		for (auto w : vecRespWitch)
		{
			Parse(w, vecPlayers, vecDummy, server);
		}
		Diedelay(vecPlayers, prophet);
		//DAWN
		bNight = false;
		//Show DayNight and round
		ShowRound(bNight, server, nRound, vecPlayers);
		//First day police election
		if (nRound == 1)
		{
			string strPhasePolice(vecCommandList[CM_PUBLIC]);
			strPhasePolice += "选举警长阶段 请依次发言";
			GlobalRadio(server, strPhasePolice, vecPlayers);
			vecSet[VD_BADGE].clear();
			FreeSpeech(vecPlayers, server);
			//After every one's statement come their decisions
			int nBadge;
			bool bElected = false;
			string strEle = strAlive;
			vector<int> vecBadge;
			while (!bElected)
			{
				GlobalRadio(server, vecCommandList[CM_PUBLIC] + "投票开始", vecPlayers);
				vecSet[VD_BADGE].clear();
				vecBadge.clear();
				string msgBadge(vecCommandList[CM_BADGE]);
				msgBadge += strEle;
				GlobalRadio(server, msgBadge, vecPlayers);
				vector<string> vecRespBadge;
				GlobalGet(server, vecRespBadge, vecPlayers, false);
				GlobalRadio(server, vecCommandList[CM_SHOW] + "选举警长投票结果为", vecPlayers);
				for (auto b : vecRespBadge)
				{
					Parse(b, vecPlayers, vecSet[VD_BADGE], server);
				}
				bool bValid = false;
				for (auto i : vecSet[VD_BADGE])
				{
					if (i != REFUSE)
					{
						bValid = true;
					}
				}
				if (!bValid)
				{
					continue;
				}
				vecBadge = FindMost(vecSet[VD_BADGE], vecPlayers, false);
				if (vecBadge.size() == 1)
				{
					bElected = true;
				}
				else
				{
					strEle = ParamGenerate(vecBadge);
					GlobalRadio(server, vecCommandList[CM_PUBLIC] + "请重新投票", vecPlayers);
				}
			}
			nBadge = vecBadge[0];
			vecPlayers[nBadge].m_stateSelf.bBadged = true;
			string strBadge(vecCommandList[CM_SYNCBADGE]);
			strBadge += to_string(nBadge);
			GlobalRadio(server, strBadge, vecPlayers);
		}
		//Deal with some death
		PlayerInfo *plKill = nullptr;
		vector<PlayerInfo*> vecpDead;
		for (auto &p : vecPlayers)
		{
			if (p.m_stateSelf.bDying)
			{
				vecpDead.push_back(&p);
				p.m_stateSelf.bAlive = false;
				p.m_stateSelf.bDying = false;
				Sync(vecPlayers, server);
			}
		}
		strAlive = ParamGenerate(vecPlayers, PG_ALIVE);
		if (nRound == 1)
		{
			for (auto &pd : vecpDead)
			{
				if (nNote < MAX_NOTE)
				{
					string strPhaseNote(vecCommandList[CM_PUBLIC]);
					strPhaseNote += "遗言阶段";
					GlobalRadio(server, strPhaseNote, vecPlayers);
					SingleCry(vecPlayers, server, pd->GetID());
					++nNote;
				}
			}
		}
		//End check
		nGameEnd = EndGame(vecPlayers);
		if (nGameEnd != 0)
		{
			break;
		}
		strAlive = ParamGenerate(vecPlayers, PG_ALIVE);
		if (!vecpDead.empty())	//Someone is killed
		{
			for (auto &plDead : vecpDead)
			{
				if (plDead->m_stateSelf.bBadged)	//Move badge
				{
					MoveBadge(server, vecPlayers, plDead->GetID());
				}
				if (plDead->m_nch == hunter)	//Hunter's phase
				{
					HunterShoot(server, vecPlayers, plDead->GetID(), false, nNote);
				}
			}
			Sync(vecPlayers, server);
		}
		else	//No one died
		{
			string strPeace("_S|前一晚是平安夜");
			GlobalRadio(server, strPeace, vecPlayers);
		}
		vecpDead.clear();
		plKill = nullptr;
		nGameEnd = EndGame(vecPlayers);
		if (nGameEnd != 0)
		{
			break;
		}
		//Speak globally before exile
		GlobalRadio(server, vecCommandList[CM_PUBLIC] + "放逐投票阶段 请依次发言",vecPlayers);
		FreeSpeech(vecPlayers, server);
		//EXILE
		vector<int> vecExile;
		bool bExile = false;
		string strNameList = ParamGenerate(vecPlayers, PG_ALIVE);
		int nExile;
		while (!bExile)
		{
			vecSet[VD_EXILE].clear();
			vecExile.clear();
			GlobalRadio(server, vecCommandList[CM_PUBLIC] + "投票开始", vecPlayers);
			vector<string> vecRespExile;
			string msgExile(vecCommandList[CM_EXILE]);
			msgExile += strNameList;
			GlobalRadio(server, msgExile, vecPlayers);
			GlobalGet(server, vecRespExile, vecPlayers, true);
			GlobalRadio(server, vecCommandList[CM_SHOW] + "放逐投票结果为", vecPlayers);
			for (auto& t : vecRespExile)
			{
				Parse(t, vecPlayers, vecSet[VD_EXILE], server);
			}
			bool bValid = false;
			for (auto &i : vecSet[VD_EXILE])
			{
				if (i != REFUSE)
				{
					bValid = true;
				}
			}
			if (!bValid)
			{
				continue;
			}
			vecExile = FindMost(vecSet[VD_EXILE], vecPlayers, true);
			if (vecExile.size() == 1)
			{
				bExile = true;
			}
			else
			{
				strNameList = ParamGenerate(vecExile);
				GlobalRadio(server, vecCommandList[CM_PUBLIC] + "请重新投票", vecPlayers);
			}
		}
		nExile = vecExile[0];
		vecPlayers[nExile].m_stateSelf.bAlive = false;
		Sync(vecPlayers, server);
		nGameEnd = EndGame(vecPlayers);
		if (nGameEnd != 0)
		{
			break;
		}
		//Exile note
		if (nNote < MAX_NOTE)
		{
			string strPhaseNote(vecCommandList[CM_PUBLIC]);
			strPhaseNote += "遗言阶段";
			GlobalRadio(server, strPhaseNote, vecPlayers);
			SingleCry(vecPlayers, server, nExile);
			++nNote;
		}
		//Exile badge movement
		if (vecPlayers[nExile].m_stateSelf.bBadged)	//Move badge
		{
			MoveBadge(server, vecPlayers, nExile);
		}
		//If hunter is exiled,he choose to shoot.
		if (vecPlayers[nExile].m_nch == hunter)
		{
			HunterShoot(server, vecPlayers, nExile, true, nNote);
		}
		Sync(vecPlayers, server);
		//End check
		nGameEnd = EndGame(vecPlayers);
		if (nGameEnd != 0)
		{
			break;
		}
		++nRound;
	}
	//Game ends and Pronounce the result
	string strResult(vecCommandList[CM_RESULT]);
	strResult += Reveal(vecPlayers) + " ";
	switch (nGameEnd)
	{
	case 1:
		strResult += to_string(1);
		break;
	case 2:
		strResult += to_string(2);
		break;
	default:
		break;
	}
	GlobalRadio(server, strResult, vecPlayers);
	Sleep(3000);
	server.Shut();
	cout << "游戏结束" << endl;
	return nRound;
}

//A game ends and all identities are revealed
string Reveal(vector<PlayerInfo> &vecPlayers)
{
	string strIdentity;
	for (auto &p : vecPlayers)
	{
		strIdentity += to_string(p.m_nch);
	}
	return strIdentity;
}

//If a person die and his time will be delayed
void Diedelay(vector<PlayerInfo> &players,int mch)
{
	for (auto &p : players)
	{
		if (p.m_nch == mch && p.m_stateSelf.bAlive == false)
		{
			int nSec = rand() % 5;
			Sleep(5 + nSec);
		}
	}
}
