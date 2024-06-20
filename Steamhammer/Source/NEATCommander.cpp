#include "NEATCommander.h"
#include "Config.h"
#include "MacroAct.h"
#include "The.h"
#include <windows.h> 

namespace UAlbertaBot
{
    NEATCommander& NEATCommander::Instance()
    {
        static NEATCommander instance;
        
        return instance;
    }
    NEATCommander::NEATCommander() {
        mapWidth = BWAPI::Broodwar->mapWidth();
        mapHeight = BWAPI::Broodwar->mapHeight();
        //We'll look 32x32 section
        // try dividing
        //for example 256x256
        int dividedWidth = mapWidth / 32; //get 8
        int dividedHeight = mapHeight / 32;

        fitness = -Config::NEAT::SubtractFitnessScore;

        //Get number of sections in the map
        maxSections = dividedWidth * dividedHeight;
        for (int i = 0; i < dividedWidth; i++)
        {
            for (int j = 0; j < dividedHeight; j++)
            {
                sectionsCoords.push_back({ i * 32, j * 32 });
            }
        }
        InitializeNetwork();
    }
    void NEATCommander::update()
    {
        evaluate();
    }
    BuildOrder NEATCommander::getMacroCommands()
    {
        return BuildOrder(BWAPI::Broodwar->self()->getRace(), _actions);
    }
    void NEATCommander::resetActions()
    {
        _actions.clear();
    }
    void NEATCommander::incrementFrame()
    {
        frame++;
    }
    double NEATCommander::getFitness()
    {
        return fitness;
    }
    void NEATCommander::scoreFitness(double add)
    {

        UAB_ASSERT(fitness > 0.0f, "Negative Fitness: %s", std::to_string(fitness).c_str());
        fitness += add;
    }
    void NEATCommander::sendFitnessToTrainServer()
    {
        if (Config::NEAT::Train)
        {
            std::cout << "Genome ID " << genomeID << " ";
            std::cout << "Fitness Calculated: " << fitness << "\n";
            DBKeySpace dbkeys{};
            dbkeys.push_back("Fitness");

            DBConnector db{ Config::NEAT::TrainingServerIP.c_str(), dbkeys };

            try {
                db.addToData("Fitness", fitness);

                std::string url = "/genome/" + std::to_string(genomeID) + "/fitness";
                std::cout << url << "\n";
                db.sendData(url.c_str());
            }
            catch (std::exception e) {
                std::cout << "Error while sending fitness value to the training server: " << e.what() << "\n";
            }
        }
    }
    void NEATCommander::evaluate()
    {
        if (!network) return;
        inputVector.clear();
        
        getVisibleMap(curSection);
        
        
        int mSupply = BWAPI::Broodwar->self()->supplyTotal();
        int cSupply = BWAPI::Broodwar->self()->supplyUsed();

        int min = BWAPI::Broodwar->self()->minerals();
        if(min > 13000 && Config::NEAT::AutoSurrender) BWAPI::Broodwar->leaveGame(); //Something went wrong if you have this much mineral
        int gas = BWAPI::Broodwar->self()->gas();

        double deltaMineral = min > lastMineral ? min - lastMineral : 0;
        double deltaGas = gas > lastGas ? gas - lastGas : 0;

        lastMineral = min;
        lastGas = gas;

        scoreFitness(deltaMineral / Config::NEAT::FitnessScore_Gas_Divider);
        scoreFitness(deltaGas / Config::NEAT::FitnessScore_Gas_Divider);

        int ctime = frame;
        int sectionCoordW = sectionsCoords[curSection][0];
        int sectionCoordH = sectionsCoords[curSection][1];
        BWAPI::Unitset myUnits = the.self()->getUnits();
        int workerCount = getWorkerCount(myUnits);
        int enemyRace = BWAPI::Broodwar->enemy()->getRace();

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                inputVector.push_back(friendlyMapData[i][j]);
            }
        }
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                inputVector.push_back(enemyMapData[i][j]);
            }
        }

        inputVector.push_back(mSupply);
        inputVector.push_back(cSupply);
        inputVector.push_back(min);
        inputVector.push_back(gas);
        inputVector.push_back(ctime);
        inputVector.push_back(ctime);
        inputVector.push_back(sectionCoordW);
        inputVector.push_back(sectionCoordH);
        inputVector.push_back(workerCount);
        inputVector.push_back(enemyRace);

        network->Activate(inputVector);
        //output vector size is 10
        macroActType += network->getOutputVector()[0];
        unitType += network->getOutputVector()[1];
        techType += network->getOutputVector()[2];
        upgradeType += network->getOutputVector()[3];
        macroCommandType += network->getOutputVector()[4];
        amount1 += network->getOutputVector()[5];
        amount2 += network->getOutputVector()[6];
        tilePosX += network->getOutputVector()[7];
        tilePosY += network->getOutputVector()[8];
        macroCommandUnitType += network->getOutputVector()[9];

        curSection++;
        curSection = curSection % maxSections;
        if (curSection == 0)
        {
            int a = macroCommandUnitType > 0 ? (int)macroCommandUnitType : 0;
            a = a >= (int)NetworkUnits::NETWORK_UNIT_COUNT ? (int)NetworkUnits::NETWORK_UNIT_COUNT - 1 : a;
            NetworkUnits macroUT = (NetworkUnits)a;

            int b = unitType > 0 ? (int)unitType : 0;
            b = b >= (int)NetworkUnits::NETWORK_UNIT_COUNT ? (int)NetworkUnits::NETWORK_UNIT_COUNT - 1 : b;
            NetworkUnits ut = (NetworkUnits)b;

            int c = techType > 0 ? (int)techType : 0;
            c = c >= (int)NetworkTech::NETWORK_TECH_COUNT ? (int)NetworkTech::NETWORK_TECH_COUNT - 1 : c;
            NetworkTech tt = (NetworkTech)c;

            int d = upgradeType > 0 ? (int)upgradeType : 0;
            d = d >= (int)NetworkUpgrade::NETWORK_UPGRADE_COUNT ? (int)NetworkUpgrade::NETWORK_UPGRADE_COUNT - 1 : d;
            NetworkUpgrade ugt = (NetworkUpgrade)d;

            int e = macroCommandType > 0 ? (int)macroCommandType : 0;
            e = e >= (int)MacroCommandType::QueueBarrier ? (int)MacroCommandType::QueueBarrier - 1 : e;
            MacroCommandType mct = (MacroCommandType)e;

            int mat = macroActType > 0 ? (int)macroActType : 0;
            mat = mat >= (int)MacroActs::Default ? (int)MacroActs::Default - 1 : mat;

            MacroCommand mc = MacroCommand(mct, amount1,amount2, ToBWAPIUnit(macroUT));
            //BWAPI::TilePosition tp = ;
            MacroAct ma = MacroAct(
                mc, 
                ToBWAPIUnit(ut), 
                ToBWAPITech(tt), 
                ToBWAPIUpgrade(ugt), 
                (MacroActs)mat, 
                { (int) tilePosX , (int) tilePosY });
        
            //MacroAct action{};
            _actions.push_back(ma);
            //std::cout << "Network Evaluated " << std::to_string(_actions.size()) << " actions\n";
        
            macroActType = 0.0f; //Unit, Tech, Upgrade, Command, Default
            unitType = 0.0f;
            techType = 0.0f;
            upgradeType = 0.0f;
            macroCommandType = 0.0f;
            amount1 = 0.0f;
            amount2 = 0.0f;
            tilePosX = 0.0f; //0~256
            tilePosY = 0.0f; //0~256
            macroCommandUnitType = 0.0f;
        }
    }

    void NEATCommander::InitializeNetwork()
    {
        if (!Config::NEAT::LoadNetworkFromJSON)
        {
            bool retry = true;
            int id = -1;
            json r{};
            while (retry) {
                DBKeySpace dbkeys{};
                dbkeys.push_back("Fitness");
                DBConnector db{ Config::NEAT::TrainingServerIP.c_str(), dbkeys};
                bool res = db.getData("/genome", r);
                try {
                    //network
                    id = r[0]["id"];
                    std::cout << id << std::endl;

                    if (id == -1 || !res) throw std::exception("id is -1");
                    retry = false;
                }
                catch (std::exception e) {
                    //std::cout << "Error while retrieving new genome to evaluate: " << e.what() << "\n";
                    //std::cout << "\tWaiting for 3000 miliseconds\n";
                    r.clear();
                    id = -1;
                    Sleep(2000);
                }
            }
        
            std::cout << "Creating Network Structure" << id << "\n";

            try {
                json networkJson = json::parse(std::string(r[0]["Network"]));
                //Delete existing network
                if (!network) delete network;
                network = new FeedForwardNetwork(networkJson["input_keys"], networkJson["output_keys"]);
                for (const json& ne : networkJson["node_evals"]) {
                    network->AddNodeEval(ne);
                }
                if (network->IsNodeEvalEmpty()) throw std::exception("Faulty Gene");//Faulty Gene. Surrender immediately;
            }
            catch (std::exception e) {
                std::cout << "Error creating Network Structure: " << e.what() << "\n";
                BWAPI::Broodwar->leaveGame();
            }
            genomeID = id;
        }
    }

    int NEATCommander::getWorkerCount(BWAPI::Unitset& allMyUnits)
    {
        int r = 0;
        for (auto u : allMyUnits) {
            if (isWorkerType(u->getType()))
            {
                r++;
            }
        }
        return r;
    }

    bool NEATCommander::isWorkerType(BWAPI::UnitType type) {
        if (type == BWAPI::UnitTypes::Terran_SCV || type == BWAPI::UnitTypes::Zerg_Drone || type == BWAPI::UnitTypes::Protoss_Probe) {
            return true;
        }
    }

    /// <summary>
    /// Retrieves map data of the given section
    /// Each section is 32x32 Tiles big
    /// This function fills up the 16x16 mapData array
    /// To do this, we half-resolution the given section
    /// Units with highest importance will override the data on tile
    /// </summary>
    /// <param name="sectionNum"></param>
    void NEATCommander::getVisibleMap(int sectionNum)
    {
        int a = sectionNum;
        if (sectionNum >= maxSections) throw std::overflow_error("sectionNum is bigger than maxSections");

        int startW = sectionsCoords[sectionNum][0];
        int startH = sectionsCoords[sectionNum][1];
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                BWAPI::TilePosition tp = { startW + i * 2,startH + j * 2 };
                BWAPI::WalkPosition wp = { (startW + i * 2) * 4,(startH + j * 2) * 4 }; //WalkPosition is 8 pixels big
                if (!BWAPI::Broodwar->isVisible(tp)) {
                    friendlyMapData[i][j] = NEAT_TileType::FOG;
                    enemyMapData[i][j] = NEAT_TileType::FOG;
                    break;
                }
                else {
                    //BWAPI::UnitFilter uf = BWAPI::UnitFilter(the.self());
                    BWAPI::Unitset allyUnitsOnTile = BWAPI::Broodwar->getUnitsOnTile(startW + i * 2, startH + j * 2, BWAPI::Filter::IsAlly);
                    BWAPI::Unitset enemyUnitsOnTile = BWAPI::Broodwar->getUnitsOnTile(startW + i * 2, startH + j * 2, BWAPI::Filter::IsEnemy);

                    NEAT_TileType highestImportanceAlly = NEAT_TileType::NOTWALKABLE;
                    NEAT_TileType highestImportanceEnemy = NEAT_TileType::NOTWALKABLE;
                    if (BWAPI::Broodwar->isWalkable(wp)) {
                        highestImportanceAlly = NEAT_TileType::WALKABLE;
                        highestImportanceEnemy = NEAT_TileType::WALKABLE;
                    }

                    for (BWAPI::Unit u : allyUnitsOnTile) {
                        NEAT_TileType unitTileType = getTileType(u->getType());
                        highestImportanceAlly = unitTileType > highestImportanceAlly ? unitTileType : highestImportanceAlly;
                    }
                    for (BWAPI::Unit u : enemyUnitsOnTile) {
                        NEAT_TileType unitTileType = getTileType(u->getType());
                        highestImportanceEnemy = unitTileType > highestImportanceEnemy ? unitTileType : highestImportanceEnemy;
                    }
                    friendlyMapData[i][j] = highestImportanceAlly;
                    enemyMapData[i][j] = highestImportanceEnemy;
                }
            }
        }
    }

    BWAPI::UnitType NEATCommander::ToBWAPIUnit(NetworkUnits ut) {
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
        case NetworkUnits::Zerg_Spore_Colony:
            return BWAPI::UnitTypes::Zerg_Spore_Colony;
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
            return BWAPI::UnitTypes::Terran_SCV;
        }
    }

    BWAPI::TechType NEATCommander::ToBWAPITech(NetworkTech tt) 
    {
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
            return BWAPI::TechTypes::Stim_Packs;
        }
    }
    BWAPI::UpgradeType NEATCommander::ToBWAPIUpgrade(NetworkUpgrade ut) 
    {
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
            return BWAPI::UpgradeTypes::U_238_Shells;
        }
    }

    NEAT_TileType NEATCommander::getTileType(BWAPI::UnitType type) 
    {
        switch (type) {
            //Terran Buildings
        case BWAPI::UnitTypes::Terran_Academy:
            return NEAT_TileType::Terran_Academy;
        case BWAPI::UnitTypes::Terran_Armory:
            return NEAT_TileType::Terran_Armory;
        case BWAPI::UnitTypes::Terran_Barracks:
            return NEAT_TileType::Terran_Barracks;
        case BWAPI::UnitTypes::Terran_Bunker:
            return NEAT_TileType::Terran_Bunker;
        case BWAPI::UnitTypes::Terran_Command_Center:
            return NEAT_TileType::Terran_Command_Center;
        case BWAPI::UnitTypes::Terran_Engineering_Bay:
            return NEAT_TileType::Terran_Engineering_Bay;
        case BWAPI::UnitTypes::Terran_Factory:
            return NEAT_TileType::Terran_Factory;
        case BWAPI::UnitTypes::Terran_Starport:
            return NEAT_TileType::Terran_Starport;
        case BWAPI::UnitTypes::Terran_Supply_Depot:
            return NEAT_TileType::Terran_Supply_Depot;
        case BWAPI::UnitTypes::Terran_Refinery:
            return NEAT_TileType::Terran_Refinery;
        case BWAPI::UnitTypes::Terran_Science_Facility:
            return NEAT_TileType::Terran_Science_Facility;
        case BWAPI::UnitTypes::Terran_Comsat_Station:
            return NEAT_TileType::Terran_Comsat_Station;
        case BWAPI::UnitTypes::Terran_Nuclear_Silo:
            return NEAT_TileType::Terran_Nuclear_Silo;
        case BWAPI::UnitTypes::Terran_Control_Tower:
            return NEAT_TileType::Terran_Control_Tower;
        case BWAPI::UnitTypes::Terran_Covert_Ops:
            return NEAT_TileType::Terran_Covert_Ops;
        case BWAPI::UnitTypes::Terran_Physics_Lab:
            return NEAT_TileType::Terran_Physics_Lab;
        case BWAPI::UnitTypes::Terran_Machine_Shop:
            return NEAT_TileType::Terran_Machine_Shop;
        case BWAPI::UnitTypes::Terran_Missile_Turret:
            return NEAT_TileType::Terran_Missile_Turret;

            //Zerg buildings
        case BWAPI::UnitTypes::Zerg_Hatchery:
            return NEAT_TileType::Zerg_Hatchery;
        case BWAPI::UnitTypes::Zerg_Lair:
            return NEAT_TileType::Zerg_Lair;
        case BWAPI::UnitTypes::Zerg_Hive:
            return NEAT_TileType::Zerg_Hive;
        case BWAPI::UnitTypes::Zerg_Nydus_Canal:
            return NEAT_TileType::Zerg_Nydus_Canal;
        case BWAPI::UnitTypes::Zerg_Hydralisk_Den:
            return NEAT_TileType::Zerg_Hydralisk_Den;
        case BWAPI::UnitTypes::Zerg_Defiler_Mound:
            return NEAT_TileType::Zerg_Defiler_Mound;
        case BWAPI::UnitTypes::Zerg_Greater_Spire:
            return NEAT_TileType::Zerg_Greater_Spire;
        case BWAPI::UnitTypes::Zerg_Queens_Nest:
            return NEAT_TileType::Zerg_Queens_Nest;
        case BWAPI::UnitTypes::Zerg_Evolution_Chamber:
            return NEAT_TileType::Zerg_Evolution_Chamber;
        case BWAPI::UnitTypes::Zerg_Ultralisk_Cavern:
            return NEAT_TileType::Zerg_Ultralisk_Cavern;
        case BWAPI::UnitTypes::Zerg_Spire:
            return NEAT_TileType::Zerg_Spire;
        case BWAPI::UnitTypes::Zerg_Spawning_Pool:
            return NEAT_TileType::Zerg_Spawning_Pool;
        case BWAPI::UnitTypes::Zerg_Creep_Colony:
            return NEAT_TileType::Zerg_Creep_Colony;
        case BWAPI::UnitTypes::Zerg_Spore_Colony:
            return NEAT_TileType::Zerg_Spore_Colony;
        case BWAPI::UnitTypes::Zerg_Sunken_Colony:
            return NEAT_TileType::Zerg_Sunken_Colony;
        case BWAPI::UnitTypes::Zerg_Extractor:
            return NEAT_TileType::Zerg_Extractor;

            //Protoss buildings
        case BWAPI::UnitTypes::Protoss_Nexus:
            return NEAT_TileType::Protoss_Nexus;
        case BWAPI::UnitTypes::Protoss_Robotics_Facility:
            return NEAT_TileType::Protoss_Robotics_Facility;
        case BWAPI::UnitTypes::Protoss_Pylon:
            return NEAT_TileType::Protoss_Pylon;
        case BWAPI::UnitTypes::Protoss_Assimilator:
            return NEAT_TileType::Protoss_Assimilator;
        case BWAPI::UnitTypes::Protoss_Observatory:
            return NEAT_TileType::Protoss_Observatory;
        case BWAPI::UnitTypes::Protoss_Gateway:
            return NEAT_TileType::Protoss_Gateway;
        case BWAPI::UnitTypes::Protoss_Photon_Cannon:
            return NEAT_TileType::Protoss_Photon_Cannon;
        case BWAPI::UnitTypes::Protoss_Citadel_of_Adun:
            return NEAT_TileType::Protoss_Citadel_of_Adun;
        case BWAPI::UnitTypes::Protoss_Cybernetics_Core:
            return NEAT_TileType::Protoss_Cybernetics_Core;
        case BWAPI::UnitTypes::Protoss_Templar_Archives:
            return NEAT_TileType::Protoss_Templar_Archives;
        case BWAPI::UnitTypes::Protoss_Forge:
            return NEAT_TileType::Protoss_Forge;
        case BWAPI::UnitTypes::Protoss_Stargate:
            return NEAT_TileType::Protoss_Stargate;
        case BWAPI::UnitTypes::Protoss_Fleet_Beacon:
            return NEAT_TileType::Protoss_Fleet_Beacon;
        case BWAPI::UnitTypes::Protoss_Arbiter_Tribunal:
            return NEAT_TileType::Protoss_Arbiter_Tribunal;
        case BWAPI::UnitTypes::Protoss_Robotics_Support_Bay:
            return NEAT_TileType::Protoss_Robotics_Support_Bay;
        case BWAPI::UnitTypes::Protoss_Shield_Battery:
            return NEAT_TileType::Protoss_Shield_Battery;

            // Terran Units
        case BWAPI::UnitTypes::Terran_Marine:
            return NEAT_TileType::Terran_Marine;
        case BWAPI::UnitTypes::Terran_Firebat:
            return NEAT_TileType::Terran_Firebat;
        case BWAPI::UnitTypes::Terran_Medic:
            return NEAT_TileType::Terran_Medic;
        case BWAPI::UnitTypes::Terran_Ghost:
            return NEAT_TileType::Terran_Ghost;
        case BWAPI::UnitTypes::Terran_Vulture:
            return NEAT_TileType::Terran_Vulture;
        case BWAPI::UnitTypes::Terran_Goliath:
            return NEAT_TileType::Terran_Goliath;
        case BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode:
            return NEAT_TileType::Terran_Siege_Tank_Tank_Mode;
        case BWAPI::UnitTypes::Terran_Wraith:
            return NEAT_TileType::Terran_Wraith;
        case BWAPI::UnitTypes::Terran_Dropship:
            return NEAT_TileType::Terran_Dropship;
        case BWAPI::UnitTypes::Terran_Science_Vessel:
            return NEAT_TileType::Terran_Science_Vessel;
        case BWAPI::UnitTypes::Terran_Battlecruiser:
            return NEAT_TileType::Terran_Battlecruiser;
        case BWAPI::UnitTypes::Terran_Valkyrie:
            return NEAT_TileType::Terran_Valkyrie;
        case BWAPI::UnitTypes::Terran_SCV:
            return NEAT_TileType::Terran_SCV;

            // Zerg Units
        case BWAPI::UnitTypes::Zerg_Zergling:
            return NEAT_TileType::Zerg_Zergling;
        case BWAPI::UnitTypes::Zerg_Hydralisk:
            return NEAT_TileType::Zerg_Hydralisk;
        case BWAPI::UnitTypes::Zerg_Ultralisk:
            return NEAT_TileType::Zerg_Ultralisk;
        case BWAPI::UnitTypes::Zerg_Broodling:
            return NEAT_TileType::Zerg_Broodling;
        case BWAPI::UnitTypes::Zerg_Drone:
            return NEAT_TileType::Zerg_Drone;
        case BWAPI::UnitTypes::Zerg_Defiler:
            return NEAT_TileType::Zerg_Defiler;
        case BWAPI::UnitTypes::Zerg_Infested_Terran:
            return NEAT_TileType::Zerg_Infested_Terran;
        case BWAPI::UnitTypes::Zerg_Lurker:
            return NEAT_TileType::Zerg_Lurker;
        case BWAPI::UnitTypes::Zerg_Overlord:
            return NEAT_TileType::Zerg_Overlord;
        case BWAPI::UnitTypes::Zerg_Mutalisk:
            return NEAT_TileType::Zerg_Mutalisk;
        case BWAPI::UnitTypes::Zerg_Guardian:
            return NEAT_TileType::Zerg_Guardian;
        case BWAPI::UnitTypes::Zerg_Queen:
            return NEAT_TileType::Zerg_Queen;
        case BWAPI::UnitTypes::Zerg_Scourge:
            return NEAT_TileType::Zerg_Scourge;
        case BWAPI::UnitTypes::Zerg_Devourer:
            return NEAT_TileType::Zerg_Devourer;

            // Protoss Units
        case BWAPI::UnitTypes::Protoss_Dark_Templar:
            return NEAT_TileType::Protoss_Dark_Templar;
        case BWAPI::UnitTypes::Protoss_Dark_Archon:
            return NEAT_TileType::Protoss_Dark_Archon;
        case BWAPI::UnitTypes::Protoss_Probe:
            return NEAT_TileType::Protoss_Probe;
        case BWAPI::UnitTypes::Protoss_Zealot:
            return NEAT_TileType::Protoss_Zealot;
        case BWAPI::UnitTypes::Protoss_Dragoon:
            return NEAT_TileType::Protoss_Dragoon;
        case BWAPI::UnitTypes::Protoss_High_Templar:
            return NEAT_TileType::Protoss_High_Templar;
        case BWAPI::UnitTypes::Protoss_Archon:
            return NEAT_TileType::Protoss_Archon;
        case BWAPI::UnitTypes::Protoss_Reaver:
            return NEAT_TileType::Protoss_Reaver;
        case BWAPI::UnitTypes::Protoss_Corsair:
            return NEAT_TileType::Protoss_Corsair;
        case BWAPI::UnitTypes::Protoss_Shuttle:
            return NEAT_TileType::Protoss_Shuttle;
        case BWAPI::UnitTypes::Protoss_Scout:
            return NEAT_TileType::Protoss_Scout;
        case BWAPI::UnitTypes::Protoss_Arbiter:
            return NEAT_TileType::Protoss_Arbiter;
        case BWAPI::UnitTypes::Protoss_Carrier:
            return NEAT_TileType::Protoss_Carrier;
        case BWAPI::UnitTypes::Protoss_Observer:
            return NEAT_TileType::Protoss_Observer;

        default:
            return NEAT_TileType::WALKABLE;
        }
    }
}