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
const int MAX_SIZE = 100;	//Max size of buffer
const int MAX_NOTE = 4;		//Max size of notes

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
	"_N",	//RM_NAME = Name the player
	"_X",	//RM_XOHTER = A police dies and turns his badge
	"_H",	//RM_HUNTER = A hunter dies and shoots
	"_G"	//RM_GROUP = Werewolves Grouptalk Ends
};

enum enResponseList
{
	RM_KILL, RM_TALK, RM_ANTIDOTE, RM_BADGE,
	RM_EXILE, RM_INDICATE, RM_NOTE, RM_POISON,
	RM_WITCH, RM_NAME, RM_XOHTER, RM_HUNTER,
	RM_GROUP, CM_VOTE
};

//Command Message
vector<string> vecCommandList
{
	"_S",	//CM_SHOW = Show parametre string
	"_K",	//CM_KILL = Want return the victim's ID
	"_B",	//CM_BADGE = Want return ID vote for police
	"_E",	//CM_EXILE = Want return ID vote for exile
	"_I",	//CM_INDICATE = Want return ID to identify
	"_W",	//CM_WITCH = witch
	"_H",	//CM_HUNTER = Want return ID to shoot
	"_N",	//CM_NOTE = Want return the note
	"_C",	//CM_CHARACTER = Tell the result of pick character
	"_T",	//CM_TALK = Want return talking
	"_X",	//CM_XOTHER = Want to Move badge
	"_R",	//CM_RESULT = End Game and Show Result	
	"_G",	//CM_GROUP = Begin a Group talk
	"_YA",	//CM_SYNCA = Sync alive
	"_YD",	//CM_SYNCD = Sync dying
	"_IR",	//CM_IDRES = Result of Indication
	"_V"	//CM_VOTE = Vote case
};

enum enCommandList
{
	CM_SHOW, CM_KILL, CM_BADGE, CM_EXILE,
	CM_INDICATE, CM_WITCH,
	CM_HUNTER, CM_NOTE, CM_CHARACTER, CM_TALK,
	CM_XOTHER,CM_RESULT,CM_GROUP,CM_SYNCA,
	CM_SYNCD,CM_IDRES, CM_VOTE
};

