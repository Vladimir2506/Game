//Name : gameinfo.cpp
//Author:  ÏÄ×¿·²
//Date : 2017-04-30
//Description :Implementation of gameinfo
//Copyright : All by author
#include "gameinfo.h"

//Constructor
PlayerInfo::PlayerInfo(TID id)
	:m_stateSelf()
{
	m_nch = -1;
	m_ID = id;
}

//Get id
TID PlayerInfo::GetID()
{
	return m_ID;
}
