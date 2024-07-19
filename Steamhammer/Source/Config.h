
#pragma once

#include "BWAPI.h"
#include <cassert>

namespace Config
{
    namespace ConfigFile
    {
        extern bool ConfigFileFound;
        extern bool ConfigFileParsed;
        extern std::string ConfigFileLocation;
    }

    namespace BotInfo
    {
        extern std::string BotName;
        extern std::string Authors;
        extern bool PrintInfoOnStart;
    }

    namespace IO
    {
        extern std::string ErrorLogFilename;
        extern bool LogAssertToErrorFile;

        extern std::string StaticDir;
        extern std::string PreparedDataDir;
        extern std::string ReadDir;
        extern std::string WriteDir;
        extern int MaxGameRecords;
    }

    namespace Skills
    {
        extern bool UnderSCHNAIL;
        extern bool SCHNAILMeansHuman;
        extern bool HumanOpponent;
        extern bool SurrenderWhenHopeIsLost;

        extern bool ScoutHarassEnemy;
    }

    namespace NEAT
    {
        extern bool Train;
        extern std::string TrainingServerIP;
        extern int RetryTimer;
        extern bool LoadNetworkFromJSON;
        extern std::string NetworkJSON;

        extern int WinScore;
        extern int ArmyKillScore;
        extern int BuildingScore;
        extern int UnitCompleteScore;

        extern bool AutoSurrender;

        extern bool PrintNetworkOutput;

        extern bool LogInputVector;
        extern std::string InputLogFileName;
        extern bool LogOutputVector;
        extern std::string OutputLogFileName;
        extern bool LogOutputDecision;
        extern std::string DecisionLogFileName;

        extern int OutputSpaceAvailabilityScore;
        extern int BuildingKillScore;
    }
    namespace BWAPIOptions
    {
        extern int SetLocalSpeed;
        extern int SetFrameSkip;
        extern bool EnableUserInput;
        extern bool EnableCompleteMapInformation;
    }

    namespace Tournament
    {
        extern int GameEndFrame;	
    }

    namespace Debug
    {
        extern bool DrawGameInfo;
        extern bool DrawUnitHealthBars;
        extern bool DrawProductionInfo;
        extern bool DrawQueueFixInfo;
        extern bool DrawScoutInfo;
        extern bool DrawWorkerInfo;
        extern bool DrawModuleTimers;
        extern bool DrawReservedBuildingTiles;
        extern bool DrawCombatSimulationInfo;
        extern bool DrawBuildingInfo;
        extern bool DrawEnemyUnitInfo;
        extern bool DrawUnitCounts;
        extern bool DrawHiddenEnemies;
        extern bool DrawMapInfo;
        extern bool DrawMapGrid;
        extern bool DrawMapDistances;
        extern bool DrawTerrainHeights;
        extern bool DrawBaseInfo;
        extern bool DrawExpoScores;
        extern bool DrawUnitTargets;
        extern bool DrawUnitOrders;
        extern bool DrawMicroState;
        extern bool DrawSquadInfo;
        extern bool DrawClusters;
        extern bool DrawDefenseClusters;
        extern bool DrawResourceAmounts;

        extern BWAPI::Color ColorLineTarget;
        extern BWAPI::Color ColorLineMineral;
        extern BWAPI::Color ColorUnitNearEnemy;
        extern BWAPI::Color ColorUnitNotNearEnemy;
    }

    namespace Micro
    {
        extern bool KiteWithRangedUnits;
        extern bool WorkersDefendRush;
        extern int RetreatMeleeUnitShields;
        extern int RetreatMeleeUnitHP;
        extern int CombatSimRadius;         
        extern int ScoutDefenseRadius;
    }
    
    namespace Macro
    {
        extern int WorkersPerRefinery;
        extern double WorkersPerPatch;
        extern int ProductionJamFrameLimit;
        extern bool ExpandToIslands;
    }

    namespace Tools
    {
        extern int MAP_GRID_SIZE;
    }
}