#pragma once
#include "BuildOrder.h"
#include "db.h"
#include "NodeEval.h"
#include "time.h"
#include <vector>
#include <array>

#include <random>
#include "TimerManager.h"
namespace UAlbertaBot
{
	enum class NetworkTerranOptions {
		//units
		Terran_SCV,
		Terran_Marine,
		Terran_Firebat,
		Terran_Medic,
		Terran_Ghost,
		Terran_Vulture, 
		Terran_Siege_Tank_Tank_Mode, 
		Terran_Goliath,
		Terran_Wraith, 
		Terran_Dropship, 
		Terran_Science_Vessel, 
		Terran_Valkyrie, 
		Terran_Battlecruiser,
		//buildings
		Terran_Command_Center, 
		Terran_Comsat_Station, 
		Terran_Nuclear_Silo, 
		Terran_Supply_Depot,
		Terran_Barracks, 
		Terran_Refinery, 
		Terran_Engineering_Bay, 
		Terran_Bunker, 
		Terran_Missile_Turret,
		Terran_Academy, 
		Terran_Factory,
		Terran_Armory,
		Terran_Machine_Shop, 
		Terran_Starport, 
		Terran_Control_Tower, 
		Terran_Science_Facility, 
		Terran_Covert_Ops,
		Terran_Physics_Lab, 
		//Special units and techs that is handled in the macro command
		//Terran_Siege_Tank_Siege_Mode, 
		//Terran_Vulture_Spider_Mine,
		//Nuclear_Strike,
		//Tech types
		Cloaking_Field, 
		Defensive_Matrix,
		EMP_Shockwave,
		Lockdown,
		Optical_Flare,
		Personnel_Cloaking,
		Spider_Mines,
		Stim_Packs,
		Tank_Siege_Mode,
		Yamato_Gun, 
		//Upgrade types
		Apollo_Reactor,
		Caduceus_Reactor,
		Charon_Boosters, 
		Colossus_Reactor,
		Ion_Thrusters,
		Moebius_Reactor, 
		Ocular_Implants, 
		Terran_Infantry_Armor, 
		Terran_Infantry_Weapons, 
		Terran_Ship_Plating,
		Terran_Ship_Weapons, 
		Terran_Vehicle_Plating,
		Terran_Vehicle_Weapons, 
		Titan_Reactor,
		U_238_Shells,
		NETWORK_OPTION_COUNT
	};

	enum class NetworkProtossOptions {
		//All Units
		Protoss_Probe,
		Protoss_Zealot,
		Protoss_Dragoon,
		Protoss_High_Templar,
		Protoss_Archon,
		Protoss_Dark_Templar,
		Protoss_Dark_Archon,
		Protoss_Reaver,
		Protoss_Shuttle,
		Protoss_Observer,
		Protoss_Scout,
		Protoss_Corsair,
		Protoss_Arbiter,
		Protoss_Carrier,
		//All Buildings
		Protoss_Nexus,
		Protoss_Pylon,
		Protoss_Assimilator,
		Protoss_Gateway,
		Protoss_Forge,
		Protoss_Photon_Cannon,
		Protoss_Shield_Battery,
		Protoss_Cybernetics_Core,
		Protoss_Citadel_of_Adun,
		Protoss_Templar_Archives,
		Protoss_Robotics_Facility,
		Protoss_Robotics_Support_Bay,
		Protoss_Observatory,
		Protoss_Stargate,
		Protoss_Fleet_Beacon,
		Protoss_Arbiter_Tribunal,
		//All Researchable tech
		Psionic_Storm,
		Hallucination,
		Recall,
		Stasis_Field,
		Disruption_Web,
		Mind_Control,
		Maelstrom,
		//All upgrades
		Apial_Sensors,
		Protoss_Ground_Armor,
		Protoss_Air_Armor,
		Protoss_Ground_Weapons,
		Protoss_Air_Weapons,
		Protoss_Plasma_Shields,
		Singularity_Charge,
		Leg_Enhancements,
		Scarab_Damage,
		Reaver_Capacity,
		Gravitic_Drive,
		Sensor_Array,
		Gravitic_Boosters,
		Khaydarin_Amulet,
		Gravitic_Thrusters,
		Carrier_Capacity,
		Khaydarin_Core,
		Argus_Jewel,
		Argus_Talisman,

