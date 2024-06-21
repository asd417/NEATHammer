#include <fstream>

#include "ParseUtils.h"
#include "JSONTools.h"

#include "BuildOrder.h"
#include "Random.h"
#include "StrategyManager.h"

#include <regex>

// Parse the configuration file.
// Parse manual commands.
// Provide a few simple parsing routines for wider use.

using namespace UAlbertaBot;

// Parse the JSON configuration file into Config:: variables.
void ParseUtils::ParseConfigFile(const std::string & filename)
{
    rapidjson::Document doc;

    // Calculate our race and the matchup as C strings.
    // The race is spelled out: Terran Protoss Zerg Unknown
    // where "Unknown" means the enemy picked Random.
    // The matchup is abbreviated: ZvT

    const std::string ourRaceStr(BWAPI::Broodwar->self()->getRace().getName());
    const std::string theirRaceStr(BWAPI::Broodwar->enemy()->getRace().getName());
    const std::string matchupStr(ourRaceStr.substr(0, 1) + 'v' + theirRaceStr.substr(0, 1));

    const char * ourRace = ourRaceStr.c_str();
    const char * matchup = matchupStr.c_str();

    // Number of starting locations on the map.
    const int mapSize = BWAPI::Broodwar->getStartLocations().size();
    UAB_ASSERT(mapSize >= 2 && mapSize <= 8, "bad map size");
    const std::string mapWeightString = std::string("Weight") + std::string("012345678").at(mapSize);

    std::string config = FileUtils::ReadFile(filename);

    if (config.length() == 0)
    {
        std::cout << "Config File Not Found\n";
        return;
    }

    Config::ConfigFile::ConfigFileFound = true;
    

    bool parsingFailed = doc.Parse(config.c_str()).HasParseError();
    if (parsingFailed)
    {
        return;
    }

    // Parse the Bot Info
    if (doc.HasMember("Bot Info") && doc["Bot Info"].IsObject())
    {
        const rapidjson::Value & info = doc["Bot Info"];
        JSONTools::ReadString("BotName", info, Config::BotInfo::BotName);
        JSONTools::ReadString("Authors", info, Config::BotInfo::Authors);
        JSONTools::ReadBool("PrintInfoOnStart", info, Config::BotInfo::PrintInfoOnStart);
    }

    // Parse the BWAPI Options
    if (doc.HasMember("BWAPI") && doc["BWAPI"].IsObject())
    {
        const rapidjson::Value & bwapi = doc["BWAPI"];
        JSONTools::ReadInt("SetLocalSpeed", bwapi, Config::BWAPIOptions::SetLocalSpeed);
        JSONTools::ReadInt("SetFrameSkip", bwapi, Config::BWAPIOptions::SetFrameSkip);
        JSONTools::ReadBool("UserInput", bwapi, Config::BWAPIOptions::EnableUserInput);
        JSONTools::ReadBool("CompleteMapInformation", bwapi, Config::BWAPIOptions::EnableCompleteMapInformation);
    }

    // Parse the Micro Options
    if (doc.HasMember("Micro") && doc["Micro"].IsObject())
    {
        const rapidjson::Value & micro = doc["Micro"];

        Config::Micro::KiteWithRangedUnits = GetBoolByRace("KiteWithRangedUnits", micro);
        Config::Micro::WorkersDefendRush = GetBoolByRace("WorkersDefendRush", micro);
        
        Config::Micro::RetreatMeleeUnitShields = GetIntByRace("RetreatMeleeUnitShields", micro);
        Config::Micro::RetreatMeleeUnitHP = GetIntByRace("RetreatMeleeUnitHP", micro);
        Config::Micro::CombatSimRadius = GetIntByRace("CombatSimRadius", micro);
        Config::Micro::ScoutDefenseRadius = GetIntByRace("ScoutDefenseRadius", micro);
    }

    // Parse the Macro Options
    /*if (doc.HasMember("Macro") && doc["Macro"].IsObject())
    {
        const rapidjson::Value & macro = doc["Macro"];
        JSONTools::ReadInt("BOSSFrameLimit", macro, Config::Macro::BOSSFrameLimit);
        Config::Macro::ProductionJamFrameLimit = GetIntByRace("ProductionJamFrameLimit", macro);
        Config::Macro::WorkersPerRefinery = GetIntByRace("WorkersPerRefinery", macro);
        Config::Macro::WorkersPerPatch = GetDoubleByRace("WorkersPerPatch", macro);
        Config::Macro::AbsoluteMaxWorkers = GetIntByRace("AbsoluteMaxWorkers", macro);
        Config::Macro::BuildingSpacing = GetIntByRace("BuildingSpacing", macro);
        JSONTools::ReadInt("PylonSpacing", macro, Config::Macro::PylonSpacing);
    }*/
    if (doc.HasMember("NEAT") && doc["NEAT"].IsObject())
    {
        const rapidjson::Value& neat = doc["NEAT"];
        JSONTools::ReadBool("Train", neat, Config::NEAT::Train);
        JSONTools::ReadString("TrainingServerIP", neat, Config::NEAT::TrainingServerIP);
        JSONTools::ReadBool("LoadNetworkFromJSON", neat, Config::NEAT::LoadNetworkFromJSON);
        JSONTools::ReadString("NetworkJSON", neat, Config::NEAT::NetworkJSON);
        JSONTools::ReadInt("WinScore", neat, Config::NEAT::WinScore);
        JSONTools::ReadInt("EnemyShowScore", neat, Config::NEAT::EnemyShowScore);
        JSONTools::ReadInt("FitnessScore_Mineral_Divider", neat, Config::NEAT::FitnessScore_Mineral_Divider);
        JSONTools::ReadInt("FitnessScore_Gas_Divider", neat, Config::NEAT::FitnessScore_Gas_Divider);
        JSONTools::ReadInt("SubtractFitnessScore", neat, Config::NEAT::SubtractFitnessScore);
        JSONTools::ReadBool("AutoSurrender", neat, Config::NEAT::AutoSurrender);
    }

    // Parse the Debug Options
    if (doc.HasMember("Debug") && doc["Debug"].IsObject())
    {
        const rapidjson::Value & debug = doc["Debug"];
        JSONTools::ReadBool("DrawGameInfo", debug, Config::Debug::DrawGameInfo);
        JSONTools::ReadBool("DrawBuildOrderSearchInfo", debug, Config::Debug::DrawBuildOrderSearchInfo);
        JSONTools::ReadBool("DrawQueueFixInfo", debug, Config::Debug::DrawQueueFixInfo);
        JSONTools::ReadBool("DrawUnitHealthBars", debug, Config::Debug::DrawUnitHealthBars);
        JSONTools::ReadBool("DrawWorkerInfo", debug, Config::Debug::DrawWorkerInfo);
        JSONTools::ReadBool("DrawProductionInfo", debug, Config::Debug::DrawProductionInfo);
        JSONTools::ReadBool("DrawScoutInfo", debug, Config::Debug::DrawScoutInfo);
        JSONTools::ReadBool("DrawSquadInfo", debug, Config::Debug::DrawSquadInfo);
        JSONTools::ReadBool("DrawClusters", debug, Config::Debug::DrawClusters);
        JSONTools::ReadBool("DrawDefenseClusters", debug, Config::Debug::DrawDefenseClusters);
        JSONTools::ReadBool("DrawCombatSimInfo", debug, Config::Debug::DrawCombatSimulationInfo);
        JSONTools::ReadBool("DrawBuildingInfo", debug, Config::Debug::DrawBuildingInfo);
        JSONTools::ReadBool("DrawStaticDefensePlan", debug, Config::Debug::DrawStaticDefensePlan);
        JSONTools::ReadBool("DrawModuleTimers", debug, Config::Debug::DrawModuleTimers);
        JSONTools::ReadBool("DrawEnemyUnitInfo", debug, Config::Debug::DrawEnemyUnitInfo);
        JSONTools::ReadBool("DrawUnitCounts", debug, Config::Debug::DrawUnitCounts);
        JSONTools::ReadBool("DrawHiddenEnemies", debug, Config::Debug::DrawHiddenEnemies);
        JSONTools::ReadBool("DrawMapInfo", debug, Config::Debug::DrawMapInfo);
        JSONTools::ReadBool("DrawMapGrid", debug, Config::Debug::DrawMapGrid);
        JSONTools::ReadBool("DrawMapDistances", debug, Config::Debug::DrawMapDistances);
        JSONTools::ReadBool("DrawTerrainHeights", debug, Config::Debug::DrawTerrainHeights);
        JSONTools::ReadBool("DrawBaseInfo", debug, Config::Debug::DrawBaseInfo);
        JSONTools::ReadBool("DrawExpoScores", debug, Config::Debug::DrawExpoScores);
        JSONTools::ReadBool("DrawStrategyBossInfo", debug, Config::Debug::DrawStrategyBossInfo);
        JSONTools::ReadBool("DrawUnitTargets", debug, Config::Debug::DrawUnitTargets);
        JSONTools::ReadBool("DrawUnitOrders", debug, Config::Debug::DrawUnitOrders);
        JSONTools::ReadBool("DrawLurkerTactics", debug, Config::Debug::DrawLurkerTactics);
        JSONTools::ReadBool("DrawMicroState", debug, Config::Debug::DrawMicroState);
        JSONTools::ReadBool("DrawReservedBuildingTiles", debug, Config::Debug::DrawReservedBuildingTiles);
        JSONTools::ReadBool("DrawResourceAmounts", debug, Config::Debug::DrawResourceAmounts); 
    }

    // Parse the Tool options.
    if (doc.HasMember("Tools") && doc["Tools"].IsObject())
    {
        const rapidjson::Value & tool = doc["Tools"];

        JSONTools::ReadInt("MapGridSize", tool, Config::Tools::MAP_GRID_SIZE);
    }

    // Parse the IO options.
    if (doc.HasMember("IO") && doc["IO"].IsObject())
    {
        const rapidjson::Value & io = doc["IO"];

        JSONTools::ReadString("ErrorLogFilename", io, Config::IO::ErrorLogFilename);
        JSONTools::ReadBool("LogAssertToErrorFile", io, Config::IO::LogAssertToErrorFile);

        JSONTools::ReadString("StaticDirectory", io, Config::IO::StaticDir);
        JSONTools::ReadString("PreparedDataDirectory", io, Config::IO::PreparedDataDir);
        JSONTools::ReadString("ReadDirectory", io, Config::IO::ReadDir);
        JSONTools::ReadString("WriteDirectory", io, Config::IO::WriteDir);

        JSONTools::ReadString("OpeningTimingFile", io, Config::IO::OpeningTimingFile);
        
        JSONTools::ReadInt("MaxGameRecords", io, Config::IO::MaxGameRecords);

        Config::IO::ReadOpponentModel = GetBoolByRace("ReadOpponentModel", io);
        Config::IO::WriteOpponentModel = GetBoolByRace("WriteOpponentModel", io);
    }

    // Parse the Skills options.
    if (doc.HasMember("Skills") && doc["Skills"].IsObject())
    {
        const rapidjson::Value & skills = doc["Skills"];

        Config::Skills::SCHNAILMeansHuman = GetBoolByRace("SCHNAILMeansHuman", skills);
        Config::Skills::HumanOpponent = GetBoolByRace("HumanOpponent", skills);
        Config::Skills::SurrenderWhenHopeIsLost = GetBoolByRace("SurrenderWhenHopeIsLost", skills);

        Config::Skills::ScoutHarassEnemy = GetBoolByRace("ScoutHarassEnemy", skills);
        Config::Skills::GasSteal = GetBoolByRace("GasSteal", skills);

        JSONTools::ReadBool("Burrow", skills, Config::Skills::Burrow);
        JSONTools::ReadBool("UseCannonRangeBug", skills, Config::Skills::Burrow);

        JSONTools::ReadInt("MaxQueens", skills, Config::Skills::MaxQueens);
        JSONTools::ReadInt("MaxInfestedTerrans", skills, Config::Skills::MaxInfestedTerrans);
        JSONTools::ReadInt("MaxDefilers", skills, Config::Skills::MaxDefilers);
    }

    // Are we running under SCHNAIL?
    {
        // Do this in braces so the stream object gets destroyed right away.
        std::ifstream schnail;

        schnail.open(Config::IO::ReadDir + "schnail.env");
        Config::Skills::UnderSCHNAIL = schnail.good();
    }
    if (Config::Skills::UnderSCHNAIL && Config::Skills::SCHNAILMeansHuman)
    {
        // Override the configured value.
        Config::Skills::HumanOpponent = true;
    }

    // This depends on whether the opponent is a human, decided above.
    // Opening selection below may depend on the results.
    // File reading only happens if Config::IO::ReadOpponentModel is true.
    //OpponentModel::Instance().read();


    Config::ConfigFile::ConfigFileParsed = true;
}

