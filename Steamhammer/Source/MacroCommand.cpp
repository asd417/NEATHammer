#include "MacroCommand.h"

using namespace UAlbertaBot;

// Default constructor for when the value doesn't matter.
MacroCommand::MacroCommand()
    : _type(MacroCommandType::None)
    , _amount(0)
    , _unitType(BWAPI::UnitTypes::None)
{
}

MacroCommand::MacroCommand(MacroCommandType type)
    : _type(type)
    , _amount(0)
    , _unitType(BWAPI::UnitTypes::None)
{
    UAB_ASSERT(!hasNumericArgument(type), "missing MacroCommand argument");
}

MacroCommand::MacroCommand(MacroCommandType type, int amount)
    : _type(type)
    , _amount(amount)
    , _unitType(BWAPI::UnitTypes::None)
{
    UAB_ASSERT(hasNumericArgument(type), "extra MacroCommand argument");
}

MacroCommand::MacroCommand(MacroCommandType type, int amount, int amount2)
    : _type(type)
    , _amount(amount)
    , _amount2(amount2)
    , _unitType(BWAPI::UnitTypes::None)
{
    UAB_ASSERT(hasNumericArgument2(type), "extra MacroCommand argument");
}

UAlbertaBot::MacroCommand::MacroCommand(MacroCommandType type, int amount, int amount2, BWAPI::UnitType unitType)
    : _type(type)
    , _amount(amount)
    , _amount2(amount2)
    , _unitType(unitType)
{
}

MacroCommand::MacroCommand(MacroCommandType type, BWAPI::UnitType unitType)
    : _type(type)
    , _amount(0)
    , _unitType(unitType)
{
    UAB_ASSERT(hasUnitArgument(type), "extra MacroCommand argument");
}

const std::list<MacroCommandType> MacroCommand::allCommandTypes()
{
    return std::list<MacroCommandType>
    { 
      MacroCommandType::StartGas
    , MacroCommandType::StopGas
    , MacroCommandType::Aggressive
    , MacroCommandType::Defensive
    , MacroCommandType::PostWorker
    , MacroCommandType::UnpostWorkers
    , MacroCommandType::Nonadaptive
    , MacroCommandType::Lift
    , MacroCommandType::QueueBarrier
    , MacroCommandType::AssignToSquad
    , MacroCommandType::RemoveFromSquad
    , MacroCommandType::SetSquadOrder
    };
}

// The command has a numeric argument, the _amount.
bool MacroCommand::hasNumericArgument(MacroCommandType t)
{
    return
        t == MacroCommandType::SetSquadOrder //used as squad index
        || t == MacroCommandType::AssignToSquad //used as squad index
        || t == MacroCommandType::RemoveFromSquad //used as squad index

        ;
}
// The command has a second numeric argument, the _amount2
bool UAlbertaBot::MacroCommand::hasNumericArgument2(MacroCommandType t)
{
    return
        t == MacroCommandType::SetSquadOrder //used as order type
        || t == MacroCommandType::AssignToSquad //used as search radius 
        || t == MacroCommandType::RemoveFromSquad //used as search radius 
        ;
}

// The command has a unit type argument, the _unitType.
bool MacroCommand::hasUnitArgument(MacroCommandType t)
{
    return 
        t == MacroCommandType::Lift
        || t == MacroCommandType::AssignToSquad
        || t == MacroCommandType::RemoveFromSquad
        ;
}

const std::string MacroCommand::getName(MacroCommandType t)
{
    if (t == MacroCommandType::StartGas)
    {
        return "go start gas";
    }
    if (t == MacroCommandType::StopGas)
    {
        return "go stop gas";
    }
    if (t == MacroCommandType::Aggressive)
    {
        return "go aggressive";
    }
    if (t == MacroCommandType::Defensive)
    {
        return "go defensive";
    }
    if (t == MacroCommandType::PostWorker)
    {
        return "go post worker";
    }
    if (t == MacroCommandType::UnpostWorkers)
    {
        return "go unpost workers";
    }
    if (t == MacroCommandType::Nonadaptive)
    {
        return "go nonadaptive";
    }
    if (t == MacroCommandType::Lift)
    {
        return "go lift";
    }
    if (t == MacroCommandType::QueueBarrier)
    {
        return "go queue barrier";
    }
    if (t == MacroCommandType::AssignToSquad)
    {
        return "assign to squad";
    }
    if (t == MacroCommandType::RemoveFromSquad)
    {
        return "remove to squad";
    }
    if (t == MacroCommandType::SetSquadOrder)
    {
        return "order squad";
    }

    UAB_ASSERT(t == MacroCommandType::None, "unrecognized MacroCommandType");
    return "go none";
}

const std::string MacroCommand::getName() const
{
    if (hasNumericArgument(_type))
    {
        std::stringstream name;
        name << getName(_type) << " " << _amount;
        return name.str();
    }
    else
    {
        return getName(_type);
    }
}
