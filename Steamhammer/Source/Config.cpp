#include "Config.h"
#include "UABAssert.h"

// Most values here are default values that apply if the configuration entry
// is missing from the config file, or is invalid.

// The ConfigFile record tells where to find the config file, so it's different.

namespace Config
{
    namespace ConfigFile
    {
        bool ConfigFileFound                = false;
        bool ConfigFileParsed               = false;
        std::string ConfigFileLocation      = "bwapi-data/AI/NEATHammer.json";
    }

    namespace IO
    {
        std::string ErrorLogFilename		= "Steamhammer_ErrorLog.txt";
        bool LogAssertToErrorFile			= false;
        std::string StaticDir               = "bwapi-data/AI/";
        std::string PreparedDataDir         = "bwapi-data/AI/om/";
        std::string ReadDir                 = "bwapi-data/read/";
        std::string WriteDir				= "bwapi-data/write/";
        std::string OpeningTimingFile       = "timings.txt";
        int MaxGameRecords					= 0;
    }

    namespace Skills
    {
        bool UnderSCHNAIL                   = false;
        bool SCHNAILMeansHuman              = true;
        bool HumanOpponent                  = false;
        bool SurrenderWhenHopeIsLost        = true;

        bool ScoutHarassEnemy               = false;
    }

    namespace NEAT
    {
        bool Train = true;
        //Connection Settings
        std::string TrainingServerIP = "http://127.0.0.1:5000";
        int RetryTimer = 30;
        
        bool LoadNetworkFromJSON = false;
        std::string NetworkJSON = "";

        int ArmyKillScore = 100; // awared for kills made by non-worker units and buildings (to avoid bot learning to kill zerglings with scv)
        int AlliedUnitDeathPenalty = 1; // to encourage smart macro decision. penalty when any non-building unit is killed
        int WinScore = 100; 
        int BuildingScore = 10; // awarded for every building built
        int UnitCompleteScore = 1000; // awared for every army unit created

        bool AutoSurrender = true;

        bool PrintNetworkOutput = false;

        bool LogInputVector = false;
        std::string InputLogFileName = "Log_NEAT_Input.txt";
        bool LogOutputVector = false;
        std::string OutputLogFileName = "Log_NEAT_Output.txt";
        bool LogOutputDecision = false;
        std::string DecisionLogFileName = "Log_NEAT_Reasoning.txt";
    }

    namespace BotInfo
    {
        std::string BotName                 = "NEATHammer";
        std::string Authors                 = "Yeonghun Lee";
        bool PrintInfoOnStart               = true;
    }

    namespace BWAPIOptions
    {
        int SetLocalSpeed                   = 0;
        int SetFrameSkip                    = 100;
        bool EnableUserInput                = true;
        bool EnableCompleteMapInformation   = false;
    }

    namespace Tournament
    {
        int GameEndFrame                    = 86400;
    }

    namespace Debug
    {
        bool DrawGameInfo                   = true;
        bool DrawUnitHealthBars             = false;
        bool DrawProductionInfo             = false;
        bool DrawQueueFixInfo				= false;
        bool DrawScoutInfo                  = false;
        bool DrawWorkerInfo                 = false;
        bool DrawModuleTimers               = false;
        bool DrawReservedBuildingTiles      = false;
        bool DrawCombatSimulationInfo       = false;
        bool DrawBuildingInfo               = false;
        bool DrawEnemyUnitInfo              = false;
        bool DrawUnitCounts                 = false;
        bool DrawHiddenEnemies				= false;
        bool DrawMapInfo					= false;
        bool DrawMapGrid                    = false;
        bool DrawMapDistances				= false;
        bool DrawTerrainHeights             = false;
        bool DrawBaseInfo					= false;
        bool DrawExpoScores					= false;
        bool DrawUnitTargets				= false;
        bool DrawUnitOrders					= false;
        bool DrawMicroState					= false;
        bool DrawSquadInfo                  = false;
        bool DrawClusters					= false;
        bool DrawDefenseClusters			= false;
        bool DrawResourceAmounts            = false;

        BWAPI::Color ColorLineTarget        = BWAPI::Colors::White;
        BWAPI::Color ColorLineMineral       = BWAPI::Colors::Cyan;
        BWAPI::Color ColorUnitNearEnemy     = BWAPI::Colors::Red;
        BWAPI::Color ColorUnitNotNearEnemy  = BWAPI::Colors::Green;
    }

    namespace Micro
    {
        bool KiteWithRangedUnits            = true;
        bool WorkersDefendRush              = true;
        int RetreatMeleeUnitShields         = 0;
        int RetreatMeleeUnitHP              = 0;
        int CombatSimRadius					= 300;      // radius of units around frontmost unit for combat sim
        int ScoutDefenseRadius				= 600;		// radius to chase enemy scout worker
    }

    namespace Macro
    {
        int ProductionJamFrameLimit			= 33;
        int WorkersPerRefinery              = 3;
        double WorkersPerPatch              = 3.0;
        bool ExpandToIslands				= false;
    }

    namespace Tools
    {
        extern int MAP_GRID_SIZE            = 320;      // size of grid spacing in MapGrid
    }
}