void ParseUtils::ParseTextCommand(const std::string & commandString)
{
    std::stringstream ss(commandString);

    std::string command;
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    std::string variableName;
    std::transform(variableName.begin(), variableName.end(), variableName.begin(), ::tolower);

    std::string val;

    ss >> command;
    ss >> variableName;
    ss >> val;

    if (command == "/set")
    {
        // BWAPI options
        if (variableName == "setlocalspeed") { Config::BWAPIOptions::SetLocalSpeed = GetIntFromString(val); BWAPI::Broodwar->setLocalSpeed(Config::BWAPIOptions::SetLocalSpeed); }
        else if (variableName == "setframeskip") { Config::BWAPIOptions::SetFrameSkip = GetIntFromString(val); BWAPI::Broodwar->setFrameSkip(Config::BWAPIOptions::SetFrameSkip); }
        else if (variableName == "userinput") { Config::BWAPIOptions::EnableUserInput = GetBoolFromString(val); if (Config::BWAPIOptions::EnableUserInput) BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput); }
        else if (variableName == "completemapinformation") { Config::BWAPIOptions::EnableCompleteMapInformation = GetBoolFromString(val); if (Config::BWAPIOptions::EnableCompleteMapInformation) BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput); }
        
        // Micro Options
        else if (variableName == "workersdefendrush") { Config::Micro::WorkersDefendRush = GetBoolFromString(val); }
        else if (variableName == "combatsimradius") { Config::Micro::CombatSimRadius = GetIntFromString(val); }

        // Macro Options
        else if (variableName == "absolutemaxworkers") { Config::Macro::AbsoluteMaxWorkers = GetIntFromString(val); }

        // Debug Options
        else if (variableName == "errorlogfilename") { Config::IO::ErrorLogFilename = val; }
        else if (variableName == "drawgameinfo") { Config::Debug::DrawGameInfo = GetBoolFromString(val); }
        else if (variableName == "drawunithealthbars") { Config::Debug::DrawUnitHealthBars = GetBoolFromString(val); }
        else if (variableName == "drawproductioninfo") { Config::Debug::DrawProductionInfo = GetBoolFromString(val); }
        else if (variableName == "drawbuildordersearchinfo") { Config::Debug::DrawBuildOrderSearchInfo = GetBoolFromString(val); }
        else if (variableName == "drawscoutinfo") { Config::Debug::DrawScoutInfo = GetBoolFromString(val); }
        else if (variableName == "drawqueuefixinfo") { Config::Debug::DrawQueueFixInfo = GetBoolFromString(val); }
        else if (variableName == "drawenemyunitinfo") { Config::Debug::DrawEnemyUnitInfo = GetBoolFromString(val); }
        else if (variableName == "drawunitcounts") { Config::Debug::DrawUnitCounts = GetBoolFromString(val); }
        else if (variableName == "drawhiddenenemies") { Config::Debug::DrawHiddenEnemies = GetBoolFromString(val); }
        else if (variableName == "drawmoduletimers") { Config::Debug::DrawModuleTimers = GetBoolFromString(val); }
        else if (variableName == "drawcombatsiminfo") { Config::Debug::DrawCombatSimulationInfo = GetBoolFromString(val); }
        else if (variableName == "drawunittargets") { Config::Debug::DrawUnitTargets = GetBoolFromString(val); }
        else if (variableName == "drawunitorders") { Config::Debug::DrawUnitOrders = GetBoolFromString(val); }
        else if (variableName == "drawmicrostate") { Config::Debug::DrawMicroState = GetBoolFromString(val); }
        else if (variableName == "drawmapinfo") { Config::Debug::DrawMapInfo = GetBoolFromString(val); }
        else if (variableName == "drawmapgrid") { Config::Debug::DrawMapGrid = GetBoolFromString(val); }
        else if (variableName == "drawmapdistances") { Config::Debug::DrawMapDistances = GetBoolFromString(val); }
        else if (variableName == "drawterrainheights") { Config::Debug::DrawTerrainHeights = GetBoolFromString(val); }
        else if (variableName == "drawbaseinfo") { Config::Debug::DrawBaseInfo = GetBoolFromString(val); }
        else if (variableName == "drawexposcores") { Config::Debug::DrawExpoScores = GetBoolFromString(val); }
        else if (variableName == "drawstrategybossinfo") { Config::Debug::DrawStrategyBossInfo = GetBoolFromString(val); }
        else if (variableName == "drawsquadinfo") { Config::Debug::DrawSquadInfo = GetBoolFromString(val); }
        else if (variableName == "drawclusters") { Config::Debug::DrawClusters = GetBoolFromString(val); }
        else if (variableName == "drawdefenseclusters") { Config::Debug::DrawDefenseClusters = GetBoolFromString(val); }
        else if (variableName == "drawworkerinfo") { Config::Debug::DrawWorkerInfo = GetBoolFromString(val); }
        else if (variableName == "drawbuildinginfo") { Config::Debug::DrawBuildingInfo = GetBoolFromString(val); }
        else if (variableName == "drawstaticdefenseplan") { Config::Debug::DrawStaticDefensePlan = GetBoolFromString(val); }
        else if (variableName == "drawreservedbuildingtiles") { Config::Debug::DrawReservedBuildingTiles = GetBoolFromString(val); }
        else if (variableName == "drawresourceamounts") { Config::Debug::DrawResourceAmounts = GetBoolFromString(val); }

        else { UAB_ASSERT_WARNING(false, "Unknown variable name for /set: %s", variableName.c_str()); }
    }
    else
    {
        UAB_ASSERT_WARNING(false, "Unknown command: %s", command.c_str());
    }
}

