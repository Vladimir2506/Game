#include "gameinfo.h"

PlayerInfo::PlayerInfo(std::string strName, TID id)
	:m_strIdentity(), m_stateSelf()
{
	m_nch = -1;
	m_strPlayerName = strName;
	m_ID = id;
}

TID PlayerInfo::GetID()
{
	return m_ID;
}

std::string PlayerInfo::GetName()
{
	return m_strPlayerName;
}
