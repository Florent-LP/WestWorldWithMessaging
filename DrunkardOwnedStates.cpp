#include "DrunkardOwnedStates.h"
#include "Drunkard.h"
#include "Locations.h"
#include "Time/CrudeTimer.h"
#include "MessageDispatcher.h"
#include "MessageTypes.h"
#include "EntityNames.h"

#include <iostream>
using std::cout;
#include "pthread.h"

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
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

	switch(msg.Msg) {
		case Msg_RivalSaysHi:
			cout << "\nMessage handled by " << GetNameOfEntity(drunkard->ID()) << " at time: " << Clock->GetCurrentTime();
			pthread_mutex_unlock(&consoleMutex);

			drunkard->SetRivalSpotted(true);
			return true;
		break;
		case Msg_RivalSaysBye:
			cout << "\nMessage handled by " << GetNameOfEntity(drunkard->ID()) << " at time: " << Clock->GetCurrentTime();
			pthread_mutex_unlock(&consoleMutex);

			drunkard->SetRivalSpotted(false);
			return true;
		break;
		case Msg_AcceptFight:
			cout << "\nMessage handled by " << GetNameOfEntity(drunkard->ID()) << " at time: " << Clock->GetCurrentTime();
			pthread_mutex_unlock(&consoleMutex);

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
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Time to fill the tank!";
	pthread_mutex_unlock(&consoleMutex);
}

void GetDrunk::Execute(Drunkard* drunkard) {
	drunkard->IncreaseDrunkenness();
	
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
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
	pthread_mutex_unlock(&consoleMutex);

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
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": I bet I can handle any o' y'all!"; 
	pthread_mutex_unlock(&consoleMutex);
	drunkard->IncreaseFatigue();
}

void SeekFight::Execute(Drunkard* drunkard) {
	if (drunkard->Fatigued()) {
		drunkard->GetFSM()->ChangeState(SleepTilRested::Instance());

	} else if (drunkard->getDrunkenness() == 0) {
		drunkard->GetFSM()->ChangeState(GetDrunk::Instance());

	} else {
		pthread_mutex_lock(&consoleMutex);
		SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Seekin' the fight";

		if (drunkard->RivalSpotted()) {
			switch(RandInt(0,2)) {
				case 0:
					cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Get out, ya'll never be the man your mother is!";
				break;
				case 1:
					cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": If ya were twice as smart, ya'd still be stupid!";
				break;
				case 2:
					cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": D'ya have to leave so soon? Ah was just 'bout to poison the beer.";
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
		pthread_mutex_unlock(&consoleMutex);
		
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
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": O'right, let's settle this, you chuckle head!";
	pthread_mutex_unlock(&consoleMutex);
}

void GetInFight::Execute(Drunkard* drunkard) {
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": POW!" << " (" << drunkard->getFatigue() << ")";
	pthread_mutex_unlock(&consoleMutex);

	drunkard->IncreaseFatigue();
	if (drunkard->Fatigued()) {
		pthread_mutex_lock(&consoleMutex);
		cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Dammit, I'm done! You win this time!" << " (" << drunkard->getFatigue() << ")";
		pthread_mutex_unlock(&consoleMutex);
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
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Fight's over!";
	pthread_mutex_unlock(&consoleMutex);
}


bool GetInFight::OnMessage(Drunkard* drunkard, const Telegram& msg) {
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

	switch(msg.Msg) {
		case Msg_YouWinThisTime: {
			cout << "\nMessage received by " << GetNameOfEntity(drunkard->ID()) << " at time: " << Clock->GetCurrentTime();

			SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
			cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Ha! Got enough already?!";
			pthread_mutex_unlock(&consoleMutex);

			drunkard->SetRivalSpotted(false);
			drunkard->GetFSM()->ChangeState(SeekFight::Instance());               
		}
		return true;
	}
	pthread_mutex_unlock(&consoleMutex);
	return false;
}

//-------------------------------------------------------------------------SleepTilRested

SleepTilRested* SleepTilRested::Instance() {
	static SleepTilRested instance;
	return &instance;
}

void SleepTilRested::Enter(Drunkard* drunkard) {
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Uh... So much tired";
	pthread_mutex_unlock(&consoleMutex);
	m_iSleepDuration = drunkard->SleepDuration();
}

void SleepTilRested::Execute(Drunkard* drunkard) {
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_BLUE|FOREGROUND_INTENSITY);

	if (drunkard->getFatigue() == m_iSleepDuration) {
		cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": Throwing up - Uh... Got a hangover... Need to heal myself with moar beer!";
		pthread_mutex_unlock(&consoleMutex);
		drunkard->GetFSM()->ChangeState(GetDrunk::Instance());
	} else {
		cout << "\n" << GetNameOfEntity(drunkard->ID()) << ": " << "Burp... ZZZZ... ";
		pthread_mutex_unlock(&consoleMutex);
		drunkard->DecreaseFatigue();
	} 
}

void SleepTilRested::Exit(Drunkard* drunkard) {
	drunkard->ResetDrunkenness();
}

bool SleepTilRested::OnMessage(Drunkard* drunkard, const Telegram& msg) {
	return false;
}