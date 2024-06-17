#include "NEATCommander.h"
#include "MacroAct.h"

namespace UAlbertaBot
{
    NEATCommander& NEATCommander::Instance()
    {
        static NEATCommander instance;
        return instance;
    }
    BuildOrder& NEATCommander::getMacroCommands()
    {
        return BuildOrder(BWAPI::Broodwar->self()->getRace(), _actions);
    }
    void NEATCommander::resetActions()
    {
        _actions.clear();
    }
    void NEATCommander::evaluate()
    {
        //MacroAct._parent is not necessary
        //MacroAct._macroLocation is used to automatically calculate macro location
        // it should instead be done by the network
        int macroActType; //Unit, Tech, Upgrade, Command, Default
        int unitType; 
        int techType;
        int upgradeType;
        int macroCommandType;
        int amount1;
        int amount2;
        int tilePosX; //0~256
        int tilePosY; //0~256
        int macroCommandUnitType;
        NetworkUnits ut = (NetworkUnits)macroCommandUnitType;

        if (macroActType == MacroActs::Command) {
            MacroCommand mc = MacroCommand((MacroCommandType)macroCommandType, amount1,amount2, ToBWAPIUnit(ut));
            //BWAPI::TilePosition tp = ;
            MacroAct ma = MacroAct(mc, { tilePosX , tilePosY });
        }
        //MacroAct action{};
        //_actions.push_back();
        
    }

