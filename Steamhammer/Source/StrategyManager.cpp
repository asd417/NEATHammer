#include "StrategyManager.h"
#include "NEATCommander.h"
#include "Bases.h"
#include "CombatCommander.h"
#include "MapTools.h"
#include "OpponentModel.h"
#include "ProductionManager.h"
#include "StrategyBossZerg.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

StrategyManager::StrategyManager() 
    : _selfRace(the.self()->getRace())
    , _enemyRace(the.enemy()->getRace())
    , _emptyBuildOrder(the.self()->getRace())
    , _openingGroup("")
    , _hasDropTech(false)
    , _highWaterBases(1)
    , _openingStaticDefenseDropped(false)
{
}

StrategyManager & StrategyManager::Instance() 
{
    static StrategyManager instance;
    return instance;
}

const BuildOrder & StrategyManager::getOpeningBookBuildOrder() const
{
    auto buildOrderIt = _strategies.find(Config::Strategy::StrategyName);

    // look for the build order in the build order map
    if (buildOrderIt != std::end(_strategies))
    {
        return (*buildOrderIt).second._buildOrder;
    }
    else
    {
        UAB_ASSERT_WARNING(false, "Strategy not found: %s, returning empty initial build order", Config::Strategy::StrategyName.c_str());
        return _emptyBuildOrder;
    }
}

// This is used for terran and protoss.
const bool StrategyManager::shouldExpandNow() const
{
    // if there is no place to expand to, we can't expand
    // We check mineral expansions only.
    if (the.map.getNextExpansion(false, true, false) == BWAPI::TilePositions::None)
    {
        return false;
    }

    // if we have idle workers then we need a new expansion
    if (WorkerManager::Instance().getNumIdleWorkers() > 3)
    {
        return true;
    }

    // if we have excess minerals, expand
    if (the.self()->minerals() > 600)
    {
        return true;
    }

    size_t numDepots =
        the.my.all.count(BWAPI::UnitTypes::Terran_Command_Center) +
        the.my.all.count(BWAPI::UnitTypes::Protoss_Nexus);
    int minute = the.now() / (24 * 60);

    // we will make expansion N after array[N] minutes have passed
    std::vector<int> expansionTimes = {4, 10, 15, 20, 24, 28, 35, 40};

    for (size_t i(0); i < expansionTimes.size(); ++i)
    {
        if (numDepots < (i+2) && minute > expansionTimes[i])
        {
            return true;
        }
    }

    return false;
}

void StrategyManager::addStrategy(const std::string & name, Strategy & strategy)
{
    _strategies[name] = strategy;
}

// Set _openingGroup depending on the current strategy, which in principle
// might be from the config file or from opening learning.
// This is part of initialization; it happens early on.
void StrategyManager::setOpeningGroup()
{
    auto buildOrderItr = _strategies.find(Config::Strategy::StrategyName);

    if (buildOrderItr != std::end(_strategies))
    {
        _openingGroup = (*buildOrderItr).second._openingGroup;
    }
}

const std::string & StrategyManager::getOpeningGroup() const
{
    return _openingGroup;
}

