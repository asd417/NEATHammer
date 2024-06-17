#pragma once
#include "BuildOrder.h"
#include <vector>
namespace UAlbertaBot
{
	class NEATCommander {
	public:
		static NEATCommander& Instance();
		void update();
		BuildOrder& getMacroCommands();
		void resetActions();
	private:
		void evaluate();
		std::vector<MacroAct> _actions;
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
		Protoss_Carrier
	};
	enum class NetworkBuildings {
		Terran_Command_Center,
		Terran_Comsat_Station,
		Terran_Nuclear_Silo,
		Terran_Supply_Depot,
		Terran_Refinery,
		Terran_Barracks,
		Terran_Supply_Depot,
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
		Terran_Armory
	};
}