    BWAPI::UnitType ToBWAPIUnit(NetworkUnits ut) {
        switch (ut) {
        case NetworkUnits::Terran_SCV:
            return BWAPI::UnitTypes::Terran_SCV;
        case NetworkUnits::Terran_Marine:
            return BWAPI::UnitTypes::Terran_Marine;
        case NetworkUnits::Terran_Firebat:
            return BWAPI::UnitTypes::Terran_Firebat;
        case NetworkUnits::Terran_Medic:
            return BWAPI::UnitTypes::Terran_Medic;
        case NetworkUnits::Terran_Ghost:
            return BWAPI::UnitTypes::Terran_Ghost;
        case NetworkUnits::Terran_Vulture:
            return BWAPI::UnitTypes::Terran_Vulture;
        case NetworkUnits::Terran_Siege_Tank_Tank_Mode:
            return BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode;
        case NetworkUnits::Terran_Goliath:
            return BWAPI::UnitTypes::Terran_Goliath;
        case NetworkUnits::Terran_Wraith:
            return BWAPI::UnitTypes::Terran_Wraith;
        case NetworkUnits::Terran_Dropship:
            return BWAPI::UnitTypes::Terran_Dropship;
        case NetworkUnits::Terran_Science_Vessel:
            return BWAPI::UnitTypes::Terran_Science_Vessel;
        case NetworkUnits::Terran_Valkyrie:
            return BWAPI::UnitTypes::Terran_Valkyrie;
        case NetworkUnits::Terran_Battlecruiser:
            return BWAPI::UnitTypes::Terran_Battlecruiser;
        case NetworkUnits::Terran_Command_Center:
            return BWAPI::UnitTypes::Terran_Command_Center;
        case NetworkUnits::Terran_Comsat_Station:
            return BWAPI::UnitTypes::Terran_Comsat_Station;
        case NetworkUnits::Terran_Nuclear_Silo:
            return BWAPI::UnitTypes::Terran_Nuclear_Silo;
        case NetworkUnits::Terran_Supply_Depot:
            return BWAPI::UnitTypes::Terran_Supply_Depot;
        case NetworkUnits::Terran_Refinery:
            return BWAPI::UnitTypes::Terran_Refinery;
        case NetworkUnits::Terran_Barracks:
            return BWAPI::UnitTypes::Terran_Barracks;
        case NetworkUnits::Terran_Engineering_Bay:
            return BWAPI::UnitTypes::Terran_Engineering_Bay;
        case NetworkUnits::Terran_Missile_Turret:
            return BWAPI::UnitTypes::Terran_Missile_Turret;
        case NetworkUnits::Terran_Academy:
            return BWAPI::UnitTypes::Terran_Academy;
        case NetworkUnits::Terran_Factory:
            return BWAPI::UnitTypes::Terran_Factory;
        case NetworkUnits::Terran_Machine_Shop:
            return BWAPI::UnitTypes::Terran_Machine_Shop;
        case NetworkUnits::Terran_Starport:
            return BWAPI::UnitTypes::Terran_Starport;
        case NetworkUnits::Terran_Control_Tower:
            return BWAPI::UnitTypes::Terran_Control_Tower;
        case NetworkUnits::Terran_Science_Facility:
            return BWAPI::UnitTypes::Terran_Science_Facility;
        case NetworkUnits::Terran_Physics_Lab:
            return BWAPI::UnitTypes::Terran_Physics_Lab;
        case NetworkUnits::Terran_Covert_Ops:
            return BWAPI::UnitTypes::Terran_Covert_Ops;
        case NetworkUnits::Terran_Armory:
            return BWAPI::UnitTypes::Terran_Armory;
        case NetworkUnits::Zerg_Drone:
            return BWAPI::UnitTypes::Zerg_Drone;
        case NetworkUnits::Zerg_Zergling:
            return BWAPI::UnitTypes::Zerg_Zergling;
        case NetworkUnits::Zerg_Overlord:
            return BWAPI::UnitTypes::Zerg_Overlord;
        case NetworkUnits::Zerg_Hydralisk:
            return BWAPI::UnitTypes::Zerg_Hydralisk;
        case NetworkUnits::Zerg_Lurker:
            return BWAPI::UnitTypes::Zerg_Lurker;
        case NetworkUnits::Zerg_Mutalisk:
            return BWAPI::UnitTypes::Zerg_Mutalisk;
        case NetworkUnits::Zerg_Guardian:
            return BWAPI::UnitTypes::Zerg_Guardian;
        case NetworkUnits::Zerg_Devourer:
            return BWAPI::UnitTypes::Zerg_Devourer;
        case NetworkUnits::Zerg_Queen:
            return BWAPI::UnitTypes::Zerg_Queen;
        case NetworkUnits::Zerg_Scourge:
            return BWAPI::UnitTypes::Zerg_Scourge;
        case NetworkUnits::Zerg_Defiler:
            return BWAPI::UnitTypes::Zerg_Defiler;
        case NetworkUnits::Zerg_Ultralisk:
            return BWAPI::UnitTypes::Zerg_Ultralisk;
        case NetworkUnits::Zerg_Infested_Terran:
            return BWAPI::UnitTypes::Zerg_Infested_Terran;
        case NetworkUnits::Zerg_Hatchery:
            return BWAPI::UnitTypes::Zerg_Hatchery;
        case NetworkUnits::Zerg_Lair:
            return BWAPI::UnitTypes::Zerg_Lair;
        case NetworkUnits::Zerg_Hive:
            return BWAPI::UnitTypes::Zerg_Hive;
        case NetworkUnits::Zerg_Creep_Colony:
            return BWAPI::UnitTypes::Zerg_Creep_Colony;
        case NetworkUnits::Zerg_Sunken_Colony:
            return BWAPI::UnitTypes::Zerg_Sunken_Colony;
        case NetworkUnits::Zerg_Extractor:
            return BWAPI::UnitTypes::Zerg_Extractor;
        case NetworkUnits::Zerg_Spawning_Pool:
            return BWAPI::UnitTypes::Zerg_Spawning_Pool;
        case NetworkUnits::Zerg_Evolution_Chamber:
            return BWAPI::UnitTypes::Zerg_Evolution_Chamber;
        case NetworkUnits::Zerg_Hydralisk_Den:
            return BWAPI::UnitTypes::Zerg_Hydralisk_Den;
        case NetworkUnits::Zerg_Spire:
            return BWAPI::UnitTypes::Zerg_Spire;
        case NetworkUnits::Zerg_Greater_Spire:
            return BWAPI::UnitTypes::Zerg_Greater_Spire;
        case NetworkUnits::Zerg_Queens_Nest:
            return BWAPI::UnitTypes::Zerg_Queens_Nest;
        case NetworkUnits::Zerg_Ultralisk_Cavern:
            return BWAPI::UnitTypes::Zerg_Ultralisk_Cavern;
        case NetworkUnits::Zerg_Defiler_Mound:
            return BWAPI::UnitTypes::Zerg_Defiler_Mound;
        case NetworkUnits::Zerg_Nydus_Canal:
            return BWAPI::UnitTypes::Zerg_Nydus_Canal;
        case NetworkUnits::Protoss_Probe:
            return BWAPI::UnitTypes::Protoss_Probe;
        case NetworkUnits::Protoss_Zealot:
            return BWAPI::UnitTypes::Protoss_Zealot;
        case NetworkUnits::Protoss_Dragoon:
            return BWAPI::UnitTypes::Protoss_Dragoon;
        case NetworkUnits::Protoss_High_Templar:
            return BWAPI::UnitTypes::Protoss_High_Templar;
        case NetworkUnits::Protoss_Archon:
            return BWAPI::UnitTypes::Protoss_Archon;
        case NetworkUnits::Protoss_Dark_Templar:
            return BWAPI::UnitTypes::Protoss_Dark_Templar;
        case NetworkUnits::Protoss_Dark_Archon:
            return BWAPI::UnitTypes::Protoss_Dark_Archon;
        case NetworkUnits::Protoss_Reaver:
            return BWAPI::UnitTypes::Protoss_Reaver;
        case NetworkUnits::Protoss_Shuttle:
            return BWAPI::UnitTypes::Protoss_Shuttle;
        case NetworkUnits::Protoss_Observer:
            return BWAPI::UnitTypes::Protoss_Observer;
        case NetworkUnits::Protoss_Scout:
            return BWAPI::UnitTypes::Protoss_Scout;
        case NetworkUnits::Protoss_Corsair:
            return BWAPI::UnitTypes::Protoss_Corsair;
        case NetworkUnits::Protoss_Arbiter:
            return BWAPI::UnitTypes::Protoss_Arbiter;
        case NetworkUnits::Protoss_Carrier:
            return BWAPI::UnitTypes::Protoss_Carrier;
        case NetworkUnits::Protoss_Nexus:
            return BWAPI::UnitTypes::Protoss_Nexus;
        case NetworkUnits::Protoss_Pylon:
            return BWAPI::UnitTypes::Protoss_Pylon;
        case NetworkUnits::Protoss_Assimilator:
            return BWAPI::UnitTypes::Protoss_Assimilator;
        case NetworkUnits::Protoss_Gateway:
            return BWAPI::UnitTypes::Protoss_Gateway;
        case NetworkUnits::Protoss_Forge:
            return BWAPI::UnitTypes::Protoss_Forge;
        case NetworkUnits::Protoss_Photon_Cannon:
            return BWAPI::UnitTypes::Protoss_Photon_Cannon;
        case NetworkUnits::Protoss_Shield_Battery:
            return BWAPI::UnitTypes::Protoss_Shield_Battery;
        case NetworkUnits::Protoss_Cybernetics_Core:
            return BWAPI::UnitTypes::Protoss_Cybernetics_Core;
        case NetworkUnits::Protoss_Citadel_of_Adun:
            return BWAPI::UnitTypes::Protoss_Citadel_of_Adun;
        case NetworkUnits::Protoss_Templar_Archives:
            return BWAPI::UnitTypes::Protoss_Templar_Archives;
        case NetworkUnits::Protoss_Robotics_Facility:
            return BWAPI::UnitTypes::Protoss_Robotics_Facility;
        case NetworkUnits::Protoss_Robotics_Support_Bay:
            return BWAPI::UnitTypes::Protoss_Robotics_Support_Bay;
        case NetworkUnits::Protoss_Observatory:
            return BWAPI::UnitTypes::Protoss_Observatory;
        case NetworkUnits::Protoss_Stargate:
            return BWAPI::UnitTypes::Protoss_Stargate;
        case NetworkUnits::Protoss_Fleet_Beacon:
            return BWAPI::UnitTypes::Protoss_Fleet_Beacon;
        case NetworkUnits::Protoss_Arbiter_Tribunal:
            return BWAPI::UnitTypes::Protoss_Arbiter_Tribunal;
        default:
            throw std::invalid_argument("Unknown NetworkUnit");
        }
    }