BWAPI::Race ParseUtils::GetRace(const std::string & raceName)
{
    if (raceName == "Protoss")
    {
        return BWAPI::Races::Protoss;
    }

    if (raceName == "Terran")
    {
        return BWAPI::Races::Terran;
    }

    if (raceName == "Zerg")
    {
        return BWAPI::Races::Zerg;
    }

    if (raceName == "Random")
    {
        return BWAPI::Races::Random;
    }

    UAB_ASSERT_WARNING(false, "Race not found: %s", raceName.c_str());
    return BWAPI::Races::None;
}

// Parse a strategy declaration and return the chosen strategy.
// If a strategy, return it in stratName:
//   "11Gas10Pool"
// If a strategy mix declaration, pick the strategy randomly by weight:
//   { "Zerg" : [
//     { "Weight" : 30, "Strategy" : "4PoolFast" },
//     { "Weight" : 20, "Strategy" : "4PoolSlow" },
//     { "Weight" : 50, "Strategy" : "5Pool" }
//   ]},
// Return whether parsing was successful.
// Internal routine not for wider use.
bool ParseUtils::_ParseStrategy(
    const rapidjson::Value & item,
    std::string & stratName,        // updated and returned
    const std::string & mapWeightString,
    const std::string & raceString,
    const rapidjson::Value * strategyCombos)
{
    if (item.IsString())
    {
        stratName = std::string(item.GetString());
        return _LookUpStrategyCombo(item, stratName, mapWeightString, raceString, strategyCombos);
    }

    if (item.IsObject() && item.HasMember(raceString.c_str()))
    {
        if (item[raceString.c_str()].IsString())
        {
            stratName = std::string(item[raceString.c_str()].GetString());
            return _LookUpStrategyCombo(item[raceString.c_str()], stratName, mapWeightString, raceString, strategyCombos);
        }
        else if (item[raceString.c_str()].IsArray())
        {
            const rapidjson::Value & mix = item[raceString.c_str()];

            std::vector<std::string> strategies;    // strategy name
            std::vector<int> weights;               // cumulative weight of strategy
            int totalWeight = 0;                    // cumulative weight of last strategy (so far)

            // 1. Collect the weights and strategies.
            // Skip choices which are not allowed by a Requires: declaration.
            for (size_t i(0); i < mix.Size(); ++i)
            {
                // BWAPI::Broodwar->printf("at %s", mix[i]["Strategy"].GetString());
                if (mix[i].IsObject() &&
                    mix[i].HasMember("Strategy") && mix[i]["Strategy"].IsString() &&
                    _MeetsRequirements(mix[i]["Strategy"].GetString(), strategyCombos))
                {
                    int weight;
                    if (mix[i].HasMember(mapWeightString.c_str()) && mix[i][mapWeightString.c_str()].IsInt())
                    {
                        weight = mix[i][mapWeightString.c_str()].GetInt();
                    }
                    else if (mix[i].HasMember("Weight") && mix[i]["Weight"].IsInt())
                    {
                        weight = mix[i]["Weight"].GetInt();
                    }
                    else
                    {
                        continue;
                    }
                    strategies.push_back(mix[i]["Strategy"].GetString());
                    totalWeight += weight;
                    weights.push_back(totalWeight);
                }
                else
                {
                    return false;
                }
            }

            // 2. Choose a strategy at random by weight.
            // If none met their requirements, we'll fall through and fail.
            // BWAPI::Broodwar->printf("mix %d, weights %d", mix.Size(), weights.size());
            int w = Random::Instance().index(totalWeight);
            for (size_t i = 0; i < weights.size(); ++i)
            {
                if (w < weights[i])
                {
                    stratName = strategies[i];
                    return _LookUpStrategyCombo(item, stratName, mapWeightString, raceString, strategyCombos);
                }
            }
            UAB_ASSERT(false, "random strategy fell through");
        }
    }

    return false;
}

