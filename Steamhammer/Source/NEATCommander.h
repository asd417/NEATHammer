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
		NETWORK_UNIT_COUNT
	};
	enum class NetworkUnits {
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

		Terran_Command_Center,
		Terran_Comsat_Station,
		Terran_Nuclear_Silo,
		Terran_Supply_Depot,
		Terran_Refinery,
		Terran_Barracks,
		Terran_Engineering_Bay,
		Terran_Missile_Turret,
		Terran_Academy,
		Terran_Factory,
		Terran_Machine_Shop,
		Terran_Starport,
		Terran_Control_Tower,
		Terran_Science_Facility,
		Terran_Physics_Lab,
		Terran_Covert_Ops,
		Terran_Armory,

		Zerg_Drone,
		Zerg_Zergling,
		Zerg_Overlord,
		Zerg_Hydralisk,
		Zerg_Lurker,
		Zerg_Mutalisk,
		Zerg_Guardian,
		Zerg_Devourer,
		Zerg_Queen,
		Zerg_Scourge,
		Zerg_Defiler,
		Zerg_Ultralisk,
		Zerg_Infested_Terran,

		Zerg_Hatchery,
		Zerg_Lair,
		Zerg_Hive,
		Zerg_Creep_Colony,
		Zerg_Sunken_Colony,
		Zerg_Spore_Colony,
		Zerg_Extractor,
		Zerg_Spawning_Pool,
		Zerg_Evolution_Chamber,
		Zerg_Hydralisk_Den,
		Zerg_Spire,
		Zerg_Greater_Spire,
		Zerg_Queens_Nest,
		Zerg_Ultralisk_Cavern,
		Zerg_Defiler_Mound,
		Zerg_Nydus_Canal,

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
		NETWORK_UNIT_COUNT
	};
	//Only include researchable techs	
	enum class NetworkTech {
		Stim_Packs,
		Optical_Flare,
		Restoration,
		Tank_Siege_Mode,
		Irradiate,
		EMP_Shockwave,
		Cloaking_Field,
		Personnel_Cloaking,
		Lockdown,
		Yamato_Gun,
		Nuclear_Strike,

		Burrowing,
		Spawn_Broodlings,
		Plague,
		Consume,
		Ensnare,
		Lurker_Aspect,

		Psionic_Storm,
		Hallucination,
		Recall,
		Stasis_Field,
		Disruption_Web,
		Mind_Control,
		Maelstrom,
		NETWORK_TECH_COUNT
	};

	enum class NetworkUpgrade {
		U_238_Shells,
		Terran_Infantry_Armor,
		Terran_Infantry_Weapons,
		Terran_Vehicle_Plating,
		Terran_Vehicle_Weapons,
		Ion_Thrusters,
		Titan_Reactor,
		Terran_Ship_Plating,
		Terran_Ship_Weapons,
		Moebius_Reactor,
		Ocular_Implants,
		Apollo_Reactor,
		Colossus_Reactor,
		Caduceus_Reactor,
		Charon_Boosters,

		Zerg_Carapace,
		Zerg_Flyer_Carapace,
		Zerg_Melee_Attacks,
		Zerg_Missile_Attacks,
		Zerg_Flyer_Attacks,
		Ventral_Sacs,
		Antennae,
		Pneumatized_Carapace,
		Metabolic_Boost,
		Adrenal_Glands,
		Muscular_Augments,
		Grooved_Spines,
		Gamete_Meiosis,
		Metasynaptic_Node,
		Chitinous_Plating,
		Anabolic_Synthesis,

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
		NETWORK_UPGRADE_COUNT
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
	private:
		NEATCommander();
		int mapWidth;
		int mapHeight;
		int lastMineral;
		int lastGas;

		//output vector
		double macroActType; //Unit, Tech, Upgrade, Command, Default
		double unitType;
		double techType;
		double upgradeType;
		double macroCommandType;
		double amount1;
		double amount2;
		double tilePosX; //0~256
		double tilePosY; //0~256
		double macroCommandUnitType;

		void InitializeNetwork();

		bool isWorkerType(BWAPI::UnitType type);
		int getWorkerCount(BWAPI::Unitset& allUnits);

		void evaluate();
		void getVisibleMap(int sectionNum);
		BWAPI::UnitType ToBWAPIUnitProtoss(NetworkProtossUnits ut);
		BWAPI::UnitType ToBWAPIUnit(NetworkUnits ut);
		BWAPI::TechType ToBWAPITech(NetworkTech tt);
		BWAPI::UpgradeType ToBWAPIUpgrade(NetworkUpgrade ut);
		NEAT_TileType getTileType(BWAPI::UnitType type);
		std::vector<MacroAct> _actions{};

		int frame = 0;
		int maxSections;
		int curSection = 0;
		std::vector<std::array<int, 2>> sectionsCoords{};
		std::array<std::array<int, 16>, 16> enemyMapData{};
		std::array<std::array<int, 16>, 16> friendlyMapData{};

		FeedForwardNetwork* network;
		std::vector<double> inputVector{};

		int genomeID = 0;
		double fitness = 0.0f;
	};

}