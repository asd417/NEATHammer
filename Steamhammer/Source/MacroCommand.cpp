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
    //UAB_ASSERT(!hasNumericArgument(type), "missing MacroCommand argument");
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

// The command has a numeric argument, the _amount.
bool MacroCommand::hasNumericArgument(MacroCommandType t)
{
    return t == MacroCommandType::PullWorkers;
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
    if (t == MacroCommandType::PullWorkers)
    {
        return "go pull workers";
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