//Unused
const MetaPairVector StrategyManager::getBuildOrderGoal()
{
    /*if (_selfRace == BWAPI::Races::Protoss)
    {
        return getProtossBuildOrderGoal();
    }
    else if (_selfRace == BWAPI::Races::Terran)
    {
        return getTerranBuildOrderGoal();
    }
    else if (_selfRace == BWAPI::Races::Zerg)
    {
        return getZergBuildOrderGoal();
    }*/

    return MetaPairVector();
}
//Unused
const MetaPairVector StrategyManager::getProtossBuildOrderGoal()
{
    // the goal to return
    MetaPairVector goal;

    // These counts include uncompleted units (except for numNexusCompleted).
    int numPylons = the.my.all.count(BWAPI::UnitTypes::Protoss_Pylon);
    int numNexusCompleted = the.my.completed.count(BWAPI::UnitTypes::Protoss_Nexus);
    int numNexusAll = the.my.all.count(BWAPI::UnitTypes::Protoss_Nexus);
    int numGateways = the.my.all.count(BWAPI::UnitTypes::Protoss_Gateway);
    int numProbes = the.my.all.count(BWAPI::UnitTypes::Protoss_Probe);
    int numCannon = the.my.all.count(BWAPI::UnitTypes::Protoss_Photon_Cannon);
    int numObservers = the.my.all.count(BWAPI::UnitTypes::Protoss_Observer);
    int numZealots = the.my.all.count(BWAPI::UnitTypes::Protoss_Zealot);
    int numDragoons = the.my.all.count(BWAPI::UnitTypes::Protoss_Dragoon);
    int numDarkTemplar = the.my.all.count(BWAPI::UnitTypes::Protoss_Dark_Templar);
    int numReavers = the.my.all.count(BWAPI::UnitTypes::Protoss_Reaver);
    int numCorsairs = the.my.all.count(BWAPI::UnitTypes::Protoss_Corsair);
    int numCarriers = the.my.all.count(BWAPI::UnitTypes::Protoss_Carrier);

    bool hasStargate = the.my.completed.count(BWAPI::UnitTypes::Protoss_Stargate) > 0;

    int maxProbes = WorkerManager::Instance().getMaxWorkers();

    PlayerSnapshot enemies(the.enemy());

    BWAPI::Player self = the.self();

    //if (_openingGroup == "zealots")
    //{
    //    goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot, numZealots + 6));

    //    if (numNexusAll >= 3)
    //    {
    //        // In the end, switch to carriers; not so many dragoons.
    //        goal.push_back(MetaPair(BWAPI::UpgradeTypes::Carrier_Capacity, 1));
    //        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Carrier, numCarriers + 1));
    //        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon, numDragoons + 1));
    //    }
    //    else if (numNexusAll >= 2)
    //    {
    //        // Once we have a 2nd nexus, add dragoons.
    //        goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
    //        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon, numDragoons + 4));
    //    }

    //    // Once dragoons are out, get zealot speed.
    //    if (numDragoons > 0)
    //    {
    //        goal.push_back(MetaPair(BWAPI::UpgradeTypes::Leg_Enhancements, 1));
    //    }

    //    // Finally add templar archives.
    //    if (the.my.all.count(BWAPI::UnitTypes::Protoss_Citadel_of_Adun) > 0)
    //    {
    //        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Templar_Archives, 1));
    //    }

    //    // If we have templar archives, make
    //    // 1. a small fixed number of dark templar to force a reaction, and
    //    // 2. an even number of high templar to merge into archons (so the high templar disappear quickly).
    //    if (the.my.completed.count(BWAPI::UnitTypes::Protoss_Templar_Archives) > 0)
    //    {
    //        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, std::max(3, numDarkTemplar)));
    //        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_High_Templar, 2));
    //    }
    //}
    //else if (_openingGroup == "dragoons")
    //{
    //    goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
    //    goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon, numDragoons + numGateways));

    //    // Once we have a 2nd nexus, add reavers.
    //    if (numNexusAll >= 2)
    //    {
    //        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Reaver, numReavers + 1));
    //    }

    //    // If we have templar archives, make a small fixed number of DTs to force a reaction.
    //    if (the.my.completed.count(BWAPI::UnitTypes::Protoss_Templar_Archives) > 0)
    //    {
    //        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, std::max(3, numDarkTemplar)));
    //    }
    //}
    //else if (_openingGroup == "dark templar")
    //{
    //    goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, numDarkTemplar + 2));

    //    // Once we have a 2nd nexus, add dragoons.
    //    if (numNexusAll >= 2)
    //    {
    //        goal.push_back(MetaPair(BWAPI::UpgradeTypes::Singularity_Charge, 1));
    //        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon, numDragoons + 4));

    //        if (numGateways > 4)
    //        {
    //            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot, numZealots + numGateways - 6));
    //        }
    //        if (numZealots >= 6)
    //        {
    //            goal.push_back(MetaPair(BWAPI::UpgradeTypes::Leg_Enhancements, 1));
    //        }
    //    }
    //}
    //else if (_openingGroup == "drop")
    //{
    //    goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, numDarkTemplar + 2));

    //    // The drop prep is carried out entirely by the opening book.
    //    // Immediately transition into something else.
    //    _openingGroup = "dragoons";
    //}
    //else
    //{
    //    UAB_ASSERT_WARNING(false, "Unknown Opening Group: %s", _openingGroup.c_str());
    //    _openingGroup = "dragoons";    // we're misconfigured, but try to do something
    //}

    //// If we're doing a corsair thing and it's still working, slowly add more.
    //if (_enemyRace == BWAPI::Races::Zerg)
    //{
    //    if (hasStargate)
    //    {
    //        if (numCorsairs < 6 && self->deadUnitCount(BWAPI::UnitTypes::Protoss_Corsair) == 0 ||
    //            numCorsairs < 9 && enemies.count(BWAPI::UnitTypes::Zerg_Mutalisk > numCorsairs))
    //        {
    //            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Corsair, numCorsairs + 1));
    //        }
    //    }
    //    else
    //    {
    //        // No stargate. Make one if it's useful.
    //        if (enemies.count(BWAPI::UnitTypes::Zerg_Mutalisk) > 3)
    //        {
    //            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Stargate, 1));
    //        }
    //    }
    //}

    //// Maybe get some static defense against air attack.
    //const int enemyAirToGround =
    //    enemies.count(BWAPI::UnitTypes::Terran_Wraith) / 8 +
    //    enemies.count(BWAPI::UnitTypes::Terran_Battlecruiser) / 3 +
    //    enemies.count(BWAPI::UnitTypes::Protoss_Scout) / 5 +
    //    enemies.count(BWAPI::UnitTypes::Zerg_Mutalisk) / 6;
    //if (enemyAirToGround > 0)
    //{
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Protoss_Photon_Cannon, enemyAirToGround));
    //}

    //// Get observers if we have a second base, or if the enemy has cloaked units.
    //if (numNexusCompleted >= 2 || InformationManager::Instance().enemyHasCloakTech())
    //{
    //    goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));

    //    if (numObservers < 3 && self->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
    //    {
    //        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, numObservers + 1));
    //    }
    //}

    //// Make more probes, up to a limit.
    //goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Probe, std::min(maxProbes, numProbes + 8)));

    //// If the map has islands, get drop after we have 3 bases.
    //if (Config::Macro::ExpandToIslands && numNexusCompleted >= 3 && the.bases.hasIslandBases())
    //{
    //    goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Shuttle, 1));
    //}

    //// if we want to expand, insert a nexus into the build order
    //if (shouldExpandNow())
    //{
    //    goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
    //}

    return goal;
}
//Left as example. No function calls this.
const MetaPairVector StrategyManager::getTerranBuildOrderGoal()
{
    // the goal to return
    std::vector<MetaPair> goal;

    // These counts include uncompleted units.
    int numSCVs			= the.my.all.count(BWAPI::UnitTypes::Terran_SCV);
    int numCC           = the.my.all.count(BWAPI::UnitTypes::Terran_Command_Center);            
    int numRefineries   = the.my.all.count(BWAPI::UnitTypes::Terran_Refinery);            
    int numMarines      = the.my.all.count(BWAPI::UnitTypes::Terran_Marine);
    int numMedics       = the.my.all.count(BWAPI::UnitTypes::Terran_Medic);
    int numWraith       = the.my.all.count(BWAPI::UnitTypes::Terran_Wraith);
    int numVultures     = the.my.all.count(BWAPI::UnitTypes::Terran_Vulture);
    int numVessels		= the.my.all.count(BWAPI::UnitTypes::Terran_Science_Vessel);
    int numGoliaths		= the.my.all.count(BWAPI::UnitTypes::Terran_Goliath);
    int numTanks        = the.my.all.count(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
                        + the.my.all.count(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode);
    int numBCs          = the.my.all.count(BWAPI::UnitTypes::Terran_Battlecruiser);

    bool hasEBay		= the.my.completed.count(BWAPI::UnitTypes::Terran_Engineering_Bay) > 0;
    bool hasAcademy		= the.my.completed.count(BWAPI::UnitTypes::Terran_Academy) > 0;
    bool hasArmory		= the.my.completed.count(BWAPI::UnitTypes::Terran_Armory) > 0;
    bool hasPhysicsLab  = the.my.completed.count(BWAPI::UnitTypes::Terran_Physics_Lab) > 0;

    int maxSCVs = WorkerManager::Instance().getMaxWorkers();

    bool makeVessel = false;

    BWAPI::Player self = the.self();

    //if (_openingGroup == "anti-rush")
    //{
    //    int numRax = the.my.all.count(BWAPI::UnitTypes::Terran_Barracks);

    //    CombatCommander::Instance().setAggression(false);
    //    
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Marine, numMarines + numRax));
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_SCV, std::min(maxSCVs, numSCVs + 1)));
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Bunker, 1));
    //    
    //    if (self->minerals() > 250)
    //    {
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Barracks, numRax + 1));
    //    }

    //    // If we survived long enough, transition to something more interesting.
    //    if (numMarines >= 10)
    //    {
    //        _openingGroup = "bio";
    //        CombatCommander::Instance().setAggression(true);
    //    }
    //}
    //else if (_openingGroup == "bio")
    //{
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Marine, numMarines + 8));

    //    if (numMarines >= 8)
    //    {
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Academy, 1));
    //        if (numRefineries == 0)
    //        {
    //            goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Refinery, 1));
    //        }
    //    }
    //    if (hasAcademy)
    //    {
    //        // 1 medic for each 5 marines.
    //        int medicGoal = std::max(numMedics, numMarines / 5);
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Medic, medicGoal));
    //        if (!self->hasResearched(BWAPI::TechTypes::Stim_Packs))
    //        {
    //            goal.push_back(std::pair<MacroAct, int>(BWAPI::TechTypes::Stim_Packs, 1));
    //        }
    //        else
    //        {
    //            goal.push_back(std::pair<MacroAct, int>(BWAPI::UpgradeTypes::U_238_Shells, 1));
    //        }
    //    }
    //    if (numMarines > 16)
    //    {
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Engineering_Bay, 1));
    //    }
    //    if (hasEBay)
    //    {
    //        int weaponsUps = self->getUpgradeLevel(BWAPI::UpgradeTypes::Terran_Infantry_Weapons);
    //        if (weaponsUps == 0 &&
    //            !self->isUpgrading(BWAPI::UpgradeTypes::Terran_Infantry_Weapons))
    //        {
    //            goal.push_back(std::pair<MacroAct, int>(BWAPI::UpgradeTypes::Terran_Infantry_Weapons, 1));
    //        }
    //        else if (weaponsUps > 0 &&
    //            self->getUpgradeLevel(BWAPI::UpgradeTypes::Terran_Infantry_Armor) == 0 &&
    //            !self->isUpgrading(BWAPI::UpgradeTypes::Terran_Infantry_Armor))
    //        {
    //            goal.push_back(std::pair<MacroAct, int>(BWAPI::UpgradeTypes::Terran_Infantry_Armor, 1));
    //        }
    //        else if (weaponsUps > 0 &&
    //            weaponsUps < 3 &&
    //            !self->isUpgrading(BWAPI::UpgradeTypes::Terran_Infantry_Weapons) &&
    //            numVessels > 0)
    //        {
    //            goal.push_back(std::pair<MacroAct, int>(BWAPI::UpgradeTypes::Terran_Infantry_Weapons, weaponsUps + 1));
    //        }
    //    }

    //    // Add in tanks if they're useful.
    //    const int enemiesCounteredByTanks =
    //        the.your.seen.count(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode) +
    //        the.your.seen.count(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode) +
    //        the.your.seen.count(BWAPI::UnitTypes::Protoss_Dragoon) +
    //        the.your.seen.count(BWAPI::UnitTypes::Protoss_Reaver) +
    //        the.your.seen.count(BWAPI::UnitTypes::Zerg_Lurker) +
    //        the.your.seen.count(BWAPI::UnitTypes::Zerg_Ultralisk);
    //    const bool enemyHasStaticDefense =
    //        the.your.seen.count(BWAPI::UnitTypes::Terran_Bunker) > 0 ||
    //        the.your.seen.count(BWAPI::UnitTypes::Protoss_Photon_Cannon) > 0 ||
    //        the.your.seen.count(BWAPI::UnitTypes::Zerg_Sunken_Colony) > 0;
    //    if (enemiesCounteredByTanks > 0 || enemyHasStaticDefense)
    //    {
    //        int nTanksWanted;
    //        if (enemiesCounteredByTanks > 0)
    //        {
    //            nTanksWanted = std::min({ numMarines / 4, enemiesCounteredByTanks, numTanks + 2 });
    //        }
    //        else
    //        {
    //            nTanksWanted = numTanks;
    //            if (numTanks < 2)
    //            {
    //                nTanksWanted = numTanks + 1;
    //            }
    //        }
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, nTanksWanted));
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::TechTypes::Tank_Siege_Mode, 1));
    //    }

    //    /*
    //     * TODO the first battlecruiser never builds but jams the queue due to an unknown bug
    //    // Eventually make battlecruisers.
    //    if (numVessels > 0 && numRefineries >= 3 && the.my.completed.count(BWAPI::UnitTypes::Terran_Physics_Lab) == 0)
    //    {
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Physics_Lab, 1));
    //    }
    //    if (hasPhysicsLab && numRefineries >= 3)
    //    {
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Battlecruiser, numBCs + 1));
    //    }
    //    */
    //}
    //else if (_openingGroup == "vultures")
    //{
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Vulture, numVultures + 3));
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UpgradeTypes::Ion_Thrusters, 1));

    //    if (numVultures >= 6)
    //    {
    //        // The rush is over, transition out on the next call.
    //        _openingGroup = "tanks";
    //    }
    //    ProductionManager::Instance().liftBuildings(BWAPI::UnitTypes::Terran_Barracks);
    //}
    //else if (_openingGroup == "tanks")
    //{
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Vulture, numVultures + 4));
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, numTanks + 2));
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::TechTypes::Tank_Siege_Mode, 1));

    //    if (numVultures > 0)
    //    {
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UpgradeTypes::Ion_Thrusters, 1));
    //    }
    //    if (numTanks >= 6)
    //    {
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Goliath, numGoliaths + 4));
    //    }
    //    if (numGoliaths >= 4)
    //    {
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UpgradeTypes::Charon_Boosters, 1));
    //    }
    //    if (self->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode))
    //    {
    //        makeVessel = true;
    //    }
    //    ProductionManager::Instance().liftBuildings(BWAPI::UnitTypes::Terran_Barracks);
    //    if (hasEBay)
    //    {
    //        ProductionManager::Instance().liftBuildings(BWAPI::UnitTypes::Terran_Engineering_Bay);
    //    }
    //}
    //else if (_openingGroup == "drop")
    //{
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UpgradeTypes::Ion_Thrusters, 1));
    //    goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Vulture, numVultures + 1));

    //    // The drop prep is carried out entirely by the opening book.
    //    // Immediately transition into something else.
    //    if (_enemyRace == BWAPI::Races::Zerg)
    //    {
    //        _openingGroup = "bio";
    //    }
    //    else
    //    {
    //        _openingGroup = "tanks";
    //    }
    //}
    //else
    //{
    //    BWAPI::Broodwar->printf("Unknown Opening Group: %s", _openingGroup.c_str());
    //    _openingGroup = "bio";       // we're misconfigured, but try to do something
    //}

    //if (numCC > 1 || InformationManager::Instance().enemyHasCloakTech())
    //{
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Academy, 1));
    //    if (numRefineries == 0)
    //    {
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Refinery, 1));
    //    }
    //}

    //if (numCC > 0 && hasAcademy)
    //{
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Comsat_Station, the.my.completed.count(BWAPI::UnitTypes::Terran_Command_Center)));
    //}

    //if (makeVessel || InformationManager::Instance().enemyHasCloakTech())
    //{
    //    // Maintain 1 vessel to spot for the ground squad and 1 to go with the recon squad.
    //    if (numVessels < 2)
    //    {
    //        goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Science_Vessel, numVessels + 1));
    //    }
    //}

    //if (hasArmory &&
    //    self->getUpgradeLevel(BWAPI::UpgradeTypes::Terran_Vehicle_Weapons) == 0 &&
    //    !self->isUpgrading(BWAPI::UpgradeTypes::Terran_Vehicle_Weapons))
    //{
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UpgradeTypes::Terran_Vehicle_Weapons, 1));
    //}

    //// Make more SCVs, up to a limit. The anti-rush strategy makes its own SCVs.
    //if (_openingGroup != "anti-rush")
    //{
    //    goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_SCV, std::min(maxSCVs, numSCVs + 2 * numCC)));
    //}

    //// If the map has islands, get drop after we have 3 bases.
    //if (Config::Macro::ExpandToIslands && numCC >= 3 && the.bases.hasIslandBases())
    //{
    //    goal.push_back(MetaPair(BWAPI::UnitTypes::Terran_Dropship, 1));
    //}

    //if (shouldExpandNow())
    //{
    //    goal.push_back(std::pair<MacroAct, int>(BWAPI::UnitTypes::Terran_Command_Center, numCC + 1));
    //}

    return goal;
}