		NETWORK_OPTION_COUNT
	};

	//Complex Tile Type Enum
	//Entries with higher number will override the data if the tile has multiple types of units
	enum NEAT_TileType {
		FOG = 0,
		WALKABLE,
		NOTWALKABLE,
		//Mineral and Gas
		GAS,
		MINERAL,
		//Buildings
		Terran_Command_Center,
		Terran_Nuclear_Silo,
		Terran_Comsat_Station,
		Terran_Supply_Depot,
		Terran_Barracks,
		Terran_Refinery,
		Terran_Engineering_Bay,
		Terran_Missile_Turret,
		Terran_Bunker,
		Terran_Academy,
		Terran_Factory,
		Terran_Machine_Shop,
		Terran_Armory,
		Terran_Starport,
		Terran_Control_Tower,
		Terran_Science_Facility,
		Terran_Physics_Lab,
		Terran_Covert_Ops,
		Protoss_Nexus,
		Protoss_Pylon,
		Protoss_Assimilator,
		Protoss_Gateway,
		Protoss_Cybernetics_Core,
		Protoss_Forge,
		Protoss_Photon_Cannon,
		Protoss_Shield_Battery,
		Protoss_Citadel_of_Adun,
		Protoss_Templar_Archives,
		Protoss_Robotics_Facility,
		Protoss_Robotics_Support_Bay,
		Protoss_Observatory,
		Protoss_Stargate,
		Protoss_Arbiter_Tribunal,
		Protoss_Fleet_Beacon,
		Zerg_Hatchery,
		Zerg_Creep_Colony,
		Zerg_Sunken_Colony,
		Zerg_Spore_Colony,
		Zerg_Extractor,
		Zerg_Spawning_Pool,
		Zerg_Evolution_Chamber,
		Zerg_Lair,
		Zerg_Queens_Nest,
		Zerg_Nydus_Canal,
		Zerg_Spire,
		Zerg_Greater_Spire,
		Zerg_Hive,
		Zerg_Ultralisk_Cavern,
		Zerg_Hydralisk_Den,
		Zerg_Defiler_Mound,
		Zerg_Infested_Command_Center,


		Terran_SCV,
		Terran_Marine,
		Terran_Medic,
		Terran_Firebat,
		Terran_Ghost,
		Terran_Vulture,
		Terran_Vulture_Spider_Mine,
		Terran_Siege_Tank_Tank_Mode,
		Terran_Siege_Tank_Siege_Mode,
		Terran_Goliath,
		Terran_Wraith,
		Terran_Valkyrie,
		Terran_Science_Vessel,
		Terran_Dropship,
		Terran_Battlecruiser,
		Terran_Nuclear_Missile,
		Protoss_Probe,
		Protoss_Zealot,
		Protoss_Dragoon,
		Protoss_High_Templar,
		Protoss_Dark_Templar,
		Protoss_Archon,
		Protoss_Dark_Archon,
		Protoss_Reaver,
		Protoss_Scarab,
		Protoss_Observer,
		Protoss_Shuttle,
		Protoss_Scout,
		Protoss_Corsair,
		Protoss_Arbiter,
		Protoss_Interceptor,
		Protoss_Carrier,
		Zerg_Drone,
		Zerg_Zergling,
		Zerg_Overlord,
		Zerg_Hydralisk,
		Zerg_Lurker_Egg,
		Zerg_Lurker,
		Zerg_Infested_Terran,
		Zerg_Queen,
		Zerg_Mutalisk,
		Zerg_Guardian,
		Zerg_Devourer,
		Zerg_Scourge,
		Zerg_Broodling,
		Zerg_Ultralisk,
		Zerg_Defiler,
		MAX
	};
	//Simple Tile Type Enum
	enum NEAT_TileType_Simple {
		sFOG = 0,
		sNOTWALKABLE,
		sWALKABLE,
		sMINERAL,
		sGAS,
		sUNIT,
		sMAX
	};
	class NEATCommander {
		enum NetworkType {
			FEEDFORWARD,
			RECURRENT
		};
	public:
		static NEATCommander& Instance();
		void update();
		BuildOrder getMacroCommands();
		void resetActions();
		void incrementFrame();