    BWAPI::TechType ToBWAPITech(NetworkTech tt) {
        switch (tt) {
        case NetworkTech::Stim_Packs:
            return BWAPI::TechTypes::Stim_Packs;
        case NetworkTech::Optical_Flare:
            return BWAPI::TechTypes::Optical_Flare;
        case NetworkTech::Restoration:
            return BWAPI::TechTypes::Restoration;
        case NetworkTech::Tank_Siege_Mode:
            return BWAPI::TechTypes::Tank_Siege_Mode;
        case NetworkTech::Irradiate:
            return BWAPI::TechTypes::Irradiate;
        case NetworkTech::EMP_Shockwave:
            return BWAPI::TechTypes::EMP_Shockwave;
        case NetworkTech::Cloaking_Field:
            return BWAPI::TechTypes::Cloaking_Field;
        case NetworkTech::Personnel_Cloaking:
            return BWAPI::TechTypes::Personnel_Cloaking;
        case NetworkTech::Lockdown:
            return BWAPI::TechTypes::Lockdown;
        case NetworkTech::Yamato_Gun:
            return BWAPI::TechTypes::Yamato_Gun;
        case NetworkTech::Nuclear_Strike:
            return BWAPI::TechTypes::Nuclear_Strike;
        case NetworkTech::Burrowing:
            return BWAPI::TechTypes::Burrowing;
        case NetworkTech::Spawn_Broodlings:
            return BWAPI::TechTypes::Spawn_Broodlings;
        case NetworkTech::Plague:
            return BWAPI::TechTypes::Plague;
        case NetworkTech::Consume:
            return BWAPI::TechTypes::Consume;
        case NetworkTech::Ensnare:
            return BWAPI::TechTypes::Ensnare;
        case NetworkTech::Lurker_Aspect:
            return BWAPI::TechTypes::Lurker_Aspect;
        case NetworkTech::Psionic_Storm:
            return BWAPI::TechTypes::Psionic_Storm;
        case NetworkTech::Hallucination:
            return BWAPI::TechTypes::Hallucination;
        case NetworkTech::Recall:
            return BWAPI::TechTypes::Recall;
        case NetworkTech::Stasis_Field:
            return BWAPI::TechTypes::Stasis_Field;
        case NetworkTech::Disruption_Web:
            return BWAPI::TechTypes::Disruption_Web;
        case NetworkTech::Mind_Control:
            return BWAPI::TechTypes::Mind_Control;
        case NetworkTech::Maelstrom:
            return BWAPI::TechTypes::Maelstrom;
        default:
            throw std::invalid_argument("Unknown NetworkTech");
        }
    }
    
