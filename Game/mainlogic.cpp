#include "character.h"
#include "gameinfo.h"
#include "Server.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sstream>

using namespace std;

//Some global constants
const int PLAYER_NUM = 8;	//Total players
const int REFUSE = 479;
const int WOLF_NUM = 3;
const int VILL_NUM = 5;
//Some global references

//Response Message 
vector<string> vecResponseList
{
	"_K","_T","_A","_B",
	"_E","_I","_N","_P",
	"_W","_M","_X","_H"
};

enum enResponseList 
{
	RM_KILL, RM_TALK, RM_ANTIDOTE, RM_BADGE,
	RM_EXILE, RM_INDICATE, RM_NOTE, RM_POISSON,
	RM_WITCH, RM_MESSAGE,RM_XOHTER, RM_HUNTER
};

//Command Message
vector<string> vecCommandList
{
	"_S","_K","_B","_E",
	"_I","_P","_A","_W",
	"_H","_N","_C","_T",
	"_X","_R"
};

enum enCommandList
{
	CM_SHOW, CM_KILL, CM_BADGE, CM_EXILE,
	CM_INDICATE, CM_POISSON, CM_ANTITODE, CM_WITCH,
	CM_HUNTER, CM_NOTE, CM_CHARACTER, CM_TALK,
	CM_XOTHER,CM_RESULT
};

//Vectors need to delegate and get the maximum 
enum enDelegate
{
	VD_KILL,VD_BADGE,VD_EXILE
};

//Identity Decision by random
void RandomPick(vector<PlayerInfo> & players, CServer & server)
{
	vector<int> vecIdentity = { villager, villager, werewolf, werewolf, werewolf, witch, hunter, prophet };
	srand(time(nullptr));
	int nLimit = players.size();
	while (nLimit > 0)
	{
		int nMark = rand() % nLimit;
		players[nLimit - 1].m_nch = vecIdentity[nMark];
		vecIdentity.erase(vecIdentity.begin() + nMark);
		--nLimit;
	}
	for (int k = 0;k < nLimit;++k)
	{
		PlayerInfo &p = players[k];
		string strIdentity;
		switch (p.m_nch)
		{
		case villager:
			p.m_strIdentity = "Villager";
			break;
		case werewolf:
			p.m_strIdentity = "Werewolf";
			break;
		case witch:
			p.m_strIdentity = "Witch";
			break;
		case hunter:
			p.m_strIdentity = "Hunter";
			break;
		case prophet:
			p.m_strIdentity = "Prophet";
			break;
		default:
			break;
		}
		//Sendmsg of identity
		string msg1("_C|"), msg2("_S|Your identity is ");
		msg1 += p.m_strIdentity;
		msg2 += p.m_strIdentity;
		server.SendMsg(msg1.c_str(), LEN(msg1), p.GetID());
		server.SendMsg(msg2.c_str(), LEN(msg2), p.GetID());
	}
}

//Parametres generator
enum enGen
{
	PG_ALIVE, PG_DYING
};

