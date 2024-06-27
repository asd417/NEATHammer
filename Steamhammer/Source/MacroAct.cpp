#include "MacroAct.h"

#include "BuildingPlacer.h"
#include "ProductionManager.h"
#include "The.h"
#include "UnitUtil.h"

#include <regex>

using namespace UAlbertaBot;

// Map unit type names to unit types.
static std::map<std::string, BWAPI::UnitType> _unitTypesByName;

void MacroAct::initializeUnitTypesByName()
{
    if (_unitTypesByName.size() == 0)       // if not already initialized
    {
        for (BWAPI::UnitType unitType : BWAPI::UnitTypes::allUnitTypes())
        {
            std::string typeName = TrimRaceName(unitType.getName());
            std::replace(typeName.begin(), typeName.end(), '_', ' ');
            std::transform(typeName.begin(), typeName.end(), typeName.begin(), ::tolower);
            _unitTypesByName[typeName] = unitType;
        }
    }
}

BWAPI::UnitType MacroAct::getUnitTypeFromString(const std::string & s) const
{
    auto it = _unitTypesByName.find(s);
    if (it != _unitTypesByName.end())
    {
        return (*it).second;
    }
    return BWAPI::UnitTypes::Unknown;
}

MacroAct::MacroAct() 
    : _type(MacroActs::Default)
    , _tileLocation(BWAPI::TilePositions::None)
    , _parent(nullptr)
{
}

MacroAct::MacroAct (BWAPI::UnitType t) 
    : _unitType(t)
    , _type(MacroActs::Unit) 
    , _tileLocation(BWAPI::TilePositions::None)
    , _parent(nullptr)
{
}

/// <summary>
/// For placing a building precisely on a pre-chosen tile.
/// </summary>
/// <param name="t"></param>
/// <param name="tile"></param>
MacroAct::MacroAct(BWAPI::UnitType t, BWAPI::TilePosition tile)
    : _unitType(t)
    , _type(MacroActs::Unit)
    , _tileLocation(tile)
    , _parent(nullptr)
{
}

MacroAct::MacroAct(BWAPI::UnitType t, BWAPI::Unit parent) 
    : _unitType(t)
    , _type(MacroActs::Unit)
    , _tileLocation(BWAPI::TilePositions::None)
    , _parent(parent)
{
}

MacroAct::MacroAct(BWAPI::TechType t)
    : _techType(t)
    , _type(MacroActs::Tech) 
    , _tileLocation({0,0})
    , _parent(nullptr)
{
}

MacroAct::MacroAct(BWAPI::UpgradeType t) 
    : _upgradeType(t)
    , _type(MacroActs::Upgrade) 
    , _tileLocation({ 0,0 })
    , _parent(nullptr)
{
}

MacroAct::MacroAct(MacroCommandType t)
    : _macroCommandType(t)
    , _type(MacroActs::Command)
    , _tileLocation(BWAPI::TilePositions::None)
    , _parent(nullptr)
{
}

UAlbertaBot::MacroAct::MacroAct(MacroCommand t, const BWAPI::TilePosition& tile)
    : _macroCommandType(t)
    , _type(MacroActs::Command)
    , _tileLocation(tile)
    , _parent(nullptr)
{
}

MacroAct::MacroAct(MacroCommandType t, int amount)
    : _macroCommandType(t, amount)
    , _type(MacroActs::Command)
    , _tileLocation(BWAPI::TilePositions::None)
    , _parent(nullptr)
{
}

MacroAct::MacroAct(MacroCommandType t, BWAPI::UnitType type)
    : _macroCommandType(t, type)
    , _type(MacroActs::Command)
    , _tileLocation(BWAPI::TilePositions::None)
    , _parent(nullptr)
{
}

size_t MacroAct::type() const
{
    return _type;
}

bool MacroAct::isUnit() const 
{
    return _type == MacroActs::Unit; 
}

bool MacroAct::isWorker() const
{
    return _type == MacroActs::Unit && _unitType.isWorker();
}

