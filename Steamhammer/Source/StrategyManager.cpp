#include "StrategyManager.h"
#include "NEATCommander.h"
#include "Bases.h"
#include "CombatCommander.h"
#include "MapTools.h"

#include "ProductionManager.h"

#include "UnitUtil.h"

using namespace UAlbertaBot;

StrategyManager::StrategyManager() 
{
}

StrategyManager & StrategyManager::Instance() 
{
    static StrategyManager instance;
    return instance;
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


void StrategyManager::queryNetworkEvaluation()
{
    if (!canPlanBuildOrderNow())
    {
        return;
    }

    //BuildOrder & buildOrder = BOSSManager::Instance().getBuildOrder();
    BuildOrder buildOrder = NEATCommander::Instance().getMacroCommands();
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