// Called to refill the production queue when it is empty.
void StrategyManager::freshProductionPlan()
{
    //if (_selfRace == BWAPI::Races::Zerg)
    //{
    //    ProductionManager::Instance().setBuildOrder(StrategyBossZerg::Instance().freshProductionPlan());
    //}
    //else
    //{
        performBuildOrderSearch();
    //}
}

void StrategyManager::performBuildOrderSearch()
{
    if (!canPlanBuildOrderNow())
    {
        return;
    }

    //BuildOrder & buildOrder = BOSSManager::Instance().getBuildOrder();
    BuildOrder buildOrder = NEATCommander::Instance().getMacroCommands();//why is buildOrder size set to 0 become 89150862?
    if (buildOrder.size() != 0)
    {
        ProductionManager::Instance().setBuildOrder(buildOrder);
        //BOSSManager::Instance().reset();
        NEATCommander::Instance().resetActions();
    }
}

// this will return true if any unit is on the first frame of its training time remaining
// this can cause issues for the build order search system so don't plan a search on these frames
bool StrategyManager::canPlanBuildOrderNow() const
{
    for (BWAPI::Unit unit : the.self()->getUnits())
    {
        if (unit->getRemainingTrainTime() == 0)
        {
            continue;
        }

        BWAPI::UnitType trainType = unit->getLastCommand().getUnitType();

        if (unit->getRemainingTrainTime() == trainType.buildTime())
        {
            return false;
        }
    }

    return true;
}