// The unit type is a mobile combat unit for purposes of counting resources spent toward combat.
// Different from UnitUtil::IsCombatSimUnit() and UnitUtil::IsCombatUnit().
bool MacroAct::isCombatUnit() const
{
    if (_type == MacroActs::Unit)
    {
        if (_unitType.isBuilding() || _unitType.isWorker() || _unitType.isDetector() || _unitType.spaceProvided() > 0)
        {
            return false;
        }

        return
            _unitType.canAttack() ||                             // includes carriers and reavers
            _unitType == BWAPI::UnitTypes::Terran_Medic ||
            _unitType == BWAPI::UnitTypes::Terran_Science_Vessel ||
            _unitType == BWAPI::UnitTypes::Protoss_High_Templar ||
            _unitType == BWAPI::UnitTypes::Protoss_Dark_Archon;
    }

    return false;
}

bool MacroAct::isTech() const
{ 
    return _type == MacroActs::Tech; 
}

bool MacroAct::isUpgrade() const 
{ 
    return _type == MacroActs::Upgrade; 
}

bool MacroAct::isCommand() const 
{ 
    return _type == MacroActs::Command; 
}

bool MacroAct::isBuilding()	const 
{ 
    return _type == MacroActs::Unit && _unitType.isBuilding(); 
}

bool MacroAct::isAddon() const
{
    return _type == MacroActs::Unit && _unitType.isAddon();
}

bool MacroAct::isRefinery()	const
{ 
    return _type == MacroActs::Unit && _unitType.isRefinery();
}

// The standard supply unit, ignoring the hatchery (which provides 1 supply) and nexus/CC.
bool MacroAct::isSupply() const
{
    return isUnit() &&
        (  _unitType == BWAPI::UnitTypes::Terran_Supply_Depot
        || _unitType == BWAPI::UnitTypes::Protoss_Pylon
        || _unitType == BWAPI::UnitTypes::Zerg_Overlord);
}

BWAPI::UnitType MacroAct::getUnitType() const
{
    //UAB_ASSERT(_type == MacroActs::Unit, "getUnitType of non-unit");
    return _unitType;
}

BWAPI::TechType MacroAct::getTechType() const
{
    //UAB_ASSERT(_type == MacroActs::Tech, "getTechType of non-tech");
    return _techType;
}

BWAPI::UpgradeType MacroAct::getUpgradeType() const
{
    //(_type == MacroActs::Upgrade, "getUpgradeType of non-upgrade");
    return _upgradeType;
}

MacroCommand MacroAct::getCommandType() const
{
    //UAB_ASSERT(_type == MacroActs::Command, "getCommandType of non-command");
    return _macroCommandType;
}


BWAPI::TilePosition MacroAct::getTileLocation() const
{
    return _tileLocation;
}

// Supply required if this is produced.
// It is NOT THE SAME as the supply required to have one of the units; it is the extra supply needed
// to make one of them.
int MacroAct::supplyRequired() const
{
    if (isUnit())
    {
        if (_unitType.isTwoUnitsInOneEgg())
        {
            // Zerglings or scourge.
            return 2;
        }
        if (_unitType == BWAPI::UnitTypes::Zerg_Lurker)
        {
            // Difference between hydralisk supply and lurker supply.
            return 2;
        }
        if (_unitType == BWAPI::UnitTypes::Zerg_Guardian || _unitType == BWAPI::UnitTypes::Zerg_Devourer)
        {
            // No difference between mutalisk supply and guardian/devourer supply.
            return 0;
        }
        return _unitType.supplyRequired();
    }
    return 0;
}

// NOTE Because upgrades vary in price with level, this is context dependent.
int MacroAct::mineralPrice() const
{
    if (isCommand()) {
        //if (_macroCommandType.getType() == MacroCommandType::ExtractorTrickDrone ||
        //    _macroCommandType.getType() == MacroCommandType::ExtractorTrickZergling) {
        //    // 50 for the extractor and 50 for the unit. Never mind that you get some back.
        //    return 100;
        //}
        return 0;
    }
    if (isUnit())
    {
        // Special case for sunks and spores, which are built from drones by Building Manager:
        // Return the price of the creep colony, not the total price (125) and not the final morph price (50).
        if (getUnitType() == BWAPI::UnitTypes::Zerg_Sunken_Colony || getUnitType() == BWAPI::UnitTypes::Zerg_Spore_Colony)
        {
            return 75;
        }

        return _unitType.mineralPrice();
    }
    if (isTech())
    {
        return _techType.mineralPrice();
    }
    if (isUpgrade())
    {
        if (_upgradeType.maxRepeats() > 1 && BWAPI::Broodwar->self()->getUpgradeLevel(_upgradeType) > 0)
        {
            return _upgradeType.mineralPrice(1 + BWAPI::Broodwar->self()->getUpgradeLevel(_upgradeType));
        }
        return _upgradeType.mineralPrice();
    }

    //UAB_ASSERT(false, "bad MacroAct");
    return 0;
}

