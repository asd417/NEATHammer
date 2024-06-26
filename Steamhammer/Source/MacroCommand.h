#pragma once

#include "Common.h"

namespace UAlbertaBot
{

enum class MacroCommandType
    { None
    , Scout
    , StartGas
    , StopGas
    , Aggressive
    , Defensive
    , PullWorkers
    , ReleaseWorkers
    , PostWorker
    , UnpostWorkers
    //Terran specific macrocommands
    , SCAN
    , SPIDERMINE
    , SIEGEMODE
    , UNSIEGEMODE
    , NUKE
    , QueueBarrier
    };

class MacroCommand
{
    MacroCommandType	_type;
    int                 _amount;
    BWAPI::UnitType     _unitType;

public:

    MacroCommand();
    MacroCommand(MacroCommandType type);
    MacroCommand(MacroCommandType type, int amount);
    MacroCommand(MacroCommandType type, BWAPI::UnitType unitType);

    static const std::list<MacroCommandType> allCommandTypes();
    static bool hasNumericArgument(MacroCommandType t);
    static bool hasUnitArgument(MacroCommandType t);
    static const std::string getName(MacroCommandType t);

    MacroCommandType getType() const { return _type; }
    int getAmount() const { return _amount; }
    BWAPI::UnitType getUnitType() const { return _unitType; }

    const std::string getName() const;

};
};