// Mutually recursive with _ParseStrategy above.
// This allows a strategy name in a strategy combo to refer to another strategy combo.
// DON'T CREATE A LOOP IN THE STRATEGY COMBOS or the program could fall into an infinite loop.
bool ParseUtils::_LookUpStrategyCombo(
    const rapidjson::Value & item,
    std::string & stratName,        // updated and returned
    const std::string & mapWeightString,
    const std::string & raceString,
    const rapidjson::Value * strategyCombos)
{
    if (strategyCombos && strategyCombos->HasMember(stratName.c_str()))
    {
        const rapidjson::Value & combo = (*strategyCombos)[stratName.c_str()];

        if (combo.IsString())
        {
            stratName = (*strategyCombos)[stratName.c_str()].GetString();
            return true;
        }
        if (combo.IsObject())
        {
            return _ParseStrategy(combo, stratName, mapWeightString, raceString, strategyCombos);
        }
        // If it failed its requirements, or if it's neither of those things, complain.
        return false;
    }

    // No need for further lookup. What we have is what we want.
    return true;
}

// The strategy meets its Require: requirements (if it has any).
// The strategy here may be either an opening object or a strategy combo, but
// in practice this check is made only for a strategy combo.
// Either may have Require:. 
bool ParseUtils::_MeetsRequirements(
    const std::string & itemName,
    const rapidjson::Value * strategyCombos)
{
    return true;

    /*
    bool isCombo =
        strategyCombos &&
        strategyCombos->HasMember(itemName.c_str()) &&
        (*strategyCombos)[itemName.c_str()].IsObject();
    const rapidjson::Value & combo = (*strategyCombos)[itemName.c_str()];
    
    const rapidjson::Value * item = isCombo
        ? &(*strategyCombos)[itemName.c_str()]
        : nullptr;

    const bool hasRequirements = item && item->IsObject() && item->HasMember("Require") && (*item)["Require"].IsObject();
    const bool hasIslandRequirement = hasRequirements &&
        (*item)["Require"].HasMember("island") && (*item)["Require"]["island"].IsBool();
    const bool requiresIsland = hasIslandRequirement && (*item)["Require"]["island"].GetBool();
    const bool isIsland = Bases::Instance().isIslandStart();
    if (isIsland && !requiresIsland)
    {
        BWAPI::Broodwar->printf("failed requirements for %s", itemName.c_str());
        return false;
    }

    // No requirements failed (possibly because no requirements exist). All good.
    BWAPI::Broodwar->printf("passed requirements for %s", itemName.c_str());
    return true;
    */
}

