#include "character.h"

bool cWerewolf::IsOperateValid(TID target, bool IsAlive)
{
	if (target < PlayerTotality
		&&	 IsAlive)			//the target is still alive
		return true;
	else return false;
}

void cWerewolf::Kill(TID target, vector<state> PlayerState)
{
	if (cWerewolf::IsOperateValid(target, PlayerState[target].IsAlive))
		PlayerState[target].IsDying = true;
}

bool cProphet::IsOperateValid(TID target, bool IsAlive, vector<TPlayerChar> Playerchar, TID ID)
{
	if (target < PlayerTotality
		&& IsAlive				//the target is still alive
		&& target != ID)		//not checking self			
		return true;
	else return false;
}

void cProphet::Check(TID target, vector<state> PlayerState, vector<TPlayerChar> Playerchar)
{
	if (IsOperateValid(target, PlayerState[target].IsAlive, Playerchar))
		PlayerCharOfProphet[target] = Playerchar[target];
}

bool cWitch::IsOperateValid(int action, TID target, vector<state> PlayerState)
{
	if (target < PlayerTotality)
		if( action == UseAntidote
			&& !IsAntidoteUsed
			&&PlayerState[target].IsDying		//the target is dying
			||action ==UsePoison
			&& !IsPoisonUsed
			&&PlayerState[target].IsAlive)		//the target is still alive			
		return true;
	return false;
}

void cWitch::Medication(int action, TID target, vector<state> PlayerState)
{
	if (IsOperateValid(action, target, PlayerState))
	{
		if (action == UseAntidote)
		{
			PlayerState[target].IsDying = false;
			IsAntidoteUsed = true;
		}

		else
		{
			if (action == UsePoison)
			{
				PlayerState[target].IsAlive = false;
				IsPoisonUsed = true;
			}
		}
	}
}

//here is a problem: as a subclass, it is unaccessible to get the ID of player, thus not sure of the "isDying"
bool cHunter::DoShoot(bool isDying, bool doshoot)		
{
	if (isDying)							//hunter is dying
		if (doshoot)						//hunter decides to shoot
			return true;
	return false;
}

bool cHunter::IsOperateValid(TID target,bool IsAlive, bool isDying, bool doshoot)
{
	if (DoShoot(isDying,doshoot))						//hunter decides to shoot
		if (IsAlive)							//the target is still alive
			return true;
	return false;
}

void cHunter::Shoot(TID target, vector<state> PlayerState, bool doshoot)
{
	if (IsOperateValid(target, PlayerState[target].IsAlive, PlayerState[ID].IsDying, doshoot))
		PlayerState[target].IsAlive = false;
}

bool cCupido::IsOperateValid(TID target1, TID target2, vector<state> PlayerState)
{
	if (!IsCombineUsed)
		if (target1 < PlayerTotality
			&&target2 < PlayerTotality
			&&target1 != target2
			&&PlayerState[target1].IsAlive
			&&PlayerState[target2].IsAlive)
			return true;
	return false;
}

void cCupido::Combine(TID target1, TID target2, vector<state> PlayerState)
{
	if (IsOperateValid(target1, target2, PlayerState))
	{
		PlayerState[target1].IsCombined = true;
		PlayerState[target2].IsCombined = true;
		IsCombineUsed = true;
	}
}

bool cGuardian::IsOperateValid(TID target, bool IsAlive, TID ID)
{
	if (LastGuarded < PlayerTotality)
	{
		if (target < PlayerTotality
			&& target != ID
			&& IsAlive)			//the target is still alive
			return true;
	}
	else
		if (target < PlayerTotality
			&&	 IsAlive)			//the target is still alive
			return true;
	else return false;
}

void cGuardian::Guard(TID target, vector<state> PlayerState)
{
	if (IsOperateValid(target, PlayerState[target].IsAlive, ID))
	{
		PlayerState[target].IsGuarded = true;
		LastGuarded = target;
	}
}