//Vectors need to delegate and get the maximum 
enum enDelegate
{
	//Three case to find most
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

string ParamGenerate(vector<PlayerInfo> & players, int nKey)
{
	string strParam;
	switch (nKey)
	{
	case PG_ALIVE:
		for (auto p : players)
		{
			if (p.m_stateSelf.bAlive)
			{
				strParam += to_string(p.GetID()) + string(" ");
			}
		}
		break;
	case PG_DYING:
		for (auto p : players)
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

string ParamGenerate(vector<int> vec)
{
	string strParam;
	for (auto i : vec)
	{
		strParam += to_string(i) + string(" ");
	}
	return strParam;
}

string BinaryGenerate(vector<PlayerInfo> players,int nKey)
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
			char szBuffer[MAX_SIZE];
			server.RecvMsg(szBuffer, MAX_SIZE, p.GetID());
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
			char szBuffer[MAX_SIZE];
			server.RecvMsg(szBuffer, MAX_SIZE, p.GetID());
			vecResp.push_back(szBuffer);
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
	string strResp, strPara;
	strResp = str.substr(0, str.find("|"));
	strPara = str.substr(1 + str.find("|"));
	DoParametres(stoi(strResp), strPara, vecPlayers , vecTogo, server);
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
		GlobalRadio(server, string("_S|") + str, vecPlayers);
		break;
	case RM_ANTIDOTE:
		vecPlayers[stoi(str)].m_stateSelf.bDying = false;
		break;
	case RM_BADGE:
	{
		int nVote = VoteRadio(vecPlayers, str, server);
		vecTogo.push_back(nVote);
		break;
	}
	case RM_EXILE:
	{
		int nExile = VoteRadio(vecPlayers, str, server);
		vecTogo.push_back(nExile);
		break;
	}
	case RM_INDICATE:
	{
		string msgProphet("_IR|This player's identity is ");
		msgProphet += vecPlayers[stoi(str)].m_strIdentity;
		GroupRadio(server, msgProphet, vecPlayers, prophet);
		break;
	}
	case RM_NOTE:
		if (str != "")
		{
			string strNote("_S|Here is the note:");
			GlobalRadio(server, strNote + str, vecPlayers);
		}
		break;
	case RM_POISON:
		vecPlayers[stoi(str)].m_stateSelf.bDying = true;
		break;
	case RM_WITCH:
		//Do nothing.
		break;
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
					msg += to_string(p.GetID()) + " " + p.GetName() + "is the new police.";
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
vector<int> FindMost(vector<int> & set,vector<PlayerInfo> & pl,bool police)
{
	vector<double> base;
	base.resize(PLAYER_NUM,0);
	for (int i = 0; i < PLAYER_NUM; ++i)
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
	double dMax = set[0];
	for (auto s : base)
	{
		if (dMax <= s)
		{
			dMax = s;
		}
	}
	vector<int> vecResult;
	for (int i = 0; i < PLAYER_NUM; ++i)
	{
		if (set[i] == dMax)
		{
			vecResult.push_back(i);
		}
	}
	return vecResult;
}

//Vote case
int VoteRadio(vector<PlayerInfo> players,string & strVote, CServer & server)
{
	string strVoter = strVote.substr(0,strVote.find("~"));
	string strVotee = strVote.substr(1 + strVote.find("~"));
	int nVoter = stoi(strVoter), nVotee = stoi(strVotee);
	string msgVote = string("_V|Player ") + strVoter + string(" ") + players[nVoter].GetName() + string(" votes") + strVotee + string(" ") + players[nVotee].GetName() + string(".");
	GlobalRadio(server, msgVote, players);
	return nVotee;
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
void ShowRound(bool bNight,CServer & server,int nRound,vector<PlayerInfo> &vecPlayers)
{
	string strRound = string("_S|Round ");
	strRound += to_string(nRound);
	strRound += (bNight ? string(" Night") : string(" Day"));
	GlobalRadio(server, strRound, vecPlayers);
}

//Data Syncronization
void Sync(vector<PlayerInfo> & players, CServer & server)
{
	string alv("_YA|"), dyi("_YD|");
	alv += (BinaryGenerate(players, PG_ALIVE));
	dyi += (BinaryGenerate(players, PG_DYING));
	GlobalRadio(server, alv, players);
	GlobalRadio(server, dyi, players);
}

//The main procedure of the game
int MainLogic()
{
	//Game Init
	CServer server, serWolf;
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
	int statgroupW = serWolf.Init(8888, "127.0.0.1");	//Open group for werewolves
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
		if (stoi(strResp) == RM_NAME)
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
	int nNote = 0;
	while (nGameEnd == 0)	
	{
		Sync(vecPlayers, server);
		if (nRound == 0)
		{
			RandomPick(vecPlayers,server);		//round 0 to pick characters
			GlobalRadio(server, string("_S|Game started!"), vecPlayers);
			++nRound;	//Start from the first Night
		}
		//Delegate to the mainlogic to calculate the one
		vector<int> vecToKill, vecToBadge, vecToExile, vecDummy;
		vector<vector<int>> vecSet{ vecToKill, vecToBadge, vecToExile };
		//Night Phase
		bNight = true;
		//Show DayNight and round
		ShowRound(bNight, server,nRound, vecPlayers);
		string strAlive;	//Generate a param string of alive players' IDs
		strAlive = ParamGenerate(vecPlayers, PG_ALIVE);
		//WEREWOLF
		//Grouptalk
		GroupRadio(server, string("_G|"), vecPlayers, werewolf);
		bool bTalk = true;
		serWolf.Run(WOLF_NUM);
		//Link to serWolf use RM_TALK to talk to each other
		//use CM_GROUP to end this procedure
		while (bTalk)
		{
			vector<string> vecGroup;
			GroupGet(serWolf, vecGroup, vecPlayers, werewolf);
			for (auto s : vecGroup)
			{
				if (s.find(vecCommandList[CM_GROUP].c_str()) != string::npos)	//Analyze the End mark
				{
					bTalk = false;
				}
			}
			if (bTalk)
			{
				for (auto s : vecGroup)
				{
					Parse(s, vecPlayers, vecDummy, serWolf);
				}
			}
		}
		serWolf.Shut();
		//End talk
		string strKill = strAlive;
		bool bKill = false;
		vector<int> vecKill;
		while (!bKill)
		{
			vecSet[VD_KILL].clear();
			string msgWolf = string("_K|") + strKill;
			GroupRadio(server, msgWolf, vecPlayers, werewolf);
			vector<string> vecRespWolf;
			GroupGet(server, vecRespWolf, vecPlayers, werewolf);
			for (auto r : vecRespWolf)
			{
				Parse(r, vecPlayers, vecSet[VD_KILL], server);
			}
			vecKill = FindMost(vecSet[VD_KILL], vecPlayers, false);
			if (vecKill.size() == 1)
			{
				bKill = true;
			}
			else
			{
				strKill = ParamGenerate(vecKill);
			}
		}
		int nKill = vecKill[0];
		vecPlayers[nKill].m_stateSelf.bDying = true;	//Sum of the wolf killing
		//PROPHET
		string msgProp = string("_I|") + strAlive;
		GroupRadio(server, msgProp, vecPlayers, prophet);
		vector<string> vecRespProphet;
		GroupGet(server, vecRespProphet, vecPlayers, prophet);
		for (auto r : vecRespProphet)
		{
			Parse(r, vecPlayers, vecDummy, server);
		}
		//WITCH
		string msgWitch("_W|");
		Sync(vecPlayers,server);
		GroupRadio(server, msgWitch, vecPlayers, witch);
		vector<string> vecRespWitch;
		GroupGet(server, vecRespWitch, vecPlayers, witch);
		for (auto w : vecRespWitch)
		{
			Parse(w, vecPlayers, vecDummy, server);
		}
		//DAWN
		bNight = false;
		//Show DayNight and round
		ShowRound(bNight, server, nRound, vecPlayers);
		//First day police election
		if (nRound == 1)
		{
			vecSet[VD_BADGE].clear();
			string strElect("_T|Please deliver your statement in Police Election.");
			GlobalRadio(server, strElect, vecPlayers);
			vector<string> vecPE;
			GlobalGet(server, vecPE, vecPlayers);
			for (auto epe : vecPE)
			{
				Parse(epe, vecPlayers, vecDummy, server);
			}
			//After every one's statement come their decisions
			int nBadge;
			bool bElected = false;
			string strEle = strAlive;
			vector<int> vecBadge;
			while (!bElected)
			{
				vecSet[VD_BADGE].clear();
				vecBadge.clear();
				string msgBadge("_B|");
				msgBadge += strEle;
				GlobalRadio(server, msgBadge, vecPlayers);
				vector<string> vecRespBadge;
				GlobalGet(server, vecRespBadge, vecPlayers);
				for (auto b : vecRespBadge)
				{
					Parse(b, vecPlayers, vecToBadge, server);
				}
				vecBadge = FindMost(vecToBadge, vecPlayers, false);
				if (vecBadge.size() == 1)
				{
					bElected = true;
				}
				else
				{
					strEle = ParamGenerate(vecBadge);
				}
			}
			nBadge = vecBadge[0];
			vecPlayers[nBadge].m_stateSelf.bBadged = true;
			string strBadge("_S|Player ");
			strBadge += to_string(nBadge) + string(" ") + vecPlayers[nBadge].GetName() + string("is elected as Police.");
			GlobalRadio(server, strBadge, vecPlayers);
		}
		//Deal with some death
		PlayerInfo *plKill = nullptr;
		vector<PlayerInfo*> vecpDead;
		for (auto p : vecPlayers)
		{
			if (p.m_stateSelf.bDying)
			{
				string msgNote("_S|Last night Player ");
				msgNote += ParamGenerate(vecPlayers, PG_DYING) + string(" ") + p.GetName() + string(" was killed.");
				GlobalRadio(server, msgNote, vecPlayers);
				vecpDead.push_back(&p);
				p.m_stateSelf.bAlive = false;
				p.m_stateSelf.bDying = false;
				//Kill Note for round 1
				if (nRound == 1)
				{
					string msgNote("_N|");
					server.SendMsg(msgNote.c_str(), LEN(msgNote), p.GetID());
					char szNote[MAX_SIZE];
					server.RecvMsg(szNote, MAX_SIZE, p.GetID());
					Parse(string(szNote), vecPlayers, vecDummy, server);
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
			for (auto plDead : vecpDead)
			{
				if (plDead->m_stateSelf.bBadged)	//Move badge
				{
					string strXOther("_X|");
					strXOther += strAlive;
					server.SendMsg(strXOther.c_str(), LEN(strXOther), plDead->GetID());
					char sz[MAX_SIZE];
					server.RecvMsg(sz, MAX_SIZE, plDead->GetID());
					Parse(string(sz), vecPlayers, vecDummy, server);
				}
				if (plDead->m_nch == hunter)	//Hunter's phase
				{
					string msgHunter("_H|");
					msgHunter += strAlive;
					server.SendMsg(msgHunter.c_str(), LEN(msgHunter), plDead->GetID());
					char szHunter[MAX_SIZE];
					server.RecvMsg(szHunter, MAX_SIZE, plDead->GetID());
					Parse(string(szHunter), vecPlayers, vecDummy, server);
					for (auto p : vecPlayers)
					{
						if (p.m_stateSelf.bDying)
						{
							plKill = &p;
						}
					}
					if (plKill != nullptr)
					{
						string strHunter("_S|Hunter chose to shoot player ");
						strHunter += to_string(plKill->GetID()) + string(" ") + plKill->GetName();
						GlobalRadio(server, strHunter, vecPlayers);
						plKill->m_stateSelf.bAlive = false;
						plKill->m_stateSelf.bDying = false;
					}
				}
			}
			Sync(vecPlayers, server);
		}
		else	//No one died
		{
			string strPeace("_S|No one was killed,last night was peace.");
			GlobalRadio(server, strPeace, vecPlayers);
		}
		vecpDead.clear();
		plKill = nullptr;
		//Speak globally
		vector<string> vecTalk;
		string msgTalk("_T|");
		strAlive = ParamGenerate(vecPlayers, PG_ALIVE);
		msgTalk += strAlive;
		GlobalRadio(server, msgTalk, vecPlayers);
		GlobalGet(server, vecTalk, vecPlayers);
		for (auto t : vecTalk)
		{
			Parse(t, vecPlayers, vecDummy, server);
		}
		strAlive = ParamGenerate(vecPlayers, PG_ALIVE);
		//EXILE
		vector<int> vecExile;
		bool bExile = false;
		string strNameList = strAlive;
		int nExile;
		while (!bExile)
		{
			vecSet[VD_EXILE].clear();
			vecExile.clear();
			vector<string> vecRespExile;
			string msgExile("_E|");
			msgExile += strNameList;
			GlobalRadio(server, msgExile, vecPlayers);
			GlobalGet(server, vecRespExile, vecPlayers);
			for (auto t : vecRespExile)
			{
				Parse(t, vecPlayers, vecSet[VD_EXILE], server);
			}
			vecExile = FindMost(vecExile, vecPlayers, true);
			if (vecExile.size() == 1)
			{
				bExile = true;
			}
			else
			{
				strNameList = ParamGenerate(vecExile);
			}
		}
		nExile = vecExile[0];
		string strExile("_S|Player ");
		strExile += to_string(nExile) + string(" ") + vecPlayers[nExile].GetName() + string(" is exiled!");
		GlobalRadio(server, strExile, vecPlayers);
		vecPlayers[nExile].m_stateSelf.bAlive = false;
		Sync(vecPlayers, server);
		//Exile note
		if (nNote < MAX_NOTE)
		{
			string msgNoteEx("_N|");
			server.SendMsg(msgNoteEx.c_str(), LEN(msgNoteEx), vecPlayers[nExile].GetID());
			char szNoteEx[MAX_SIZE];
			server.RecvMsg(szNoteEx, MAX_SIZE, vecPlayers[nExile].GetID());
			Parse(string(szNoteEx), vecPlayers, vecDummy, server);
			++nNote;
		}
		//If hunter is exiled,he choose to shoot.
		if (vecPlayers[nExile].m_nch == hunter)
		{
			string msgHunter("_H|");
			msgHunter += strAlive;
			server.SendMsg(msgHunter.c_str(), LEN(msgHunter), vecPlayers[nExile].GetID());
			char szHunter[MAX_SIZE];
			server.RecvMsg(szHunter, MAX_SIZE, vecPlayers[nExile].GetID());
			Parse(string(szHunter), vecPlayers, vecDummy, server);
			for (auto p : vecPlayers)
			{
				if (p.m_stateSelf.bDying)
				{
					plKill = &p;
				}
			}
			if (plKill != nullptr)
			{
				string strHunter("_S|Hunter chose to shoot player ");
				strHunter += to_string(plKill->GetID()) + string(" ") + plKill->GetName();
				GlobalRadio(server, strHunter, vecPlayers);
				plKill->m_stateSelf.bAlive = false;
				plKill->m_stateSelf.bDying = false;
			}
		}
		//If an exiled hunter shoot a man in the day,this man will leave his note.
		if (plKill != nullptr)
		{
			if (nNote < MAX_NOTE)
			{
				string msgHunterNote("_N|");
				server.SendMsg(msgHunterNote.c_str(), LEN(msgHunterNote), plKill->GetID());
				char szHunterNote[MAX_SIZE];
				server.RecvMsg(szHunterNote, MAX_SIZE, plKill->GetID());
				Parse(string(szHunterNote), vecPlayers, vecDummy, server);
				++nNote;
			}
		}
		plKill = nullptr;
		//End check
		nGameEnd = EndGame(vecPlayers);
		if (nGameEnd != 0)
		{
			break;
		}
		++nRound;
	}
	//Game ends and Pronounce the result
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
