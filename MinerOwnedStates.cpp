#include "MinerOwnedStates.h"
#include "fsm/State.h"
#include "Miner.h"
#include "Locations.h"
#include "messaging/Telegram.h"
#include "MessageDispatcher.h"
#include "MessageTypes.h"
#include "Time/CrudeTimer.h"
#include "EntityNames.h"

#include <iostream>
using std::cout;
#include "pthread.h"


#ifdef TEXTOUTPUT
#include <fstream>
extern std::ofstream os;
#define cout os
#endif


//------------------------------------------------------------------------methods for EnterMineAndDigForNugget
EnterMineAndDigForNugget* EnterMineAndDigForNugget::Instance()
{
  static EnterMineAndDigForNugget instance;

  return &instance;
}


void EnterMineAndDigForNugget::Enter(Miner* pMiner)
{
  //if the miner is not already located at the goldmine, he must
  //change location to the gold mine
  if (pMiner->Location() != goldmine)
  {
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Walkin' to the goldmine";
	pthread_mutex_unlock(&consoleMutex);

    pMiner->ChangeLocation(goldmine);
  }
}


void EnterMineAndDigForNugget::Execute(Miner* pMiner)
{  
  //Now the miner is at the goldmine he digs for gold until he
  //is carrying in excess of MaxNuggets. If he gets thirsty during
  //his digging he packs up work for a while and changes state to
  //gp to the saloon for a whiskey.
  pMiner->AddToGoldCarried(1);

  pMiner->IncreaseFatigue();

  pthread_mutex_lock(&consoleMutex);
  SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Pickin' up a nugget";
  pthread_mutex_unlock(&consoleMutex);

  //if enough gold mined, go and put it in the bank
  if (pMiner->PocketsFull())
  {
    pMiner->GetFSM()->ChangeState(VisitBankAndDepositGold::Instance());
  }

  if (pMiner->Thirsty())
  {
    pMiner->GetFSM()->ChangeState(QuenchThirst::Instance());
  }
}


void EnterMineAndDigForNugget::Exit(Miner* pMiner)
{
  pthread_mutex_lock(&consoleMutex);
  SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
       << "Ah'm leavin' the goldmine with mah pockets full o' sweet gold";
  pthread_mutex_unlock(&consoleMutex);
}


bool EnterMineAndDigForNugget::OnMessage(Miner* pMiner, const Telegram& msg)
{
  //send msg to global message handler
  return false;
}

//------------------------------------------------------------------------methods for VisitBankAndDepositGold

VisitBankAndDepositGold* VisitBankAndDepositGold::Instance()
{
  static VisitBankAndDepositGold instance;

  return &instance;
}

void VisitBankAndDepositGold::Enter(Miner* pMiner)
{  
  //on entry the miner makes sure he is located at the bank
  if (pMiner->Location() != bank)
  {
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Goin' to the bank. Yes siree";
	pthread_mutex_unlock(&consoleMutex);

    pMiner->ChangeLocation(bank);
  }
}


void VisitBankAndDepositGold::Execute(Miner* pMiner)
{
  pthread_mutex_lock(&consoleMutex);
  SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);

  //deposit the gold
  pMiner->AddToWealth(pMiner->GoldCarried());
    
  pMiner->SetGoldCarried(0);

  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
       << "Depositing gold. Total savings now: "<< pMiner->Wealth();
  pthread_mutex_unlock(&consoleMutex);

  //wealthy enough to have a well earned rest?
  if (pMiner->Wealth() >= ComfortLevel)
  {
	pthread_mutex_lock(&consoleMutex);
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
         << "WooHoo! Rich enough for now. Back home to mah li'lle lady";
	pthread_mutex_unlock(&consoleMutex);
      
    pMiner->GetFSM()->ChangeState(GoHomeAndSleepTilRested::Instance());      
  }

  //otherwise get more gold
  else 
  {
    pMiner->GetFSM()->ChangeState(EnterMineAndDigForNugget::Instance());
  }
}


void VisitBankAndDepositGold::Exit(Miner* pMiner)
{
  pthread_mutex_lock(&consoleMutex);
  SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Leavin' the bank";
  pthread_mutex_unlock(&consoleMutex);
}


