#include <fstream>
#include <time.h>
#include <string>
#include <thread>
#include <windows.h>
#include <SFML/Graphics.hpp>
#include <map>
#include <mutex>
#include <set>

#include "Locations.h"
#include "Miner.h"
#include "MinersWife.h"
#include "Drunkard.h"
#include "EntityManager.h"
#include "MessageDispatcher.h"
#include "misc/ConsoleUtils.h"
#include "EntityNames.h"
#include "MessageTypes.h"

std::ofstream os;

int guiManager();
std::mutex guiMtx;
std::atomic<bool> exitGui = false;
struct {
	location_type minerLoc = goldmine;
	State<Miner>* minerSt = nullptr;
	State<MinersWife>* wifeSt = nullptr;
	State<Drunkard>* drunkardSt = nullptr;
	int minerMsg = -1;
	int wifeMsg = -1;
	int drunkardMsg = -1;
} guiData;

int main()
{
  // Convenient console resizing (800x800)
  HWND consoleWnd = GetConsoleWindow();
  RECT r;
  GetWindowRect(consoleWnd, &r);
  MoveWindow(consoleWnd, r.left, r.top, 800, 600, true);

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

  // create the GUI thread
  std::thread guiTd(&guiManager);

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

	//send pertinent data to GUI
	guiMtx.lock();
	guiData.minerLoc = Bob->Location();

	guiData.minerSt = Bob->GetFSM()->CurrentState();
	guiData.wifeSt = Elsa->GetFSM()->CurrentState();
	guiData.drunkardSt = John->GetFSM()->CurrentState();

	guiMtx.unlock();
	
	// do 30 more iterations?
	if (i > 0 && i%30 == 0) {
		coutQueue->send("\nContinue story ? <y/N>\n",
			FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
		
		input = coutQueue->getLine();

		coutQueue->send((condition(input) ? "Starting 30 more iterations." : "The end."),
			FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
	}

	Sleep(800);
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

  //end the GUI thread
  exitGui = true;
  guiTd.join();

  return 0;
}


/* ------------------------ */
/* Graphical User Interface */
/* ------------------------ */
// The GUI's code has noticeably been written in a great hurry,
// please forgive its lack of readability and reusability. 

int guiManager() {
	// Watch dispatched messages
	std::set<Telegram> msgCopies;
	std::mutex copiesLock;
	Dispatch->copyMessagesIn(&msgCopies, &copiesLock);

	// Window
	const int wWidth = 900, wHeight = 600;
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(wWidth, wHeight), "West World With Messaging", sf::Style::Titlebar | sf::Style::Close, settings);
	window.setFramerateLimit(30);

	// Some positions
	std::map<location_type, sf::Vector2f> minerPos = {
		{ goldmine, sf::Vector2f(170.f, 130.f) },
		{ saloon, sf::Vector2f(420.f, 240.f) },
		{ shack, sf::Vector2f(710.f, 140.f) },
		{ bank, sf::Vector2f(420.f, 100.f) }
	};

	std::map<std::string, sf::Vector2f> minerFsmPos = {
		{ "class QuenchThirst", sf::Vector2f(45.f, 350.f) },
		{ "class DefendHonor", sf::Vector2f(212.f, 350.f) },
		{ "class EnterMineAndDigForNugget", sf::Vector2f(45.f, 440.f) },
		{ "class GoHomeAndSleepTilRested", sf::Vector2f(212.f, 440.f) },
		{ "class VisitBankAndDepositGold", sf::Vector2f(45.f, 535.f) },
		{ "class EatStew", sf::Vector2f(212.f, 535.f) }
	};
	std::map<std::string, sf::Vector2f> wifeFsmPos = {
		{ "class DoHouseWork", sf::Vector2f(324.f, 350.f) },
		{ "class CookStew", sf::Vector2f(494.f, 350.f) },
		{ "class VisitBathroom", sf::Vector2f(410.f, 460.f) }
	};
	std::map<std::string, sf::Vector2f> drunkardFsmPos = {
		{ "class GetDrunk", sf::Vector2f(604.f, 350.f) },
		{ "class SleepTilRested", sf::Vector2f(828.f, 350.f) },
		{ "class SeekFight", sf::Vector2f(604.f, 494.f) },
		{ "class GetInFight", sf::Vector2f(828.f, 494.f) }
	};

	// Text
	sf::Font font; font.loadFromFile("Resources/BouWeste.ttf");

	sf::Text mousePosTxt; mousePosTxt.setFont(font);
	mousePosTxt.setCharacterSize(12); mousePosTxt.setFillColor(sf::Color::Black);
	mousePosTxt.setString("Mouse | x: 0 | y: 0");

	sf::Text minerFsmTxt; minerFsmTxt.setFont(font);
	minerFsmTxt.setCharacterSize(12); minerFsmTxt.setFillColor(sf::Color::Black);
	minerFsmTxt.setString("Miner  FSM");

	sf::Text wifeFsmTxt; wifeFsmTxt.setFont(font);
	wifeFsmTxt.setCharacterSize(12); wifeFsmTxt.setFillColor(sf::Color::Black);
	wifeFsmTxt.setString("Wife  FSM");

	sf::Text drunkardFsmTxt; drunkardFsmTxt.setFont(font);
	drunkardFsmTxt.setCharacterSize(12); drunkardFsmTxt.setFillColor(sf::Color::Black);
	drunkardFsmTxt.setString("Drunkard  FSM");

	sf::Text minerMsgTxt; minerMsgTxt.setFont(font);
	minerMsgTxt.setCharacterSize(10); minerMsgTxt.setFillColor(sf::Color::Black);

	sf::Text wifeMsgTxt; wifeMsgTxt.setFont(font);
	wifeMsgTxt.setCharacterSize(10); wifeMsgTxt.setFillColor(sf::Color::Black);

	sf::Text drunkardMsgTxt; drunkardMsgTxt.setFont(font);
	drunkardMsgTxt.setCharacterSize(10); drunkardMsgTxt.setFillColor(sf::Color::Black);
	

	// Cursors
	sf::CircleShape minerCursor(20.f);
	minerCursor.setFillColor(sf::Color::Transparent);
	minerCursor.setOutlineColor(sf::Color::Red);
	minerCursor.setOutlineThickness(4);
	minerCursor.setPosition(minerFsmPos["class EnterMineAndDigForNugget"]);

	sf::CircleShape wifeCursor(20.f);
	wifeCursor.setFillColor(sf::Color::Transparent);
	wifeCursor.setOutlineColor(sf::Color::Green);
	wifeCursor.setOutlineThickness(4);
	wifeCursor.setPosition(wifeFsmPos["class DoHouseWork"]);

	sf::CircleShape drunkardCursor(20.f);
	drunkardCursor.setFillColor(sf::Color::Transparent);
	drunkardCursor.setOutlineColor(sf::Color::Blue);
	drunkardCursor.setOutlineThickness(4);
	drunkardCursor.setPosition(drunkardFsmPos["class SleepTilRested"]);

	// Textures
	sf::Texture tMinerFsm;    tMinerFsm.loadFromFile("Resources/miner_fsm.png");       tMinerFsm.setSmooth(true);
	sf::Texture tWifeFsm;     tWifeFsm.loadFromFile("Resources/minersWife_fsm.png");   tWifeFsm.setSmooth(true);
	sf::Texture tDrunkardFsm; tDrunkardFsm.loadFromFile("Resources/drunkard_fsm.png"); tDrunkardFsm.setSmooth(true);

	sf::Texture tCharac;      tCharac.loadFromFile("Resources/characters.png");
	sf::Texture tMine;        tMine.loadFromFile("Resources/gold_mine.png", sf::IntRect(250, 40, 230, 200)); tMine.setSmooth(true);
	sf::Texture tSaloon;      tSaloon.loadFromFile("Resources/saloon.png");            tSaloon.setSmooth(true);
	sf::Texture tHouse;       tHouse.loadFromFile("Resources/house.png");              tHouse.setSmooth(true);
	sf::Texture tBank;        tBank.loadFromFile("Resources/bank.png");                tBank.setSmooth(true);

	
	// Sprites
	const float fsmScale = 0.8f;
	sf::Sprite minerFsm;    minerFsm.setTexture(tMinerFsm);       minerFsm.setScale(fsmScale, fsmScale);
	sf::Sprite wifeFsm;     wifeFsm.setTexture(tWifeFsm);         wifeFsm.setScale(fsmScale, fsmScale);
	sf::Sprite drunkardFsm; drunkardFsm.setTexture(tDrunkardFsm); drunkardFsm.setScale(fsmScale, fsmScale);

	sf::Sprite mine; mine.setTexture(tMine);       mine.setScale(0.5f, 0.5f);
	sf::Sprite saloon; saloon.setTexture(tSaloon); saloon.setScale(0.5f, 0.5f);
	sf::Sprite house; house.setTexture(tHouse);    house.setScale(0.25f, 0.25f);
	sf::Sprite bank; bank.setTexture(tBank);       bank.setScale(0.4f, 0.4f);

	sf::Sprite miner; miner.setTexture(tCharac);       miner.setTextureRect(sf::IntRect(292, 64, 23, 32));
	sf::Sprite wife; wife.setTexture(tCharac);         wife.setTextureRect(sf::IntRect(357, 160, 23, 32));
	sf::Sprite drunkard; drunkard.setTexture(tCharac); drunkard.setTextureRect(sf::IntRect(260, 32, 23, 32));

	// Positionning
	sf::Vector2i mousePos(0, 0);

	float minerFsm_w = minerFsm.getGlobalBounds().width;
	float minerFsm_h = minerFsm.getGlobalBounds().height;
	float wifeFsm_w = wifeFsm.getGlobalBounds().width;

	minerFsm.setPosition(0, wHeight - minerFsm_h - 20);
	wifeFsm.setPosition(minerFsm_w + 10, wHeight - minerFsm_h - 20);
	drunkardFsm.setPosition(minerFsm_w + wifeFsm_w + 20, wHeight - minerFsm_h - 20);

	minerFsmTxt.setPosition(100, wHeight - minerFsm_h - 50);
	wifeFsmTxt.setPosition(400, wHeight - minerFsm_h - 50);
	drunkardFsmTxt.setPosition(700, wHeight - minerFsm_h - 50);

	mine.setPosition(50, 50);
	saloon.setPosition(320, 150);
	house.setPosition(680, 50);
	bank.setPosition(400, 10);

	miner.setPosition(150, 140);
	wife.setPosition(750, 140);
	drunkard.setPosition(460, 240);

	minerMsgTxt.setPosition(100, 172);
	wifeMsgTxt.setPosition(790, 172);
	drunkardMsgTxt.setPosition(500, 272);

	sf::Clock chronoMiner, chronoWife, chronoDrunkard;

	// Display loop
	while (window.isOpen() && !exitGui) {

		// Inputs
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			//sf::Vector2f clickPos = (sf::Vector2f)(sf::Mouse::getPosition(window));
			//.setPosition(clickPos);
		}

		mousePos = sf::Mouse::getPosition(window);
		mousePosTxt.setString("Mouse | x: " + std::to_string(mousePos.x) + " | y: " + std::to_string(mousePos.y));

		// Logic
		guiMtx.lock();

		miner.setPosition(minerPos[guiData.minerLoc]);
		minerMsgTxt.setPosition(miner.getPosition().x - minerMsgTxt.getLocalBounds().width, miner.getPosition().y + miner.getLocalBounds().height);

		minerCursor.setPosition(minerFsmPos[typeid(*(guiData.minerSt)).name()]);
		wifeCursor.setPosition(wifeFsmPos[typeid(*(guiData.wifeSt)).name()]);
		drunkardCursor.setPosition(drunkardFsmPos[typeid(*(guiData.drunkardSt)).name()]);

		guiMtx.unlock();

		if (chronoMiner.getElapsedTime().asSeconds() >= 3) {
			minerMsgTxt.setString("");
			chronoMiner.restart();
		}
		if (chronoWife.getElapsedTime().asSeconds() >= 3) {
			wifeMsgTxt.setString("");
			chronoWife.restart();
		}
		if (chronoDrunkard.getElapsedTime().asSeconds() >= 3) {
			drunkardMsgTxt.setString("");
			chronoDrunkard.restart();
		}

		copiesLock.lock();
		if (!msgCopies.empty())
		{
			//read the telegram from the front of the queue
			const Telegram& telegram = *msgCopies.begin();

			//find the recipient
			BaseGameEntity* pSender = EntityMgr->GetEntityFromID(telegram.Sender);

			switch (pSender->ID()) {
				case ent_Miner_Bob:
					minerMsgTxt.setString(MsgToStr(telegram.Msg));
					chronoMiner.restart();
				break;
				case ent_Elsa:
					wifeMsgTxt.setString(MsgToStr(telegram.Msg));
					chronoWife.restart();
				break;
				case ent_Drunkard_John:
					drunkardMsgTxt.setString(MsgToStr(telegram.Msg));
					chronoDrunkard.restart();
				break;
			}

			//remove it from the queue
			msgCopies.erase(msgCopies.begin());
		}
		copiesLock.unlock();

		// Outputs
		window.clear(sf::Color::White);

		window.draw(minerFsm);
		window.draw(wifeFsm);
		window.draw(drunkardFsm);

		window.draw(minerFsmTxt);
		window.draw(wifeFsmTxt);
		window.draw(drunkardFsmTxt);

		window.draw(minerCursor);
		window.draw(wifeCursor);
		window.draw(drunkardCursor);

		window.draw(mine);
		window.draw(saloon);
		window.draw(house);
		window.draw(bank);

		window.draw(miner);
		window.draw(wife);
		window.draw(drunkard);

		//window.draw(mousePosTxt);
		window.draw(minerMsgTxt);
		window.draw(wifeMsgTxt);
		window.draw(drunkardMsgTxt);

		window.display();
	}

	return 0;
}
