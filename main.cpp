#include <fstream>
#include <time.h>
#include <string>

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

// For Visual Studio 2008 compatiblity, I added the pthread library
// to the project since std::thread has been only added in 2011
#include "pthread.h"
template<class T>
void* updateWrapper(void* ptr) {
	((T*)ptr)->Update();
	return NULL;
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

  pthread_t t0, t1, t2;

  //run Bob, Elsa and John through a few Update calls
  std::string input = "Y";
  for (int i = 0; input == "Y" || input == "y"; i++)
  { 
	pthread_create(&t0, NULL, updateWrapper<Miner>, Bob);
	pthread_create(&t1, NULL, updateWrapper<MinersWife>, Elsa);
	pthread_create(&t2, NULL, updateWrapper<Drunkard>, John);

	pthread_join(t0, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

    //dispatch any delayed messages
    Dispatch->DispatchDelayedMessages();
	
	// Do 30 more iterations ?
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