// Do we expect or plan to drop at some point during the game?
bool StrategyManager::dropIsPlanned() const
{
    // Don't drop in ZvZ.
    if (_selfRace == BWAPI::Races::Zerg && the.enemy()->getRace() == BWAPI::Races::Zerg)
    {
        return false;
    }

    // Otherwise plan drop if the opening says so, or if the map has islands to take.
    return
        getOpeningGroup() == "drop" ||
        Config::Macro::ExpandToIslands && the.bases.hasIslandBases();
}

// Whether we have the tech and transport to drop.
bool StrategyManager::hasDropTech()
{
    if (_selfRace == BWAPI::Races::Zerg)
    {
        // NOTE May be slow drop.
        return
            the.self()->getUpgradeLevel(BWAPI::UpgradeTypes::Ventral_Sacs) > 0 &&
            the.my.completed.count(BWAPI::UnitTypes::Zerg_Overlord) > 0;
    }
    if (_selfRace == BWAPI::Races::Protoss)
    {
        return the.my.completed.count(BWAPI::UnitTypes::Protoss_Shuttle) > 0;
    }
    if (_selfRace == BWAPI::Races::Terran)
    {
        return the.my.completed.count(BWAPI::UnitTypes::Terran_Dropship) > 0;
    }

    return false;
}