bool ParseUtils::GetBoolFromString(const std::string & str)
{
    std::string boolStr(str);
    std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);

    if (boolStr == "true")
    {
        return true;
    }
    if (boolStr == "false")
    {
        return false;
    }

    UAB_ASSERT_WARNING(false, "Unknown bool from string: %s", str.c_str());
    return false;
}

// Return an integer which may be different depending on our race. The forms are:
//     "Item" : 10
// and
//     "Item" : { "Zerg" : 1, "Protoss" : 2, "Terran" : 3 }
// Anything that is missing defaults to 0. So
//     "Item" : { "Zerg" : 1 }
// is the same as
//     "Item" : { "Zerg" : 1, "Protoss" : 0, "Terran" : 0 }
// "Item" itself must be given in one form or another, though, so we can catch typos.
int ParseUtils::GetIntByRace(const char * name, const rapidjson::Value & item)
{
    if (item.HasMember(name))
    {
        // "Item" : 10
        if (item[name].IsInt())
        {
            return item[name].GetInt();
        }

        // "Item" : { "Zerg" : 1, "Protoss" : 2, "Terran" : 3 }
        if (item[name].IsObject())
        {
            const std::string raceStr(BWAPI::Broodwar->self()->getRace().getName());
            if (item[name].HasMember(raceStr.c_str()) && item[name][raceStr.c_str()].IsInt())
            {
                return item[name][raceStr.c_str()].GetInt();
            }
        }
    }
    else
    {
        UAB_ASSERT_WARNING(false, "Wrong/missing config entry '%s'", name);
    }

    return 0;
}

