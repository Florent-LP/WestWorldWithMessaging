#ifndef MESSAGE_TYPES
#define MESSAGE_TYPES

#include <string>

enum message_type
{
  Msg_HiHoneyImHome,
  Msg_StewReady,
  Msg_YouWinThisTime,
  Msg_Provoke,
  Msg_RivalSaysHi,
  Msg_RivalSaysBye,
  Msg_AcceptFight
};


inline std::string MsgToStr(int msg)
{
  switch (msg)
  {
  case Msg_HiHoneyImHome:
    
    return "HiHoneyImHome"; 

  case Msg_StewReady:
    
    return "StewReady";

  case Msg_YouWinThisTime:
    
    return "YouWinThisTime";

  case Msg_Provoke:
    
    return "Provoke";

  case Msg_RivalSaysHi:
    
    return "RivalSaysHi";

  case Msg_RivalSaysBye:
    
    return "RivalSaysBye";

  case Msg_AcceptFight:
    
    return "AcceptFight";

  default:

    return "Not recognized!";
  }
}

#endif