//Name : gameinfo.h
//Author:  ÏÄ×¿·²
//Date : 2017-04-30
//Description : Header of gameinfo
//Copyright : All by author
#ifndef _GAMEINFO_H
#define _GAMEINFO_H

#include <string>

typedef int TID;

//To notify the identity
enum enIdentity
{
	werewolf, villager, prophet, witch, hunter, cupido, guardian
};

//To notify the status
struct state						
{
	bool bAlive;						
	bool bDying;						
	bool bBadged;						
	state() :bAlive(true), bDying(false), bBadged(false) {};
};

//PlayerInfo -- description of a player
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
