#pragma once
#include "BuildOrder.h"
#include "db.h"
#include "NodeEval.h"
#include "time.h"
#include <vector>
#include <array>
namespace UAlbertaBot
{
	enum class NetworkProtossUnits {
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
		NETWORK_UNIT_COUNT
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
		Apial_Sensors,
		Gravitic_Thrusters,
		Carrier_Capacity,
		Khaydarin_Core,
		Argus_Jewel,
		Argus_Talisman,

		NETWORK_OPTION_COUNT
	};

	//Entries with higher number will override the data if the tile has multiple types of units
	enum NEAT_TileType {
		FOG = 0,
		WALKABLE,
		NOTWALKABLE,
		//Mineral and Gas
		Resource_Vespene_Geyser = 10,
		Powerup_Mineral,
		//Buildings sorted by importance
		Terran_Supply_Depot = 30,
		Protoss_Pylon,
		Protoss_Assimilator,
		Zerg_Extractor,
		Terran_Refinery,
		Protoss_Nexus,
		Terran_Command_Center,
		Terran_Comsat_Station,
		Zerg_Hatchery,
		Terran_Barracks,
		Protoss_Gateway,
		Terran_Academy,
		Protoss_Cybernetics_Core,
		Protoss_Forge,
		Terran_Engineering_Bay,
		Zerg_Evolution_Chamber,
		Terran_Factory,
		Terran_Armory,
		Terran_Machine_Shop,
		Zerg_Queens_Nest,
		Terran_Starport,
		Zerg_Lair,
		Protoss_Citadel_of_Adun,
		Protoss_Observatory,
		Zerg_Spire,
		Protoss_Robotics_Support_Bay,
		Protoss_Arbiter_Tribunal,
		Zerg_Greater_Spire,
		Protoss_Templar_Archives,
		Protoss_Robotics_Facility,
		Terran_Control_Tower,
		Protoss_Fleet_Beacon,
		Protoss_Stargate,
		Terran_Science_Facility,
		Terran_Physics_Lab,
		Terran_Covert_Ops,
		Zerg_Hive,
		Zerg_Ultralisk_Cavern,
		Zerg_Spawning_Pool,
		Terran_Nuclear_Silo,
		Zerg_Hydralisk_Den,
		Zerg_Defiler_Mound,

		//Buildings that directly affect combat. More important than any other buildings
		Zerg_Creep_Colony = 120,
		Protoss_Shield_Battery,
		Zerg_Nydus_Canal,
		Zerg_Infested_Command_Center,
		Zerg_Spore_Colony,
		Protoss_Photon_Cannon,
		Zerg_Sunken_Colony,
		Terran_Bunker,
		Terran_Missile_Turret,

		//Units sorted by strength
		Terran_SCV = 150,
		Protoss_Probe,
		Zerg_Drone,
		Protoss_Interceptor,
		Protoss_Scarab,
		Zerg_Broodling,
		Terran_Marine,
		Zerg_Zergling,
		Terran_Vulture,
		Terran_Vulture_Spider_Mine,
		Zerg_Overlord,
		Terran_Medic,
		Terran_Firebat,
		Protoss_Zealot,
		Protoss_Dragoon,
		Protoss_Observer,
		Terran_Ghost,
		Protoss_Scout,
		Zerg_Hydralisk,
		Terran_Siege_Tank_Tank_Mode,
		Terran_Siege_Tank_Siege_Mode,
		Zerg_Lurker_Egg,
		Zerg_Lurker,
		Protoss_Archon,
		Protoss_High_Templar,
		Zerg_Infested_Terran,
		Zerg_Queen,
		Terran_Valkyrie,
		Zerg_Devourer,
		Protoss_Corsair,
		Zerg_Scourge,
		Protoss_Reaver,
		Zerg_Ultralisk,
		Protoss_Dark_Templar,
		Protoss_Dark_Archon,
		Terran_Wraith,
		Terran_Goliath,
		Protoss_Shuttle,
		Terran_Dropship,
		Zerg_Mutalisk,
		Zerg_Guardian,
		Protoss_Arbiter,
		Zerg_Defiler,
		Terran_Science_Vessel,
		Protoss_Carrier,
		Terran_Battlecruiser,
		Terran_Nuclear_Missile, //This is the only thing that the bot will see when the nuke is fired (if fired within visible range?)
	};

	enum NEAT_TileType_Simple {
		sFOG = 0,
		sWALKABLE,
		sNOTWALKABLE,
		sMINERAL,
		sGAS,
		sUNIT=10
	};
	class NEATCommander {
	public:
		static NEATCommander& Instance();
		void update();
		BuildOrder getMacroCommands();
		void resetActions();
		void incrementFrame();

		double getFitness();
		void scoreFitness(double add);
		void sendFitnessToTrainServer();

		//Callbacks
		void onUnitCreate(BWAPI::Unit unit);
		void onUnitComplete(BWAPI::Unit unit);
		void onUnitDestroy(BWAPI::Unit unit);

		//When visible unit becomes invisible
		void onUnitHide(BWAPI::Unit unit);

		//When invisible unit becomes visible
		void onUnitShow(BWAPI::Unit unit);
	private:
		NEATCommander();
		int mapWidth;
		int mapHeight;
		int lastMineral;
		int lastGas;

		//output vector
		std::array<double, (size_t)NetworkProtossOptions::NETWORK_OPTION_COUNT> builderOutputs;
		std::array<double, (size_t)MacroCommandType::QueueBarrier> macroCommandTypeOutputs;
		double tilePosX; //0~256
		double tilePosY; //0~256

		void InitializeNetwork();

		bool isWorkerType(BWAPI::UnitType type);
		int getWorkerCount(BWAPI::Unitset& allUnits);
		BWAPI::Unit NEATCommander::getProducer(NetworkProtossOptions option) const;

		void evaluate();
		void getVisibleMap(int sectionNum);
		void getVisibleMapSimple(int sectionNum);
		bool canBuild(NetworkProtossOptions option, BWAPI::TilePosition& location);
		bool canBuild(NetworkProtossUnits option);
		BWAPI::TilePosition getClosestProtossBuildPosition(BWAPI::Position closestTo, BWAPI::UnitType buildingType) const;
		BWAPI::UnitType ToBWAPIUnit(NetworkProtossOptions ut);
		BWAPI::UnitType ToBWAPIUnit(NetworkProtossUnits ut);
		BWAPI::TechType ToBWAPITech(NetworkProtossOptions tt);
		BWAPI::UpgradeType ToBWAPIUpgrade(NetworkProtossOptions ut);
		NEAT_TileType getTileType(BWAPI::UnitType type);
		std::vector<MacroAct> _actions{};
		bool initialized = false;
		int frame = 0;
		int maxSections;
		int curSection = 0;
		std::vector<std::array<int, 2>> sectionsCoords{};
		std::array<std::array<int, 16>, 16> enemyMapData{};
		std::array<std::array<int, 16>, 16> enemyMapBuildingData{};
		std::array<std::array<int, 16>, 16> friendlyMapData{};
		std::array<std::array<int, 16>, 16> friendlyMapBuildingData{};

		FeedForwardNetwork* network;
		std::vector<double> inputVector{};
		

		int genomeID = 0;
		double fitness = 0.0f;
	};

}