string ParamGenerate(vector<PlayerInfo> & players, int nKey, stringstream & ss)
{
	string strParam;
	switch (nKey)
	{
	case PG_ALIVE:
		for (auto p : players)
		{
			ss.clear();
			if (p.m_stateSelf.bAlive)
			{
				ss << p.GetID();
			}
			strParam += ss.str() + string(" ");
		}
		break;
	case PG_DYING:
		for (auto p : players)
		{
			ss.clear();
			if (p.m_stateSelf.bDying)
			{
				ss << p.GetID();
			}
			strParam += ss.str() + string(" ");
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
	for (auto p : players)
	{
		server.SendMsg(msg.c_str(),LEN(msg), p.GetID());
	}
}

//Send message to a special group
void GroupRadio(CServer &server, const string &msg, vector<PlayerInfo> & players,int nChara)
{
	for (auto p : players)
	{
		if (p.m_nch == nChara && p.m_stateSelf.bAlive)
		{
			server.SendMsg(msg.c_str(), LEN(msg), p.GetID());
		}
	}
}

//Recieve messages from a special group
void GroupGet(CServer & server, vector<string> & vecResp, vector<PlayerInfo> & players, int nChara)
{
	for (auto p : players)
	{
		if (p.m_nch == nChara && p.m_stateSelf.bAlive)
		{
			char szBuffer[100];
			server.RecvMsg(szBuffer, 100, p.GetID());
			vecResp.push_back(szBuffer);
		}
	}
}

//Recieve messages globally
void GlobalGet(CServer & server, vector<string> & vecResp, vector<PlayerInfo> & players)
{
	for (auto p : players)
	{
		if (p.m_stateSelf.bAlive)
		{
			char szBuffer[100];
			server.RecvMsg(szBuffer, 100, p.GetID());
			vecResp.push_back(szBuffer);
		}
	}
}

//Parse string
void Parse(const string & str,vector<PlayerInfo> & vecPlayers,vector<vector<int>> & vecSet, CServer &server)
{
	if (str.empty())
	{
		return;
	}
	string strResp, strPara;
	strResp = str.substr(0, str.find("|"));
	strPara = str.substr(1 + str.find("|"));
	DoParametres(stoi(strResp), strPara, vecPlayers ,vecSet, server);
}

//Do something to proceed parametres
void DoParametres
(
	int nResp,	//Case of Response
	const string & str,	//Parametre of Response
	vector<PlayerInfo> & vecPlayers,	//Players of the game
	vector<vector<int>>  & vecSet,		//Delegate
	CServer & server			//Send message
)
{
	stringstream ss;
	switch (nResp) {
	case RM_KILL:
		vecSet[VD_KILL].push_back(stoi(str));
		break;
	case RM_TALK:
		GlobalRadio(server, str, vecPlayers);
		break;
	case RM_ANTIDOTE:
		vecPlayers[stoi(str)].m_stateSelf.bDying = false;
		break;
	case RM_BADGE:
		vecSet[VD_BADGE].push_back(stoi(str));
		break;
	case RM_EXILE:
		vecSet[VD_EXILE].push_back(stoi(str));
		break;
	case RM_INDICATE:
	{
		string msgProphet("_S|This player's identity is ");
		msgProphet += vecPlayers[stoi(str)].m_strIdentity;
		GroupRadio(server, msgProphet, vecPlayers, prophet);
		break;
	}
	case RM_NOTE:
		GlobalRadio(server, str, vecPlayers);
		break;
	case RM_POISSON:
		vecPlayers[stoi(str)].m_stateSelf.bDying = true;
		break;
	case RM_WITCH:
	{
		string msgWitch;
		for (auto p : vecPlayers)
		{
			if (p.m_nch == witch)
			{
				switch (stoi(str))
				{
				case 1:
				{
					msgWitch = string("_P|") + ParamGenerate(vecPlayers, PG_ALIVE, ss);
				}
				break;
				case 2:
				{
					msgWitch = string("_A|") + ParamGenerate(vecPlayers, PG_DYING, ss);
				}
				break;
				default:
					break;
				}
			}
		}
		GroupRadio(server, msgWitch, vecPlayers, witch);
		break;
	}
	case RM_XOHTER:
	{
		string msg;
		if (stoi(str) != REFUSE)
		{
			for (auto p : vecPlayers)
			{
				if (p.GetID() == stoi(str))
				{
					p.m_stateSelf.bBadged = true;
					msg = "_S|Player ";
					ss.clear();
					ss << p.GetID();
					msg += ss.str() + " " + p.GetName() + "is the new police.";
					GlobalRadio(server, msg, vecPlayers);
				}
			}
		}
		else
		{
			msg = "_S|The badge is ruinned!";
			GlobalRadio(server, msg, vecPlayers);
		}
		break;
	}
	case RM_HUNTER:
	{
		vecPlayers[stoi(str)].m_stateSelf.bDying = true;
		break;
	}
	default:
		break;
	}
}

//Find most occurance
int Regress(vector<int> set)
{
	int base[PLAYER_NUM];
	int retCode = set[0];
	for (auto i : set)
	{
		++base[i];
	}
	int tmp = 0;
	for (int i = 0; i < PLAYER_NUM; ++i)
	{
		if (base[i] >= base[tmp])
		{
			tmp = i;
		}
	}
	if (base[tmp] != 0)
	{
		retCode = tmp;
	}
	return retCode;
}

//Check the survivors
int EndGame(vector<PlayerInfo> &pl)
{
	int nEndCode = 0;
	int nWolfDead = 0;
	int nVillDead = 0;
	for (auto k : pl)
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

//Show round
void ShowRound(bool bNight,CServer & server,stringstream & ss,int nRound,vector<PlayerInfo> &vecPlayers)
{
	string strRound = string("_S|Round ");
	ss.clear();
	ss << nRound;
	strRound += ss.str();
	strRound += (bNight ? string(" Night") : string(" Day"));
	GlobalRadio(server, strRound, vecPlayers);
}

//The main procedure of the game
int MainLogic()
{
	//Game Init
	stringstream ss;
	CServer server;
	int nRound = 0;		//Counter of rounds
	bool bNight = false;
	int nGameEnd = 0;	//0 = Not End 1 = Villager Win 2 = Werewolf Win
	//PlayerInfo Init
	vector<TID> vecId, vector<string> vecName;	//Distribute IDs and Names
	for (int k = 0; k < PLAYER_NUM; ++k)
	{
		vecId.push_back(k);
	}
	int status = server.Init(8888, "127.0.0.1");	//Open a server
	if (status == 0)
	{
		server.Run(PLAYER_NUM);		//Link to every clients
	}
	else
	{
		cout << "Init failed. CODE = " << status << endl;
		return status;		// Quit if connection failed
	}
	//Send message to collect the names
	string msgName("_S|Please input your names:");
	for (int k = 0; k < PLAYER_NUM; ++k)
	{
		server.SendMsg(msgName.c_str(), LEN(msgName), k);
	}
	for (int k = 0; k < PLAYER_NUM; ++k)
	{
		char szBuffer[20];
		server.RecvMsg(szBuffer, 20, k);
		string strBuffer(szBuffer);
		string strResp, strPara;
		strResp = strBuffer.substr(0, strBuffer.find("|"));
		strPara = strBuffer.substr(1 + strBuffer.find("|"));
		if (stoi(strResp) == RM_MESSAGE)
		{
			vecName.push_back(strPara);
		}
	}
	vector<PlayerInfo> vecPlayers;	//Initialize the players
	for (int k = 0; k < vecId.size(); ++k)
	{
		vecPlayers.push_back(PlayerInfo(vecName[k], vecId[k]));
	}
	//Game Loop
	while (nGameEnd == 0)
	{
		if (nRound == 0)
		{
			RandomPick(vecPlayers,server);		//round 0 to pick characters
			GlobalRadio(server, string("_S|Game started!"), vecPlayers);
			++nRound;	//Start from the first Night
		}
		//Delegate to the mainlogic to calculate the one
		vector<int> vecToKill, vecToBadge, vecToExile;
		vector<vector<int>> vecSet{ vecToKill, vecToBadge, vecToExile };
		//Night Phase
		bNight = true;
		//Show DayNight and round
		ShowRound(bNight, server, ss, nRound, vecPlayers);
		string strAlive;	//Generate a param string of alive players' IDs
		strAlive = ParamGenerate(vecPlayers, PG_ALIVE, ss);
		//WEREWOLF
		string msgWolf = string("_K|") + strAlive;
		GroupRadio(server, msgWolf, vecPlayers, werewolf);
		vector<string> vecRespWolf;
		GroupGet(server, vecRespWolf, vecPlayers, werewolf);
		for (auto r : vecRespWolf)
		{
			Parse(r, vecPlayers, vecSet, server);
		}
		int nKill = Regress(vecSet[VD_KILL]);
		vecPlayers[nKill].m_stateSelf.bDying = true;	//Sum of the wolf killing
		//PROPHET
		string msgProp = string("_I|") + strAlive;
		GroupRadio(server, msgProp, vecPlayers, prophet);
		vector<string> vecRespProphet;
		GroupGet(server, vecRespProphet, vecPlayers, prophet);
		for (auto r : vecRespProphet)
		{
			Parse(r, vecPlayers, vecSet, server);
		}
		//WITCH
		string msgWitch = string("_W|") + strAlive;
		GroupRadio(server, msgWitch, vecPlayers, witch);
		vector<string> vecRespWitch, vecWitch;
		GroupGet(server, vecRespWitch, vecPlayers, witch);
		for (auto r : vecRespWitch)
		{
			Parse(r, vecPlayers, vecSet, server);
		}
		GroupGet(server, vecWitch, vecPlayers, witch);
		for (auto r : vecWitch)
		{
			Parse(r, vecPlayers, vecSet, server);
		}
		//DAWN
		//Show DayNight and round
		ShowRound(bNight, server, ss, nRound, vecPlayers);
		//First day select a police
		if (nRound == 1)
		{
			string msgBadge("_B|");
			msgBadge += strAlive;
			GlobalRadio(server, msgBadge, vecPlayers);
			vector<string> vecRespBadge;
			for (auto p : vecPlayers)
			{
				if (p.m_stateSelf.bAlive)
				{
					char szBuffer[100];
					server.RecvMsg(szBuffer, 100, p.GetID());
					vecRespBadge.push_back(szBuffer);
				}
			}
			for (auto b : vecRespBadge)
			{
				Parse(b, vecPlayers, vecSet, server);
			}
			int nBadge = Regress(vecSet[VD_BADGE]);
			vecPlayers[nBadge].m_stateSelf.bBadged = true;
			string strBadge("_S|Player ");
			ss.clear();
			ss << nBadge;
			strBadge += ss.str() + string(" ") + vecPlayers[nBadge].GetName() + string("is elected as Police.");
			GlobalRadio(server, strBadge, vecPlayers);
		}
		//NOTE
		PlayerInfo *plDead = nullptr, *plKill = nullptr;
		for (auto p : vecPlayers)
		{
			if (p.m_stateSelf.bDying)
			{
				string msgNote("_S|Last night Player ");
				msgNote += ParamGenerate(vecPlayers, PG_DYING, ss) + string(" ") + p.GetName() + string(" was killed.");
				GlobalRadio(server, msgNote, vecPlayers);
				plDead = &p;
				p.m_stateSelf.bAlive = false;
				p.m_stateSelf.bDying = false;
			}
		}
		//End check
		nGameEnd = EndGame(vecPlayers);
		if (nGameEnd != 0)
		{
			break;
		}
		strAlive = ParamGenerate(vecPlayers, PG_ALIVE, ss);
		if (plDead != nullptr)	//Someone is killed
		{
			if (plDead->m_stateSelf.bBadged)	//Move badge
			{
				string strXOther("_X|");
				strXOther += strAlive;
				server.SendMsg(strXOther.c_str(), LEN(strXOther), plDead->GetID());
				char sz[100];
				server.RecvMsg(sz, 100, plDead->GetID());
				Parse(string(sz), vecPlayers, vecSet, server);
			}
			if (plDead->m_nch == hunter)	//Hunter's phase
			{
				string msgHunter("_H|");
				msgHunter += strAlive;
				server.SendMsg(msgHunter.c_str(), LEN(msgHunter), plDead->GetID());
				char szHunter[100];
				server.RecvMsg(szHunter, 100, plDead->GetID());
				Parse(string(szHunter), vecPlayers, vecSet, server);
				for (auto p : vecPlayers)
				{
					if (p.m_stateSelf.bDying)
					{
						plKill = &p;
					}
				}
				if (plKill != nullptr)
				{
					string strHunter("_S|Hunter chose to kill player ");
					ss.clear();
					ss << plKill->GetID();
					strHunter += ss.str() + string(" ") + plKill->GetName();
					GlobalRadio(server, strHunter, vecPlayers);
					plKill->m_stateSelf.bAlive = false;
					plKill->m_stateSelf.bDying = false;
				}
				else
				{
					string strNoHunt("_S|Hunter did not kill anyone.");
					GlobalRadio(server, strNoHunt, vecPlayers);
				}
			}
			//Note
			string msgNote("_N|");
			server.SendMsg(msgNote.c_str(), LEN(msgNote), plDead->GetID());
			char szNote[100];
			server.RecvMsg(szNote, 100, plDead->GetID());
			Parse(string(szNote), vecPlayers, vecSet, server);
			if (plKill != nullptr)	
			{
				server.SendMsg(msgNote.c_str(), LEN(msgNote), plKill->GetID());
				char szNote2[100];
				server.RecvMsg(szNote2, 100, plKill->GetID());
				Parse(string(szNote2), vecPlayers, vecSet, server);
			}
		}
		else	//No one died
		{
			string strPeace("_S|No one was killed,last night was peace.");
			GlobalRadio(server, strPeace, vecPlayers);
		}
		//Speak globally
		vector<string> vecTalk;
		string msgTalk("_T|");
		strAlive = ParamGenerate(vecPlayers, PG_ALIVE, ss);
		msgTalk += strAlive;
		GlobalRadio(server, msgTalk, vecPlayers);
		GlobalGet(server, vecTalk, vecPlayers);
		for (auto t : vecTalk)
		{
			Parse(t, vecPlayers, vecSet, server);
		}
		//EXILE
		vector<string> vecExile;
		string msgExile("_E|");
		msgExile += strAlive;
		GlobalRadio(server, msgExile, vecPlayers);
		GlobalGet(server, vecExile, vecPlayers);
		for (auto t : vecExile)
		{
			Parse(t, vecPlayers, vecSet, server);
		}
		int nExile = Regress(vecSet[VD_EXILE]);
		string strExile("_S|Player ");
		ss.clear();
		ss << nExile;
		strExile += ss.str() + string(" ") + vecPlayers[nExile].GetName() + string(" is exiled!");
		GlobalRadio(server, strExile, vecPlayers);
		vecPlayers[nExile].m_stateSelf.bAlive = false;
		//End check
		nGameEnd = EndGame(vecPlayers);
		if (nGameEnd != 0)
		{
			break;
		}
	}
	switch (nGameEnd)
	{
	case 1:
	{
		string strWolf("_R|Werewolves win!");
		GlobalRadio(server, strWolf, vecPlayers);
		break;
	}
	case 2:
	{
		string strHuman("_R|Hunman win!");
		GlobalRadio(server, strHuman, vecPlayers);
		break;
	}
	default:
		break;
	}
	server.Shut();
	return nRound;
}
