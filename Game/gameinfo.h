#ifndef _GAMEINFO_H
#define _GAMEINFO_H

#include <string>

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
	TID m_ID;
public:
	int m_nch;
	state m_stateSelf;
	PlayerInfo(TID);
	TID GetID();
};
#endif