bool VisitBankAndDepositGold::OnMessage(Miner* pMiner, const Telegram& msg)
{
  //send msg to global message handler
  return false;
}
//------------------------------------------------------------------------methods for GoHomeAndSleepTilRested

GoHomeAndSleepTilRested* GoHomeAndSleepTilRested::Instance()
{
  static GoHomeAndSleepTilRested instance;

  return &instance;
}

void GoHomeAndSleepTilRested::Enter(Miner* pMiner)
{
  if (pMiner->Location() != shack)
  {
	pthread_mutex_lock(&consoleMutex);
    SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Walkin' home";
	pthread_mutex_unlock(&consoleMutex);

    pMiner->ChangeLocation(shack); 

    //let the wife know I'm home
    Dispatch->DispatchMessage(SEND_MSG_IMMEDIATELY, //time delay
                              pMiner->ID(),        //ID of sender
                              ent_Elsa,            //ID of recipient
                              Msg_HiHoneyImHome,   //the message
                              NO_ADDITIONAL_INFO);    
  }
}

void GoHomeAndSleepTilRested::Execute(Miner* pMiner)
{ 
  pthread_mutex_lock(&consoleMutex);
  SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);

  //if miner is not fatigued start to dig for nuggets again.
  if (pMiner->getFatigue() == 0)
  {
     cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " 
          << "All mah fatigue has drained away. Time to find more gold!";
     pthread_mutex_unlock(&consoleMutex);

     pMiner->GetFSM()->ChangeState(EnterMineAndDigForNugget::Instance());
  }

  else 
  {
    //sleep
    pMiner->DecreaseFatigue();

    cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "ZZZZ... ";
    pthread_mutex_unlock(&consoleMutex);
  } 
}

void GoHomeAndSleepTilRested::Exit(Miner* pMiner)
{ 
}


bool GoHomeAndSleepTilRested::OnMessage(Miner* pMiner, const Telegram& msg)
{
   pthread_mutex_lock(&consoleMutex);
   SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

   switch(msg.Msg)
   {
   case Msg_StewReady:

     cout << "\nMessage handled by " << GetNameOfEntity(pMiner->ID()) 
     << " at time: " << Clock->GetCurrentTime();

     SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);

     cout << "\n" << GetNameOfEntity(pMiner->ID()) 
          << ": Okay Hun, ahm a comin'!";
     pthread_mutex_unlock(&consoleMutex);

     pMiner->GetFSM()->ChangeState(EatStew::Instance());
      
     return true;

   }//end switch
   pthread_mutex_unlock(&consoleMutex);

   return false; //send message to global message handler
}

//------------------------------------------------------------------------QuenchThirst

QuenchThirst* QuenchThirst::Instance()
{
  static QuenchThirst instance;

  return &instance;
}

void QuenchThirst::Enter(Miner* pMiner)
{
  if (pMiner->Location() != saloon)
  {    
    pMiner->ChangeLocation(saloon);

	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Boy, ah sure is thusty! Walking to the saloon" << " (" << pMiner->getFatigue() << ")";
	pthread_mutex_unlock(&consoleMutex);
	
	Dispatch->DispatchMessage(
		SEND_MSG_IMMEDIATELY,
		pMiner->ID(),
		ent_Drunkard_John,
		Msg_RivalSaysHi,
		NO_ADDITIONAL_INFO
	);
  }
}

void QuenchThirst::Execute(Miner* pMiner)
{
  pMiner->BuyAndDrinkAWhiskey();

  pthread_mutex_lock(&consoleMutex);
  SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "That's mighty fine sippin' liquer";
  pthread_mutex_unlock(&consoleMutex);

  pMiner->GetFSM()->ChangeState(EnterMineAndDigForNugget::Instance());  
}


void QuenchThirst::Exit(Miner* pMiner)
{ 
	if (!pMiner->Fighting()) {
        pthread_mutex_lock(&consoleMutex);
		SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
		cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Leaving the saloon, feelin' good";
        pthread_mutex_unlock(&consoleMutex);

		Dispatch->DispatchMessage(
			SEND_MSG_IMMEDIATELY,
			pMiner->ID(),
			ent_Drunkard_John,
			Msg_RivalSaysBye,
			NO_ADDITIONAL_INFO
		);
	}
}


