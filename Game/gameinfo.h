#ifndef _GAMEINFO_H
#define _GAMEINFO_H

#include <string>

#define LEN(s) 1+s.size()

typedef int TID;

enum enIdentity
{
	werewolf, villager, prophet, witch, hunter, cupido, guardian
};

struct state						
{
	bool bAlive;						
	bool bDying;						
	bool bBadged;						
	state() :bAlive(true), bDying(false), bBadged(false) {};
};

class PlayerInfo
{
private:
	std::string m_strPlayerName;
	TID m_ID;
public:
	int m_nch;
	state m_stateSelf;
	std::string m_strIdentity;
	PlayerInfo(std::string,TID);
	TID GetID();
	std::string GetName();
};
#endif