// Similar to GetIntByRace(). Defaults to 0.0 (you probably don't want that).
double ParseUtils::GetDoubleByRace(const char * name, const rapidjson::Value & item)
{
    if (item.HasMember(name))
    {
        // "Item" : 10.0
        if (item[name].IsDouble())
        {
            return item[name].GetDouble();
        }

        // "Item" : { "Zerg" : 1.5, "Protoss" : 3.0, "Terran" : 4.5 }
        if (item[name].IsObject())
        {
            const std::string raceStr(BWAPI::Broodwar->self()->getRace().getName());
            if (item[name].HasMember(raceStr.c_str()) && item[name][raceStr.c_str()].IsDouble())
            {
                return item[name][raceStr.c_str()].GetDouble();
            }
        }
    }
    else
    {
        UAB_ASSERT_WARNING(false, "Wrong/missing config entry '%s'", name);
    }

    return 0.0;
}

// Similar to GetIntByRace(). Defaults to false.
bool ParseUtils::GetBoolByRace(const char * name, const rapidjson::Value & item)
{
    if (item.HasMember(name))
    {
        // "Item" : true
        if (item[name].IsBool())
        {
            return item[name].GetBool();
        }

        // "Item" : { "Zerg" : true, "Protoss" : false, "Terran" : false }
        if (item[name].IsObject())
        {
            const std::string raceStr(BWAPI::Broodwar->self()->getRace().getName());
            if (item[name].HasMember(raceStr.c_str()) && item[name][raceStr.c_str()].IsBool())
            {
                return item[name][raceStr.c_str()].GetBool();
            }
        }
    }
    else
    {
        UAB_ASSERT_WARNING(false, "Wrong/missing config entry '%s'", name);
    }

    return false;
}
