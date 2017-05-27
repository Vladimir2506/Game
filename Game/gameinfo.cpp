#include "gameinfo.h"

PlayerInfo::PlayerInfo(TID id)
	:m_stateSelf()
{
	m_nch = -1;
	m_ID = id;
}

TID PlayerInfo::GetID()
{
	return m_ID;
}