bool QuenchThirst::OnMessage(Miner* pMiner, const Telegram& msg)
{
	switch(msg.Msg) {
		case Msg_Provoke: {
            pthread_mutex_lock(&consoleMutex);
			SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
			cout << "\nMessage received by " << GetNameOfEntity(pMiner->ID()) << " at time: " << Clock->GetCurrentTime();

			SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
			cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": O' right, that's enough! Ya picked on the wrong guy!";
            pthread_mutex_unlock(&consoleMutex);

			Dispatch->DispatchMessage(
				SEND_MSG_IMMEDIATELY,
				pMiner->ID(),
				ent_Drunkard_John,
				Msg_AcceptFight,
				NO_ADDITIONAL_INFO
			);
			
			pMiner->setFighting(true);
			pMiner->GetFSM()->ChangeState(DefendHonor::Instance());               
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------EatStew

EatStew* EatStew::Instance()
{
  static EatStew instance;

  return &instance;
}


void EatStew::Enter(Miner* pMiner)
{
  pthread_mutex_lock(&consoleMutex);
  SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Smells Reaaal goood Elsa!";
  pthread_mutex_unlock(&consoleMutex);
}

void EatStew::Execute(Miner* pMiner)
{
  pthread_mutex_lock(&consoleMutex);
  SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Tastes real good too!";
  pthread_mutex_unlock(&consoleMutex);

  pMiner->GetFSM()->RevertToPreviousState();
}

void EatStew::Exit(Miner* pMiner)
{ 
  pthread_mutex_lock(&consoleMutex);
  SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
  cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Thankya li'lle lady. Ah better get back to whatever ah wuz doin'";
  pthread_mutex_unlock(&consoleMutex);
}


bool EatStew::OnMessage(Miner* pMiner, const Telegram& msg)
{
  //send msg to global message handler
  return false;
}

//------------------------------------------------------------------------DefendHonor

DefendHonor* DefendHonor::Instance() {
	static DefendHonor instance;
	return &instance;
}

void DefendHonor::Enter(Miner* pMiner) {
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Gotta give ya a lesson o' mine!";
	pthread_mutex_unlock(&consoleMutex);
}

void DefendHonor::Execute(Miner* pMiner) {
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": POW!" << " (" << pMiner->getFatigue() << ")";
	pthread_mutex_unlock(&consoleMutex);

	pMiner->IncreaseFatigue();
	if (pMiner->Fatigued()) {
		pthread_mutex_lock(&consoleMutex);
		SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
		cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": Dammit, I'm done! You win this time!";
		pthread_mutex_unlock(&consoleMutex);

		Dispatch->DispatchMessage(
			SEND_MSG_IMMEDIATELY,
			pMiner->ID(),
			ent_Drunkard_John,
			Msg_YouWinThisTime,
			NO_ADDITIONAL_INFO
		);

		pMiner->GetFSM()->ChangeState(GoHomeAndSleepTilRested::Instance()); 
	}
}

void DefendHonor::Exit(Miner* pMiner) { 
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
	cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": " << "Fight's over!";
	pthread_mutex_unlock(&consoleMutex);

	pMiner->setFighting(false);
}

bool DefendHonor::OnMessage(Miner* pMiner, const Telegram& msg) {
	pthread_mutex_lock(&consoleMutex);
	SetTextColor(BACKGROUND_RED|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);

	switch(msg.Msg) {
		case Msg_YouWinThisTime: {
			cout << "\nMessage received by " << GetNameOfEntity(pMiner->ID()) << " at time: " << Clock->GetCurrentTime();

			SetTextColor(FOREGROUND_RED|FOREGROUND_INTENSITY);
			cout << "\n" << GetNameOfEntity(pMiner->ID()) << ": Ya had it comin'!";
			pthread_mutex_unlock(&consoleMutex);

			pMiner->GetFSM()->ChangeState(QuenchThirst::Instance());               
		}
		return true;
	}
	pthread_mutex_unlock(&consoleMutex);
	return false;
}


