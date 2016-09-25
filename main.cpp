#include <fstream>
#include <time.h>
#include <string>
#include <thread>

#include "Locations.h"
#include "Miner.h"
#include "MinersWife.h"
#include "Drunkard.h"
#include "EntityManager.h"
#include "MessageDispatcher.h"
#include "misc/ConsoleUtils.h"
#include "EntityNames.h"


std::ofstream os;

// Convenient resizing (Microsoft Windows only)
#include <windows.h>
void resizeConsole(void) {
	HWND consoleWnd = GetConsoleWindow();
	RECT r;
	GetWindowRect(consoleWnd, &r);
	MoveWindow(consoleWnd, r.left, r.top, 800, 1024, true);
}

int main()
{
	resizeConsole();

//define this to send output to a text file (see locations.h)
#ifdef TEXTOUTPUT
  os.open("output.txt");
#endif

  //seed random number generator
  srand((unsigned) time(NULL));

  //create a miner
  Miner* Bob = new Miner(ent_Miner_Bob);

  //create his wife
  MinersWife* Elsa = new MinersWife(ent_Elsa);

  //create a drunkard
  Drunkard* John = new Drunkard(ent_Drunkard_John);

  //register them with the entity manager
  EntityMgr->RegisterEntity(Bob);
  EntityMgr->RegisterEntity(Elsa);
  EntityMgr->RegisterEntity(John);

  //run Bob, Elsa and John through a few Update calls
  std::string input = "Y";
  for (int i = 0; input == "Y" || input == "y"; i++)
  { 
    std::thread minerTd(&Miner::Update, Bob);
    std::thread wifeTd(&MinersWife::Update, Elsa);
	std::thread drunkTd(&Drunkard::Update, John);

	minerTd.join();
	wifeTd.join();
	drunkTd.join();

    //dispatch any delayed messages
    Dispatch->DispatchDelayedMessages();

	//printQueuedText();
	
	if (i > 0 && i%30 == 0) {
		SetTextColor(FOREGROUND_BLUE| FOREGROUND_RED | FOREGROUND_GREEN);
		std::cout << "\nContinue story ? <y/N>" << std::endl;
		std::getline(std::cin, input);
	}

    Sleep(800);
  }

  //tidy up
  delete Bob;
  delete Elsa;
  delete John;

  //wait for a keypress before exiting
  PressAnyKeyToContinue();


  return 0;
}






