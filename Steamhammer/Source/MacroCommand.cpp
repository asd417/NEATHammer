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
    { MacroCommandType::Scout
    , MacroCommandType::ScoutIfNeeded
    , MacroCommandType::ScoutLocation
    , MacroCommandType::ScoutOnceOnly
    , MacroCommandType::StartGas
    , MacroCommandType::StopGas
    , MacroCommandType::StealGas
    , MacroCommandType::Aggressive
    , MacroCommandType::Defensive
    , MacroCommandType::PullWorkers
    , MacroCommandType::PullWorkersLeaving
    , MacroCommandType::ReleaseWorkers
    , MacroCommandType::PostWorker
    , MacroCommandType::UnpostWorkers
    , MacroCommandType::QueueBarrier
    };
}

// The command has a numeric argument, the _amount.
bool MacroCommand::hasNumericArgument(MacroCommandType t)
{
    return
        t == MacroCommandType::PullWorkers ||
        t == MacroCommandType::PullWorkersLeaving;
}

//Lift command was the only one with the unit argument
bool UAlbertaBot::MacroCommand::hasUnitArgument(MacroCommandType t)
{
    return false;
}

const std::string MacroCommand::getName(MacroCommandType t)
{
    if (t == MacroCommandType::Scout)
    {
        return "go scout";
    }
    if (t == MacroCommandType::ScoutIfNeeded)
    {
        return "go scout if needed";
    }
    if (t == MacroCommandType::ScoutLocation)
    {
        return "go scout location";
    }
    if (t == MacroCommandType::ScoutOnceOnly)
    {
        return "go scout once around";
    }

    if (t == MacroCommandType::StartGas)
    {
        return "go start gas";
    }
    if (t == MacroCommandType::StopGas)
    {
        return "go stop gas";
    }
    if (t == MacroCommandType::StealGas)
    {
        return "go steal gas";
    }
    if (t == MacroCommandType::Aggressive)
    {
        return "go aggressive";
    }
    if (t == MacroCommandType::Defensive)
    {
        return "go defensive";
    }
    if (t == MacroCommandType::PullWorkers)
    {
        return "go pull workers";
    }
    if (t == MacroCommandType::PullWorkersLeaving)
    {
        return "go pull workers leaving";
    }
    if (t == MacroCommandType::ReleaseWorkers)
    {
        return "go release workers";
    }
    if (t == MacroCommandType::PostWorker)
    {
        return "go post worker";
    }
    if (t == MacroCommandType::UnpostWorkers)
    {
        return "go unpost workers";
    }
    if (t == MacroCommandType::QueueBarrier)
    {
        return "go queue barrier";
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