    BWAPI::UpgradeType ToBWAPIUpgrade(NetworkUpgrade ut) {
        switch (ut) {
        case NetworkUpgrade::U_238_Shells:
            return BWAPI::UpgradeTypes::U_238_Shells;
        case NetworkUpgrade::Terran_Infantry_Armor:
            return BWAPI::UpgradeTypes::Terran_Infantry_Armor;
        case NetworkUpgrade::Terran_Infantry_Weapons:
            return BWAPI::UpgradeTypes::Terran_Infantry_Weapons;
        case NetworkUpgrade::Terran_Vehicle_Plating:
            return BWAPI::UpgradeTypes::Terran_Vehicle_Plating;
        case NetworkUpgrade::Terran_Vehicle_Weapons:
            return BWAPI::UpgradeTypes::Terran_Vehicle_Weapons;
        case NetworkUpgrade::Ion_Thrusters:
            return BWAPI::UpgradeTypes::Ion_Thrusters;
        case NetworkUpgrade::Titan_Reactor:
            return BWAPI::UpgradeTypes::Titan_Reactor;
        case NetworkUpgrade::Terran_Ship_Plating:
            return BWAPI::UpgradeTypes::Terran_Ship_Plating;
        case NetworkUpgrade::Terran_Ship_Weapons:
            return BWAPI::UpgradeTypes::Terran_Ship_Weapons;
        case NetworkUpgrade::Moebius_Reactor:
            return BWAPI::UpgradeTypes::Moebius_Reactor;
        case NetworkUpgrade::Ocular_Implants:
            return BWAPI::UpgradeTypes::Ocular_Implants;
        case NetworkUpgrade::Apollo_Reactor:
            return BWAPI::UpgradeTypes::Apollo_Reactor;
        case NetworkUpgrade::Colossus_Reactor:
            return BWAPI::UpgradeTypes::Colossus_Reactor;
        case NetworkUpgrade::Caduceus_Reactor:
            return BWAPI::UpgradeTypes::Caduceus_Reactor;
        case NetworkUpgrade::Charon_Boosters:
            return BWAPI::UpgradeTypes::Charon_Boosters;
        case NetworkUpgrade::Zerg_Carapace:
            return BWAPI::UpgradeTypes::Zerg_Carapace;
        case NetworkUpgrade::Zerg_Flyer_Carapace:
            return BWAPI::UpgradeTypes::Zerg_Flyer_Carapace;
        case NetworkUpgrade::Zerg_Melee_Attacks:
            return BWAPI::UpgradeTypes::Zerg_Melee_Attacks;
        case NetworkUpgrade::Zerg_Missile_Attacks:
            return BWAPI::UpgradeTypes::Zerg_Missile_Attacks;
        case NetworkUpgrade::Zerg_Flyer_Attacks:
            return BWAPI::UpgradeTypes::Zerg_Flyer_Attacks;
        case NetworkUpgrade::Ventral_Sacs:
            return BWAPI::UpgradeTypes::Ventral_Sacs;
        case NetworkUpgrade::Antennae:
            return BWAPI::UpgradeTypes::Antennae;
        case NetworkUpgrade::Pneumatized_Carapace:
            return BWAPI::UpgradeTypes::Pneumatized_Carapace;
        case NetworkUpgrade::Metabolic_Boost:
            return BWAPI::UpgradeTypes::Metabolic_Boost;
        case NetworkUpgrade::Adrenal_Glands:
            return BWAPI::UpgradeTypes::Adrenal_Glands;
        case NetworkUpgrade::Muscular_Augments:
            return BWAPI::UpgradeTypes::Muscular_Augments;
        case NetworkUpgrade::Grooved_Spines:
            return BWAPI::UpgradeTypes::Grooved_Spines;
        case NetworkUpgrade::Gamete_Meiosis:
            return BWAPI::UpgradeTypes::Gamete_Meiosis;
        case NetworkUpgrade::Metasynaptic_Node:
            return BWAPI::UpgradeTypes::Metasynaptic_Node;
        case NetworkUpgrade::Chitinous_Plating:
            return BWAPI::UpgradeTypes::Chitinous_Plating;
        case NetworkUpgrade::Anabolic_Synthesis:
            return BWAPI::UpgradeTypes::Anabolic_Synthesis;
        case NetworkUpgrade::Protoss_Ground_Armor:
            return BWAPI::UpgradeTypes::Protoss_Ground_Armor;
        case NetworkUpgrade::Protoss_Air_Armor:
            return BWAPI::UpgradeTypes::Protoss_Air_Armor;
        case NetworkUpgrade::Protoss_Ground_Weapons:
            return BWAPI::UpgradeTypes::Protoss_Ground_Weapons;
        case NetworkUpgrade::Protoss_Air_Weapons:
            return BWAPI::UpgradeTypes::Protoss_Air_Weapons;
        case NetworkUpgrade::Protoss_Plasma_Shields:
            return BWAPI::UpgradeTypes::Protoss_Plasma_Shields;
        case NetworkUpgrade::Singularity_Charge:
            return BWAPI::UpgradeTypes::Singularity_Charge;
        case NetworkUpgrade::Leg_Enhancements:
            return BWAPI::UpgradeTypes::Leg_Enhancements;
        case NetworkUpgrade::Scarab_Damage:
            return BWAPI::UpgradeTypes::Scarab_Damage;
        case NetworkUpgrade::Reaver_Capacity:
            return BWAPI::UpgradeTypes::Reaver_Capacity;
        case NetworkUpgrade::Gravitic_Drive:
            return BWAPI::UpgradeTypes::Gravitic_Drive;
        case NetworkUpgrade::Sensor_Array:
            return BWAPI::UpgradeTypes::Sensor_Array;
        case NetworkUpgrade::Gravitic_Boosters:
            return BWAPI::UpgradeTypes::Gravitic_Boosters;
        case NetworkUpgrade::Khaydarin_Amulet:
            return BWAPI::UpgradeTypes::Khaydarin_Amulet;
        case NetworkUpgrade::Apial_Sensors:
            return BWAPI::UpgradeTypes::Apial_Sensors;
        case NetworkUpgrade::Gravitic_Thrusters:
            return BWAPI::UpgradeTypes::Gravitic_Thrusters;
        case NetworkUpgrade::Carrier_Capacity:
            return BWAPI::UpgradeTypes::Carrier_Capacity;
        case NetworkUpgrade::Khaydarin_Core:
            return BWAPI::UpgradeTypes::Khaydarin_Core;
        case NetworkUpgrade::Argus_Jewel:
            return BWAPI::UpgradeTypes::Argus_Jewel;
        case NetworkUpgrade::Argus_Talisman:
            return BWAPI::UpgradeTypes::Argus_Talisman;
        default:
            throw std::invalid_argument("Unknown NetworkUpgrade");
        }
    }


}