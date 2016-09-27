#include <fstream>
#include <time.h>
#include <string>
#include <thread>
#include <windows.h>

#include "Locations.h"
#include "Miner.h"
#include "MinersWife.h"
#include "Drunkard.h"
#include "EntityManager.h"
#include "MessageDispatcher.h"
#include "misc/ConsoleUtils.h"
#include "EntityNames.h"

std::ofstream os;

int main()
{
  // Convenient console resizing (800x1024)
  HWND consoleWnd = GetConsoleWindow();
  RECT r;
  GetWindowRect(consoleWnd, &r);
  MoveWindow(consoleWnd, r.left, r.top, 800, 1024, true);

//define this to send output to a text file (see locations.h)
#ifdef TEXTOUTPUT
  os.open("output.txt");
#endif

  //create entities
  Miner* Bob = new Miner(ent_Miner_Bob);
  MinersWife* Elsa = new MinersWife(ent_Elsa);
  Drunkard* John = new Drunkard(ent_Drunkard_John);

  //register them with the entity manager
  EntityMgr->RegisterEntity(Bob);
  EntityMgr->RegisterEntity(Elsa);
  EntityMgr->RegisterEntity(John);

  // create the console queue thread
  std::thread coutTd(&ConsoleQueue::printLoop, coutQueue, 200);

  // lambda function "condition(input)" : if input is "Y" or "y", returns true
  std::string input = "Y";
  std::function<bool(std::string)> condition;
  condition = [](std::string input) {
	  return (input == "Y" || input == "y");
  };

  //run Bob, Elsa and John through a few Update calls
  for (int i = 0; condition(input); i++)
  { 
	// entities threads
    std::thread minerTd(&Miner::Update, Bob);
    std::thread wifeTd(&MinersWife::Update, Elsa);
	std::thread drunkTd(&Drunkard::Update, John);

	minerTd.join();
	wifeTd.join();
	drunkTd.join();

    //dispatch any delayed messages
	coutQueue->waitIdle();
    Dispatch->DispatchDelayedMessages();
	
	// do 30 more iterations?
	if (i > 0 && i%30 == 0) {
		coutQueue->send("\nContinue story ? <y/N>\n",
						FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
		input = coutQueue->getLine();
		coutQueue->send((condition(input) ? "Starting 30 more iterations." : "The end."),
						FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
	}
  }

  // end infinite printing loop before joining thread
  coutQueue->breakLoop();
  coutTd.join();

  //tidy up
  delete Bob;
  delete Elsa;
  delete John;

  //wait for a keypress before exiting
  PressAnyKeyToContinue();


  return 0;
}