// NOTE Because upgrades vary in price with level, this is context dependent.
int MacroAct::gasPrice() const
{
    if (isCommand()) {
        return 0;
    }
    if (isUnit())
    {
        return _unitType.gasPrice();
    }
    if (isTech())
    {
        return _techType.gasPrice();
    }
    if (isUpgrade())
    {
        if (_upgradeType.maxRepeats() > 1 && BWAPI::Broodwar->self()->getUpgradeLevel(_upgradeType) > 0)
        {
            return _upgradeType.gasPrice(1 + BWAPI::Broodwar->self()->getUpgradeLevel(_upgradeType));
        }
        return _upgradeType.gasPrice();
    }

    //UAB_ASSERT(false, "bad MacroAct");
    return 0;
}

BWAPI::UnitType MacroAct::whatBuilds() const
{
    if (isUnit())
    {
        // Special case for sunks and spores, which are built from drones by Building Manager.
        if (isUnit() && (getUnitType() == BWAPI::UnitTypes::Zerg_Sunken_Colony || getUnitType() == BWAPI::UnitTypes::Zerg_Spore_Colony))
        {
            return BWAPI::UnitTypes::Zerg_Drone;
        }

        return _unitType.whatBuilds().first;
    }
    if (isTech())
    {
        return _techType.whatResearches();
    }
    if (isUpgrade())
    {
        return _upgradeType.whatUpgrades();
    }
    if (isCommand())
    {
        return BWAPI::UnitTypes::None;
    }

    //UAB_ASSERT(false, "bad MacroAct");
    return BWAPI::UnitTypes::Unknown;
}

std::string MacroAct::getName() const
{
    if (isUnit())
    {
        return _unitType.getName();
    }
    if (isTech())
    {
        return _techType.getName();
    }
    if (isUpgrade())
    {
        return _upgradeType.getName();
    }
    if (isCommand())
    {
        return _macroCommandType.getName();
    }

    //UAB_ASSERT(false, "bad MacroAct");
    return "error";
}