		double getFitness();
		void scoreFitness(double add);
		void sendFitnessToTrainServer();

		//Fitness Functions
		void onUnitCreate(BWAPI::Unit unit);
		void onUnitComplete(BWAPI::Unit unit);
		void onUnitDestroy(BWAPI::Unit unit);
		void onUnitHide(BWAPI::Unit unit);
		void onUnitShow(BWAPI::Unit unit);

		void onEnd(bool isWinner);

		void drawDebug(int x, int y);
		void setTimeManager(TimerManager* t); 
	private:
		NEATCommander();

		void InitializeNetwork();
		int getWorkerCount(BWAPI::Unitset& allUnits);

		void evaluate();
		void getVisibleMap(int sectionNum);
		void getVisibleMapSimple(int sectionNum);
		bool canMacro(MacroCommandType command);
		bool canBuild(NetworkTerranOptions option, BWAPI::TilePosition& location, int mineral, int gas);
		bool canBuild(NetworkProtossOptions option, BWAPI::TilePosition& location);

		BWAPI::TilePosition getClosestProtossBuildPosition(BWAPI::Position closestTo, BWAPI::UnitType buildingType) const;

		BWAPI::UnitType ToBWAPIUnit(NetworkTerranOptions ut);
		BWAPI::TechType ToBWAPITech(NetworkTerranOptions tt);
		BWAPI::UpgradeType ToBWAPIUpgrade(NetworkTerranOptions tt);

		BWAPI::UnitType ToBWAPIUnit(NetworkProtossOptions ut);
		BWAPI::TechType ToBWAPITech(NetworkProtossOptions tt);
		BWAPI::UpgradeType ToBWAPIUpgrade(NetworkProtossOptions ut);

		NEAT_TileType getTileType(BWAPI::UnitType type);

		//Input Vector
		int mapWidth;
		int mapHeight;
		int lastMineral;
		int lastGas;
		int frame = 0;
		int maxSections;
		int curSection = 0;
		std::vector<std::array<int, 2>> sectionsCoords{};
		std::array<std::array<double, 16>, 16> enemyMapData{};
		std::array<std::array<double, 16>, 16> enemyMapBuildingData{};
		std::array<std::array<double, 16>, 16> friendlyMapData{};
		std::array<std::array<double, 16>, 16> friendlyMapBuildingData{};
		std::vector<double> inputVector{};

		//Output Vector
		std::array<double, (size_t)NetworkProtossOptions::NETWORK_OPTION_COUNT> builderOutputs;
		std::array<double, (size_t)MacroCommandType::QueueBarrier> macroCommandTypeOutputs;
		double tilePosX; //0~256
		double tilePosY; //0~256
		
		bool _winner;

		Network* network;
		int genomeID = 0;
		double fitness = 0.0f;
		bool initialized = false;
		int _lastUpdateFrame = 0;
		std::vector<MacroAct> _actions{};
		int networkType = NetworkType::FEEDFORWARD;

		//Kill tracker
		std::map<int, int> killMap;
		
		//Misc

		/// <summary>
		/// NOT OWNED BY NEATCOMMANDER DO NOT DELETE
		/// </summary>
		TimerManager* timer = nullptr; 
		//Random number generator
		std::random_device rd;
		std::mt19937 gen;
		std::uniform_real_distribution<double> dis;
	};

};