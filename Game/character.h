#include <vector>
#include <iostream>

#ifndef CHARACTER_H
#define CHARACTER_H

#define PlayerTotality 8				//the total amount of players
#define CharacterTotality 7				//the total amount of character
#define UnknownPlayer -1				//the players whose characters are unknown

typedef unsigned int TID;				//to describe player's ID number (start from 0)
typedef unsigned int TPlayerChar;		//to describe player's character(look up "enum character" for specific info)

using namespace::std;

enum character 
{
	werewolf, villager, prophet, witch, hunter, cupido, guardian
};


struct state							//to describe the state of player
{
	bool IsAlive;						//true stands for live, false stands for death
	bool IsDying;						//true stands for being going to die or being killed by werewolf
	bool IsCombined;					//true stands for being combined with another player by Cupido
	bool IsGuarded;						//true stands for being protected from killing of werewolf by guardian
	bool IsBadged;						//true stands for being the police
	state() :IsAlive(true), IsDying(false), IsCombined(false), IsGuarded(false), IsBadged(false) {};
};

//character base class
class cBaseCharater 
{
protected:
	/*
	char GamerName;
	bool health;						//true stands for live, false stands for death
	*/
};

//cWerewolf
class cWerewolf : public cBaseCharater
{
private:
	bool IsOperateValid(TID target, bool IsAlive);

public:
	void Kill(TID target, vector<state> PlayerState);					//kill the appointed target
};

//villager
class cVillager : public cBaseCharater{
protected:
	bool IsOperateValid() {};
};

//prophet
class cProphet : public cBaseCharater
{
private:
	vector<int> PlayerCharOfProphet;										//the characters of players which have been checked 
	bool IsOperateValid(TID target, bool IsAlive, vector<TPlayerChar> Playerchar, TID ID);

public:
	cProphet() :PlayerCharOfProphet(PlayerTotality, UnknownPlayer) {};							
	void Check(TID target, vector<state> PlayerState, vector<TPlayerChar> Playerchar);				//check the character of target
};

class cWitch :public cBaseCharater
{
private:
	bool IsAntidoteUsed;
	bool IsPoisonUsed;
	bool IsOperateValid(int action, TID target, vector<state> PlayerState);
public:
	enum eAction 
	{
		UseAntidote,
		UsePoison,
		DoNothing
	};
	cWitch() :IsAntidoteUsed(false), IsPoisonUsed(false) {};
	void Medication(int action, TID target, vector<state> PlayerState);
};

class cHunter : public cBaseCharater
{
private:
	bool IsOperateValid(TID target,bool IsAlive, bool isDying, bool doshoot);

public:
	bool DoShoot(bool isDying, bool doshoot);				//if hunter want to shoot when he is dying
	void Shoot(TID target, vector<state> PlayerState, bool doshoot);				//shoot the character of target
};

class cCupido :public cBaseCharater
{
private:
	bool IsCombineUsed;
	bool IsOperateValid(TID target1, TID target2, vector<state> PlayerState);

public:
	cCupido() :IsCombineUsed(false) {};
	void Combine(TID target1, TID target2, vector<state> PlayerState);		//combine two players
};

class cGuardian :public cBaseCharater
{
private:
	TID LastGuarded;			//the player protected at last round
	bool IsOperateValid(TID target, bool IsAlive, TID ID);

public:
	void Guard(TID target, vector<state> PlayerState);				//protect an appointed player
	cGuardian() :LastGuarded(PlayerTotality) {};
};


#endif
