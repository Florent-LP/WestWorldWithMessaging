#include "DrunkardOwnedStates.h"
#include "Drunkard.h"
#include "Locations.h"
#include "Time/CrudeTimer.h"
#include "MessageDispatcher.h"
#include "MessageTypes.h"
#include "EntityNames.h"

#include <iostream>
using std::cout;

#ifdef TEXTOUTPUT
#include <fstream>
extern std::ofstream os;
#define cout os
#endif

//-----------------------------------------------------------------------Global state

DrunkardsGlobalState* DrunkardsGlobalState::Instance() {
  static DrunkardsGlobalState instance;
  return &instance;
}

void DrunkardsGlobalState::Execute(Drunkard* drunkard) {
}

bool DrunkardsGlobalState::OnMessage(Drunkard* drunkard, const Telegram& msg) {
	SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

	switch(msg.Msg) {
		case Msg_RivalSaysHi:
			cout << "\nMessage handled by " << GetNameOfEntity(drunkard->ID()) << " at time: " << Clock->GetCurrentTime();

			drunkard->SetRivalSpotted(true);
			return true;
		break;
		case Msg_RivalSaysBye:
			cout << "\nMessage handled by " << GetNameOfEntity(drunkard->ID()) << " at time: " << Clock->GetCurrentTime();

			drunkard->SetRivalSpotted(false);
			return true;
		break;
	}
	return false;
}

//-------------------------------------------------------------------------GetDrunk

GetDrunk* GetDrunk::Instance() {
  static GetDrunk instance;
  return &instance;
}

void GetDrunk::Enter(Drunkard* drunkard) {
  cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Time to fill the tank!";
}

void GetDrunk::Execute(Drunkard* drunkard) {
	drunkard->IncreaseDrunkenness();

	switch(RandInt(0,2)) {
		case 0:
			cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Orderin' a beer";
		break;
		case 1:
			cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Drinkin' a beer";
		break;
		case 2:
			cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Buuurp!";
		break;
	}

	// 1 in 2 chance of seeking the fight (when drunk).
	if ((RandFloat() < 0.5) && drunkard->Drunk()) {
		drunkard->GetFSM()->ChangeState(SeekFight::Instance());
	}
}

void GetDrunk::Exit(Drunkard* drunkard) {
}

bool GetDrunk::OnMessage(Drunkard* drunkard, const Telegram& msg) {
	return false;
}

//------------------------------------------------------------------------SeekFight

SeekFight* SeekFight::Instance() {
  static SeekFight instance;
  return &instance;
}

void SeekFight::Enter(Drunkard* drunkard) {  
	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": I bet I can handle any o' y'all!"; 
}

void SeekFight::Execute(Drunkard* drunkard) {
	// 1 in 10 chance of increasing fatigue (when seeking the fight).
	if ((RandFloat() < 0.1)) {
		drunkard->IncreaseFatigue();
	}

	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": C'mon, cowards!";

	if (drunkard->RivalSpotted()) {
		switch(RandInt(0,2)) {
			case 0:
				cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Get out, you'll never be the man your mother is!";
			break;
			case 1:
				cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": If you were twice as smart, you'd still be stupid!";
			break;
			case 2:
				cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Do you have to leave so soon? I was just about to poison the beer.";
			break;
		}

		Dispatch->DispatchMessage(
			SEND_MSG_IMMEDIATELY,
			drunkard->ID(),
			ent_Miner_Bob,
			Msg_Provoke,
			NO_ADDITIONAL_INFO
		);
	}
}

void SeekFight::Exit(Drunkard* drunkard) {
}

bool SeekFight::OnMessage(Drunkard* drunkard, const Telegram& msg) {
	switch(msg.Msg) {
		case Msg_AcceptFight: {
			SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
			cout << "\nMessage handled by " << GetNameOfEntity(drunkard->ID()) << " at time: " << Clock->GetCurrentTime();

			drunkard->GetFSM()->ChangeState(GetInFight::Instance());
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------GetInFight

GetInFight* GetInFight::Instance() {
	static GetInFight instance;
	return &instance;
}

void GetInFight::Enter(Drunkard* drunkard) {
	SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": O'right, let's settle this, you chuckle head!";
}

void GetInFight::Execute(Drunkard* drunkard) {
	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": *POW*";
	drunkard->IncreaseFatigue();
	if (drunkard->Fatigued()) {
		Dispatch->DispatchMessage(
			SEND_MSG_IMMEDIATELY,
			drunkard->ID(),
			ent_Miner_Bob,
			Msg_YouWinThisTime,
			NO_ADDITIONAL_INFO
		);
		cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Dammit, I'm done! You win this time!";
		drunkard->GetFSM()->ChangeState(SleepTilRested::Instance()); 
	}
}

void GetInFight::Exit(Drunkard* drunkard) {
	SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Fight's over!";
}


bool GetInFight::OnMessage(Drunkard* drunkard, const Telegram& msg) {
	SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

	switch(msg.Msg) {
		case Msg_YouWinThisTime: {
			cout << "\nMessage received by " << GetNameOfEntity(drunkard->ID()) << " at time: " << Clock->GetCurrentTime();

			SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
			cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Ha! Got enough already?!";

			drunkard->SetRivalSpotted(false);
			drunkard->GetFSM()->ChangeState(SeekFight::Instance());               
		}
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------SleepTilRested

SleepTilRested* SleepTilRested::Instance() {
  static SleepTilRested instance;
  return &instance;
}

void SleepTilRested::Enter(Drunkard* drunkard) {
  cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Uh... So much tired";
}

void SleepTilRested::Execute(Drunkard* drunkard) {
	if (!drunkard->Fatigued()) {
		cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": *THROWS UP* Uh... Got a hangover... Need to heal myself with moar beer!";
		drunkard->GetFSM()->ChangeState(GetDrunk::Instance());
	} else {
		cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": " << "Burp... ZZZZ... ";
		drunkard->DecreaseFatigue();
	} 
}

void SleepTilRested::Exit(Drunkard* drunkard) {
	drunkard->ResetDrunkenness();
}

bool SleepTilRested::OnMessage(Drunkard* drunkard, const Telegram& msg) {
	return false;
}