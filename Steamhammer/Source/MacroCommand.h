#pragma once

#include "Common.h"

namespace UAlbertaBot
{

enum class MacroCommandType
    { None
    , StartGas
    , StopGas
    , Aggressive
    , Defensive
    , PostWorker
    , UnpostWorkers
    , Nonadaptive
    , Lift
    , AssignToSquad
    , RemoveFromSquad
    , SetSquadOrder
    , QueueBarrier
    };

class MacroCommand
{
    MacroCommandType	_type;
    int                 _amount;
    int                 _amount2;
    BWAPI::UnitType     _unitType;
    
public:

    MacroCommand();
    MacroCommand(MacroCommandType type);
    MacroCommand(MacroCommandType type, int amount);
    MacroCommand(MacroCommandType type, int amount, int amount2);
    MacroCommand(MacroCommandType type, int amount, int amount2, BWAPI::UnitType unitType);
    MacroCommand(MacroCommandType type, BWAPI::UnitType unitType);

    static const std::list<MacroCommandType> allCommandTypes();
    static bool hasNumericArgument(MacroCommandType t);
    static bool hasNumericArgument2(MacroCommandType t);
    static bool hasUnitArgument(MacroCommandType t);
    static const std::string getName(MacroCommandType t);

    MacroCommandType getType() const { return _type; }
    /// <summary>
    /// Generally used as squad index
    /// </summary>
    /// <returns></returns>
    int getAmount() const { return _amount; }
    int getAmount2() const { return _amount2; }
    BWAPI::UnitType getUnitType() const { return _unitType; }

    const std::string getName() const;

};
};
