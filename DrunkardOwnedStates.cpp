#include "DrunkardOwnedStates.h"
#include "Drunkard.h"
#include "Locations.h"
#include "Time/CrudeTimer.h"
#include "MessageDispatcher.h"
#include "MessageTypes.h"
#include "EntityNames.h"

#include <iostream>
using std::cout;
#include <string>

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
	switch(msg.Msg) {
		case Msg_RivalSaysHi:
			drunkard->ConsoleLog("\nMessage handled by " + GetNameOfEntity(drunkard->ID()) + " at time: " + std::to_string(Clock->GetCurrentTime()) );

			drunkard->SetRivalSpotted(true);
			return true;
		break;
		case Msg_RivalSaysBye:
			drunkard->ConsoleLog("\nMessage handled by " + GetNameOfEntity(drunkard->ID()) + " at time: " + std::to_string(Clock->GetCurrentTime()) );

			drunkard->SetRivalSpotted(false);
			return true;
		break;
		case Msg_AcceptFight:
			drunkard->ConsoleLog("\nMessage handled by " + GetNameOfEntity(drunkard->ID()) + " at time: " + std::to_string(Clock->GetCurrentTime()) );

			drunkard->GetFSM()->ChangeState(GetInFight::Instance());
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
	drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Time to fill the tank!");
}

void GetDrunk::Execute(Drunkard* drunkard) {
	drunkard->IncreaseDrunkenness();

	switch(RandInt(0,2)) {
		case 0:
			drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Orderin' a beer");
		break;
		case 1:
			drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Drinkin' a beer");
		break;
		case 2:
			drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Buuurp!");
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
	drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": I bet I can handle any o' y'all!");
	drunkard->IncreaseFatigue();
}

void SeekFight::Execute(Drunkard* drunkard) {
	if (drunkard->Fatigued()) {
		drunkard->GetFSM()->ChangeState(SleepTilRested::Instance());

	} else if (drunkard->getDrunkenness() == 0) {
		drunkard->GetFSM()->ChangeState(GetDrunk::Instance());

	} else {
		drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Seekin' the fight");

		if (drunkard->RivalSpotted()) {
			switch(RandInt(0,2)) {
				case 0:
					drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Get out, ya'll never be the man your mother is!");
				break;
				case 1:
					drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": If ya were twice as smart, ya'd still be stupid!");
				break;
				case 2:
					drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": D'ya have to leave so soon? Ah was just 'bout to poison the beer.");
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
		
		drunkard->DecreaseDrunkenness();
	}
}

void SeekFight::Exit(Drunkard* drunkard) {
}

bool SeekFight::OnMessage(Drunkard* drunkard, const Telegram& msg) {
	return false;
}

//------------------------------------------------------------------------GetInFight

GetInFight* GetInFight::Instance() {
	static GetInFight instance;
	return &instance;
}

void GetInFight::Enter(Drunkard* drunkard) {
	drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": O'right, let's settle this, you chuckle head!");
}

void GetInFight::Execute(Drunkard* drunkard) {
	drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": POW!");

	drunkard->IncreaseFatigue();
	if (drunkard->Fatigued()) {
		drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Dammit, I'm done! You win this time!");
		drunkard->GetFSM()->ChangeState(SleepTilRested::Instance());

		Dispatch->DispatchMessage(
			SEND_MSG_IMMEDIATELY,
			drunkard->ID(),
			ent_Miner_Bob,
			Msg_YouWinThisTime,
			NO_ADDITIONAL_INFO
		);
	}
}

void GetInFight::Exit(Drunkard* drunkard) {
	drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Fight's over!");
}


bool GetInFight::OnMessage(Drunkard* drunkard, const Telegram& msg) {
	switch(msg.Msg) {
		case Msg_YouWinThisTime: {
			drunkard->ConsoleLog("\nMessage received by " + GetNameOfEntity(drunkard->ID()) + " at time: " + std::to_string(Clock->GetCurrentTime()) );

			drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Ha! Got enough already?!");

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
	drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Uh... So much tired");
	m_iSleepDuration = drunkard->SleepDuration();
}

void SleepTilRested::Execute(Drunkard* drunkard) {
	if (drunkard->getFatigue() == m_iSleepDuration) {
		drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Throwing up - Uh... Got a hangover... Need to heal myself with moar beer!");
		drunkard->GetFSM()->ChangeState(GetDrunk::Instance());
	} else {
		drunkard->Say("\n" + GetNameOfEntity(drunkard->ID()) + ": Burp... ZZZZ... ");
		drunkard->DecreaseFatigue();
	} 
}

void SleepTilRested::Exit(Drunkard* drunkard) {
	drunkard->ResetDrunkenness();
}

bool SleepTilRested::OnMessage(Drunkard* drunkard, const Telegram& msg) {
	return false;
}