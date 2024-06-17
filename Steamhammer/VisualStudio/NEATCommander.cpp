#include "NEATCommander.h"
#include "MacroAct.h"

namespace UAlbertaBot
{
    NEATCommander& NEATCommander::Instance()
    {
        static NEATCommander instance;
        return instance;
    }
    BuildOrder& NEATCommander::getMacroCommands()
    {
        return BuildOrder(BWAPI::Broodwar->self()->getRace(), _actions);
    }
    void NEATCommander::resetActions()
    {
        _actions.clear();
    }
    void NEATCommander::evaluate()
    {

        int macroActType; //Unit, Tech, Upgrade, Command, Default
        int macroActTargetType; //_unitType or _techType or _upgradeType
        //MacroAct._parent is not necessary
        //MacroAct._macroLocation is used to automatically calculate macro location
        // it should instead be done by the network
        int macroCommandType;
        int amount1;
        int amount2;
        int tilePosX;
        int tilePosY;
        int unitType;
        
    }
    
}