// The given unit can produce the macro act.
bool MacroAct::isProducer(BWAPI::Unit unit) const
{
    BWAPI::UnitType producerType = whatBuilds();

    // If the producerType is a lair, a hive will do as well. Ditto spire and greater spire.
    // Note: Burrow research in a hatchery can also be done in a lair or hive, but we rarely want to.
    // Ignore the possibility so that we don't accidentally waste lair time.
    if (!(
        producerType == unit->getType() ||
        producerType == BWAPI::UnitTypes::Zerg_Lair && unit->getType() == BWAPI::UnitTypes::Zerg_Hive ||
        producerType == BWAPI::UnitTypes::Zerg_Spire && unit->getType() == BWAPI::UnitTypes::Zerg_Greater_Spire
        ))
    {
        return false;
    }

    if (!unit->isCompleted())  { return false; }
    if (unit->isTraining())    { return false; }
    if (unit->isLifted())      { return false; }
    if (!unit->isPowered())    { return false; }
    if (unit->isUpgrading())   { return false; }
    if (unit->isResearching()) { return false; }

    if (isAddon())
    {
        // Already has an addon, or is otherwise unable to make one.
        if (!unit->canBuildAddon())
        {
            return false;
        }

        // There is latency between ordering an addon and the addon starting.
        if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Build_Addon)
        {
            return false;
        }
    }

    if (isUnit())
    {
        // Make sure the candidate hasn't already been given a morph order.
        // NOTE This is part of a workaround for a bug in BWAPI 4.4.0 latency compensation.
        //      Even without the bug, it can't hurt.
        BWAPI::Order order = the.micro.getMicroState(unit).getOrder();
        if (order == BWAPI::Orders::ZergUnitMorph || order == BWAPI::Orders::ZergBuildingMorph)
        {
            return false;
        }

        // Check for required tech buildings.
        typedef std::pair<BWAPI::UnitType, int> ReqPair;
        for (const ReqPair & pair : getUnitType().requiredUnits())
        {
            BWAPI::UnitType requiredType = pair.first;
            if (requiredType.isAddon())
            {
                if (!unit->getAddon() || (unit->getAddon()->getType() != requiredType))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

// Record the units which are currently able to carry out this macro act.
// For example, the idle barracks which can produce a marine.
// It gives a warning if you call it for a command, which has no producer.
void MacroAct::getCandidateProducers(std::vector<BWAPI::Unit> & candidates) const
{
    if (isCommand())
    {
        //UAB_ASSERT(false, "no producer of a command");
        return;
    }

    if (_parent && _parent->exists() && isProducer(_parent))
    {
        candidates.push_back(_parent);
        return;
    }

    for (BWAPI::Unit unit : the.self()->getUnits())
    {
        if (isProducer(unit))
        {
            candidates.push_back(unit);
        }
    }
}

// The item can eventually be produced; a producer exists and may be free someday.
bool MacroAct::hasEventualProducer() const
{
    BWAPI::UnitType producerType = whatBuilds();

    for (BWAPI::Unit unit : the.self()->getUnits())
    {
        // A producer is good if it is the right type and doesn't suffer from
        // any condition that makes it unable to produce ever.
        if (unit->getType() == producerType &&
            unit->isPowered() &&     // replacing a pylon is a separate queue item
            !unit->isLifted() &&     // lifting/landing a building is a separate queue item
            (!producerType.isAddon() || unit->getAddon() == nullptr))
        {
            return true;
        }

        // NOTE An addon may be required on the producer. This doesn't check.
    }

    // We didn't find a producer. We can't make it.
    return false;
}

// The item can potentially be produced soon-ish; a producer is on hand and not too busy.
bool MacroAct::hasPotentialProducer() const
{
    BWAPI::UnitType producerType = whatBuilds();

    for (BWAPI::Unit unit : the.self()->getUnits())
    {
        // A producer is good if it is the right type and doesn't suffer from
        // any condition that makes it unable to produce for a long time.
        // Producing something else only makes it busy for a short time,
        // except that research takes a long time.
        if (unit->getType() == producerType &&
            unit->isPowered() &&     // replacing a pylon is a separate queue item
            !unit->isLifted() &&     // lifting/landing a building will be a separate queue item when implemented
            !unit->isUpgrading() &&
            !unit->isResearching() &&
            (!producerType.isAddon() || unit->getAddon() == nullptr))
        {
            return true;
        }

        // NOTE An addon may be required on the producer. This doesn't check.
    }

    // BWAPI::Broodwar->printf("missing producer for %s", getName().c_str());

    // We didn't find a producer. We can't make it.
    return false;
}

// Check the units needed for producing a unit type, beyond its producer.
bool MacroAct::hasTech() const
{
    // If it's not a unit, let's assume we're good.
    if (!isUnit())
    {
        return true;
    }

    // What we have.
    std::set<BWAPI::UnitType> ourUnitTypes;
    for (BWAPI::Unit unit : the.self()->getUnits())
    {
        ourUnitTypes.insert(unit->getType());
    }

    // What we need. We only pay attention to the unit type, not the count,
    // which is needed only for merging archons and dark archons (which is not done via MacroAct).
    for (const std::pair<BWAPI::UnitType, int> & typeAndCount : getUnitType().requiredUnits())
    {
        BWAPI::UnitType requiredType = typeAndCount.first;
        if (ourUnitTypes.find(requiredType) == ourUnitTypes.end() &&
            (ProductionManager::Instance().isOutOfBook() || !requiredType.isBuilding() || !BuildingManager::Instance().isBeingBuilt(requiredType)))
        {
            // BWAPI::Broodwar->printf("missing tech: %s requires %s", getName().c_str(), requiredType.getName().c_str());
            // We don't have a type we need. We don't have the tech.
            return false;
        }
    }

    // We have the technology.
    return true;
}

// Can we produce the target now?
bool MacroAct::canProduce(BWAPI::Unit producer) const
{
    if (isCommand())
    {
        // NOTE Not always correct for an extractor trick (it may execute but do nothing).
        return true;
    }

    //UAB_ASSERT(producer != nullptr, "producer was null");

    if (ProductionManager::Instance().meetsReservedResources(*this))
    {
        if (isUnit())
        {
            // Special case for sunks and spores, which are built from drones by Building Manager.
            if (getUnitType() == BWAPI::UnitTypes::Zerg_Sunken_Colony || getUnitType() == BWAPI::UnitTypes::Zerg_Spore_Colony)
            {
                return BWAPI::Broodwar->canMake(BWAPI::UnitTypes::Zerg_Creep_Colony, producer);
            }
            if (getUnitType() == BWAPI::UnitTypes::Protoss_Archon)
            {
                //Searches 5 tile radius
                BWAPI::Unitset htNearBy = BWAPI::Broodwar->getUnitsInRadius({ _tileLocation.x * 32, _tileLocation.y * 32 }, 5 * 32, BWAPI::Filter::GetType == BWAPI::UnitTypes::Protoss_High_Templar);
                return htNearBy.size() >= 2;
            }
            if (getUnitType() == BWAPI::UnitTypes::Protoss_Dark_Archon)
            {
                //Searches 5 tile radius
                BWAPI::Unitset htNearBy = BWAPI::Broodwar->getUnitsInRadius({ _tileLocation.x * 32, _tileLocation.y * 32 }, 5 * 32, BWAPI::Filter::GetType == BWAPI::UnitTypes::Protoss_Dark_Templar);
                return htNearBy.size() >= 2;
            }

            return BWAPI::Broodwar->canMake(getUnitType(), producer);
        }
        if (isTech())
        {
            return BWAPI::Broodwar->canResearch(getTechType(), producer);
        }
        if (isUpgrade())
        {
            return BWAPI::Broodwar->canUpgrade(getUpgradeType(), producer);
        }

        //UAB_ASSERT(false, "bad MacroAct");
    }

    return false;
}

// Create a unit or start research.

void MacroAct::produce(BWAPI::Unit producer) const
{
    //UAB_ASSERT(producer != nullptr, "producer was null");

    // A terran add-on.
    if (isAddon())
    {
        the.micro.Make(producer, getUnitType());
    }
    // A building that the building manager is responsible for.
    // The building manager handles sunkens and spores.
    else if (isBuilding() && UnitUtil::NeedsWorkerBuildingType(getUnitType()))
    {
        BWAPI::UnitType type = getUnitType();
        BWAPI::TilePosition desiredPosition = _tileLocation;
        
        Building b = Building(type, _tileLocation);
        b.desiredPosition = _tileLocation;
        desiredPosition = BuildingManager::Instance().getBuildingLocation(b);
        
        if(desiredPosition != BWAPI::TilePositions::None) BuildingManager::Instance().addBuildingTask(*this, desiredPosition, producer, false);
    }
    // A non-building unit, or a morphed zerg building.
    else if (isUnit())
    {
        //Produce Archon
        if (getUnitType() == BWAPI::UnitTypes::Protoss_Archon)
        {
            //Searches 5 tile radius
            BWAPI::Unitset htNearBy = BWAPI::Broodwar->getUnitsInRadius({ _tileLocation.x * 32, _tileLocation.y * 32 }, 5 * 32, BWAPI::Filter::GetType == BWAPI::UnitTypes::Protoss_High_Templar);
            
            if (htNearBy.size() >= 2) {
                int i = 0;
                BWAPI::Unit left, right;
                for (BWAPI::Unit u : htNearBy)
                {
                    if (i == 0) left = u;
                    if (i == 1) right = u;
                    i++;
                    if (i == 2) break;
                }
                the.micro.MergeArchon(left,right);
            }
            return;
        }
        //Produce Dark Archon
        if (getUnitType() == BWAPI::UnitTypes::Protoss_Dark_Archon)
        {
            //Searches 5 tile radius
            BWAPI::Unitset dtNearBy = BWAPI::Broodwar->getUnitsInRadius({ _tileLocation.x * 32, _tileLocation.y * 32 }, 5 * 32, BWAPI::Filter::GetType == BWAPI::UnitTypes::Protoss_Dark_Templar);
            if (dtNearBy.size() >= 2) {
                int i = 0;
                BWAPI::Unit left, right;
                for (BWAPI::Unit u : dtNearBy)
                {
                    if (i == 0) left = u;
                    if (i == 1) right = u;
                    i++;
                    if (i == 2) break;
                }
                the.micro.MergeArchon(left, right);
            }
            return;
        }
        the.micro.Make(producer, getUnitType());
    }
    else if (isTech())
    {
        producer->research(getTechType());
    }
    else if (isUpgrade())
    {
        producer->upgrade(getUpgradeType());
    }
    else
    {
        //UAB_ASSERT(false, "bad MacroAct");
    }
}

std::string UAlbertaBot::MacroAct::toString()
{
    return std::string("");
}
