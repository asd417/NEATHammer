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
        initialized = false;
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
        builderOutputs.fill(0.0f);
        frame = 0;
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
        frame = the.now();
        if (initialized && frame > 171432) BWAPI::Broodwar->leaveGame(); //Game has been going for 2 hours... something probably went wrong
    }
    double NEATCommander::getFitness()
    {
        return fitness;
    }
    void NEATCommander::scoreFitness(double add)
    {
        //UAB_ASSERT(fitness > 0.0f, "Negative Fitness: %s", std::to_string(fitness).c_str());
        fitness += add;
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
                try {
                    DBConnector db{ Config::NEAT::TrainingServerIP.c_str(), dbkeys };
                    bool res = db.getData("/genome", r);
                    //network
                    id = r[0]["id"];
                    std::cout << id << std::endl;

                    if (id == -1 || !res) throw std::exception("id is -1");
                    retry = false;
                }
                catch (std::exception e) {
                    //std::cout << "Error while retrieving new genome to evaluate: " << e.what() << "\n";
                    //std::cout << "\tWaiting for 3000 miliseconds\n";
                    //UAB_ASSERT(false, "Error while retrieving new genome to evaluate: %s", e.what());
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
            initialized = true;
        }
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
    void NEATCommander::onUnitCreate(BWAPI::Unit unit)
    {
    }
    void NEATCommander::onUnitComplete(BWAPI::Unit unit)
    {
        BWAPI::UnitType type = unit->getType();
        
        if (unit->getPlayer() == the.self())
        {
            if (type.isBuilding())
            {
                //Not a huge motivation to build buildings
                scoreFitness(type.buildScore());
            }
            else if(type.isWorker())
            {
                //Some Motivation to build army
                scoreFitness(type.buildScore());
            }
            else
            {
                //Huge Motivation to build army
                scoreFitness(type.buildScore() * 10);
            }
        }
        
    }
    void NEATCommander::onUnitDestroy(BWAPI::Unit unit)
    {
        BWAPI::UnitType type = unit->getType();
        //Strong motivation to kill
        if (unit->getPlayer() == the.enemy()) scoreFitness(type.buildScore() * 20);
        //If building is destroyed deduct some of the score that was given by creating the building
        if (unit->getPlayer() == the.self() && type.isBuilding()) scoreFitness(-type.buildScore() / 20);
        //Worker destroyed = bad
        if (unit->getPlayer() == the.self() && type.isWorker()) scoreFitness(-type.buildScore() / 10);
    }
    void NEATCommander::onUnitHide(BWAPI::Unit unit)
    {
    }
    void NEATCommander::onUnitShow(BWAPI::Unit unit)
    {
        //Used detector to reveal invisible units
        if (unit->getPlayer() == the.enemy()) scoreFitness(Config::NEAT::EnemyShowScore);
    }
    void NEATCommander::drawDebug(int x, int y)
    {
        int startX = x;
        int startY = y;
        BWAPI::Broodwar->drawTextScreen(startX, startY, "NetworkOutputs:");
        y += 12;
        startY += 12;

        int c = 0;
        for (const double& bo : builderOutputs)
        {
            BWAPI::Broodwar->drawTextScreen(x, y, "%4.8f", bo);
            y += 12;
            c++;
            if (c == 20)
            {
                x += 60;
                y = startY;
            }
            if (c == 40)
            {
                x += 60;
                y = startY;
            }
        }

        for (const double& bo : builderOutputs)
        {
            BWAPI::Broodwar->drawTextScreen(x, y, "%4.8f", bo);
            y += 12;
            c++;
            if (c == 20)
            {
                x += 60;
                y = startY;
            }
            if (c == 40)
            {
                x += 60;
                y = startY;
            }
        }
        
        for (const double& bo : macroCommandTypeOutputs)
        {
            BWAPI::Broodwar->drawTextScreen(x, y, "%4.8f", bo);
            y += 12;
            c++;
            if (c == 20)
            {
                x += 60;
                y = startY;
            }
            if (c == 40)
            {
                x += 60;
                y = startY;
            }
        }
        
        BWAPI::Broodwar->drawTextScreen(x, y, "%4.8f", tilePosX);
        y += 12;
        BWAPI::Broodwar->drawTextScreen(x, y, "%4.8f", tilePosY);

        //for (int i = 0; i < network->getOutputCount(); i++) {
        //    double v = network->getOutputVector()[i];
        //    BWAPI::Broodwar->drawTextScreen(x, y, "%4.8f", v);
        //    y += 12;
        //    if (i == 20)
        //    {
        //        x += 60;
        //        y = startY;
        //    }
        //    if (i == 40)
        //    {
        //        x += 60;
        //        y = startY;
        //    }
        //    if (i == 60)
        //    {
        //        x += 60;
        //        y = startY;
        //    }
        //}
    
    }
    void NEATCommander::evaluate()
    {   
        if (!initialized) return;
        int min = BWAPI::Broodwar->self()->minerals();
        int gas = BWAPI::Broodwar->self()->gas();
        //Check whether to surrender
        //Something went wrong if you have this much mineral or gas
        if ((min > 10000 || gas > 10000) && Config::NEAT::AutoSurrender) BWAPI::Broodwar->leaveGame();
        if (!network) return;

        if (the.now() == _lastUpdateFrame) return;
        //Make sure to only call this once per frame
        _lastUpdateFrame = the.now();

        inputVector.clear();
        
        getVisibleMap(curSection);
        
        int mSupply = BWAPI::Broodwar->self()->supplyTotal();
        int cSupply = BWAPI::Broodwar->self()->supplyUsed();

        double deltaMineral = min > lastMineral ? min - lastMineral : 0;
        double deltaGas = gas > lastGas ? gas - lastGas : 0;

        lastMineral = min;
        lastGas = gas;

        //What if we remove the gas and mineral from fitness score?
        //scoreFitness(deltaMineral / Config::NEAT::FitnessScore_Gas_Divider);
        //scoreFitness(deltaGas / Config::NEAT::FitnessScore_Gas_Divider);

        int ctime = frame;
        int sectionCoordW = sectionsCoords[curSection][0];
        int sectionCoordH = sectionsCoords[curSection][1];
        BWAPI::Unitset myUnits = the.self()->getUnits();
        int workerCount = getWorkerCount(myUnits);
        int enemyRace = BWAPI::Broodwar->enemy()->getRace();

#define HyperNEAT

#ifndef HyperNEAT
        //NEAT Input: 16x16 + 16x16 + 12 = 524
        
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                inputVector.push_back(friendlyMapData[i][j]);
            }
        }
        //16x16 + 16x16
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
        inputVector.push_back(mapWidth);
        inputVector.push_back(mapWidth);
        inputVector.push_back(workerCount);
        inputVector.push_back(enemyRace);
#else
        //HyperNEAT encoding 16x2 x 16x2 = 32x32 = 1024 input nodes
        //fm |em
        //-------
        //fmb|emb
            
        for (int xi = 0; xi < 32; xi++)
        {
            for (int yi = 0; yi < 32; yi++)
            {
                if (xi < 16)
                {
                    if (yi < 16)
                    {
                        //Friendly map encoding
                        inputVector.push_back(friendlyMapData[xi][yi]);
                    }
                    else
                    {
                        //yi >= 16
                        //Friendly building map encoding
                        inputVector.push_back(friendlyMapBuildingData[xi][yi - 16]);
                    }
                }
                else
                {
                    if (yi < 16)
                    {
                        //enemy map encoding
                        inputVector.push_back(enemyMapData[xi - 16][yi]);
                    }
                    else
                    {
                        //enemy building map encoding
                        inputVector.push_back(enemyMapData[xi - 16][yi - 16]);
                    }
                }
            }
        }
        inputVector[0] = mSupply;
        inputVector[1] = cSupply;
        inputVector[2] = min;
        inputVector[3] = gas;
        inputVector[4] = ctime;
        inputVector[5] = ctime;
        inputVector[6] = sectionCoordW;
        inputVector[7] = sectionCoordH;
        inputVector[8] = mapWidth;
        inputVector[9] = mapWidth;
        inputVector[10] = workerCount;
        inputVector[11] = enemyRace;
#endif
        network->Activate(inputVector);
        //Read output nodes 
        // 
        // Network can output:
        //  Type of thing to build,
        //  MacroCommandType, 
        //  Tile position used by some of the macro command types and building build position
        // 
        //BuilderOutput and MacroCommandTypeOutput will be chosen through strongest node output 

#ifdef PROTOSS
        for (int i = 0; i < (int)NetworkProtossOptions::NETWORK_OPTION_COUNT; i++)
        {
            builderOutputs[i] += network->getOutputVector()[i];
        }
        constexpr int outputVectorOffset = (int)NetworkProtossOptions::NETWORK_OPTION_COUNT + (int)MacroCommandType::QueueBarrier;
        for (int i = (int)NetworkProtossOptions::NETWORK_OPTION_COUNT; i < outputVectorOffset; i++)
        {
            macroCommandTypeOutputs[i] += network->getOutputVector()[i];
        }
        tilePosX += network->getOutputVector()[outputVectorOffset];
        tilePosY += network->getOutputVector()[outputVectorOffset+1];

        //We scanned through the whole map
        curSection++;
        curSection = curSection % maxSections;
        //Looked through all sections. Make command
        if (curSection == 0)
        {
            std::clamp(tilePosX, (double) 0.0f, (double) 1.0f);
            std::clamp(tilePosY, (double) 0.0f, (double) 1.0f);
            int posx = int(tilePosX * mapWidth);
            int posy = int(tilePosY * mapHeight);
            BWAPI::TilePosition buildPos = { posx,posy };

            int highestBuildOptionOutput = 0;
            double highestBuildOptionOutputScore = 0;
            for (int i = 0; i < (int)NetworkProtossOptions::NETWORK_OPTION_COUNT; i++)
            {
                if (builderOutputs[i] > highestBuildOptionOutputScore && canBuild((NetworkProtossOptions)i, buildPos))
                {
                    highestBuildOptionOutput = i;
                    highestBuildOptionOutputScore = builderOutputs[i];
                }
            }
            
            int highestMacroCommandTypeOutput = 0;
            double highestMacroCommandTypeOutputScore = 0;
            for (int i = 0; i < (int)MacroCommandType::QueueBarrier; i++)
            {
                //Check if unit exists
                if (macroCommandTypeOutputs[i] > highestMacroCommandTypeOutputScore)
                {
                    highestMacroCommandTypeOutput = i;
                    highestMacroCommandTypeOutputScore = macroCommandTypeOutputs[i];
                }
            }

            MacroAct ma;
            // compare score between build option score and macro command type score
            if (highestBuildOptionOutputScore > highestMacroCommandTypeOutputScore)
            {
                //BuildOutput has stronger signal than MacroCommandUnitType signal
                if ((NetworkProtossOptions)highestBuildOptionOutput < NetworkProtossOptions::Psionic_Storm) //unit or building
                {
                    ma = MacroAct(ToBWAPIUnit((NetworkProtossOptions)highestBuildOptionOutput), buildPos);
                }
                else if ((NetworkProtossOptions)highestBuildOptionOutput < NetworkProtossOptions::Protoss_Ground_Armor) //tech
                {
                    ma = MacroAct(ToBWAPITech((NetworkProtossOptions)highestBuildOptionOutput));
                }
                else //upgrade
                {
                    ma = MacroAct(ToBWAPIUpgrade((NetworkProtossOptions)highestBuildOptionOutput));
                }
                ma.confidence = highestBuildOptionOutputScore;
            }
            else {
                ma = MacroAct((MacroCommandType)highestMacroCommandTypeOutput, {posx,posy});
                ma.confidence = highestMacroCommandTypeOutputScore;
            }
            
            _actions.push_back(ma);
            //std::cout << "Network Evaluated " << std::to_string(_actions.size()) << " actions\n";
            //Reset output vector
            builderOutputs.fill(0.0f);
            macroCommandTypeOutputs.fill(0.0f);
            tilePosX = 0.0f; //0~256
            tilePosY = 0.0f; //0~256
        }
#else
        for (int i = 0; i < (int)NetworkTerranOptions::NETWORK_OPTION_COUNT; i++)
        {
            builderOutputs[i] += network->getOutputVector()[i];
        }
        constexpr int outputVectorOffset = (int)NetworkTerranOptions::NETWORK_OPTION_COUNT + (int)MacroCommandType::QueueBarrier;
        for (int i = (int)NetworkTerranOptions::NETWORK_OPTION_COUNT; i < outputVectorOffset; i++)
        {
            macroCommandTypeOutputs[i] += network->getOutputVector()[i];
        }
        tilePosX += network->getOutputVector()[outputVectorOffset];
        tilePosY += network->getOutputVector()[outputVectorOffset + 1];

        //We scanned through the whole map
        curSection++;
        curSection = curSection % maxSections;
        //Looked through all sections. Make command
        if (curSection == 0)
        {
            tilePosX = tilePosX / (double) maxSections;
            tilePosY = tilePosY / (double) maxSections;
            std::clamp(tilePosX, (double)0.0f, (double)1.0f);
            std::clamp(tilePosY, (double)0.0f, (double)1.0f);
            int posx = int(tilePosX * mapWidth);
            int posy = int(tilePosY * mapHeight);
            BWAPI::TilePosition buildPos = { posx,posy };

            int maxBuildChoice = 0;
            double maxBuildScore = 0.0f;
            for (int i = 0; i < (int)NetworkTerranOptions::NETWORK_OPTION_COUNT; i++)
            {
                if (builderOutputs[i] > maxBuildScore && canBuild((NetworkTerranOptions)i, buildPos))
                {
                    maxBuildChoice = i;
                    maxBuildScore = builderOutputs[i];
                }
            }

            int maxCommandType = 0;
            double maxCommandScore = 0.0f;
            for (int i = 0; i < (int)MacroCommandType::QueueBarrier; i++)
            {
                //Check if unit exists
                if (macroCommandTypeOutputs[i] > maxCommandScore && canMacro((MacroCommandType)i))
                {
                    maxCommandType = i;
                    maxCommandScore = macroCommandTypeOutputs[i];
                }
            }

            MacroAct ma;
            // compare score between build option score and macro command type score
            if (maxBuildScore > maxCommandScore)
            {
                //BuildOutput has stronger signal than MacroCommandUnitType signal
                if ((NetworkTerranOptions)maxBuildChoice < NetworkTerranOptions::Cloaking_Field) //unit or building
                {
                    ma = MacroAct(ToBWAPIUnit((NetworkTerranOptions)maxBuildChoice), buildPos);
                }
                else if ((NetworkTerranOptions)maxBuildChoice < NetworkTerranOptions::Apollo_Reactor) //tech
                {
                    ma = MacroAct(ToBWAPITech((NetworkTerranOptions)maxBuildChoice));
                }
                else //upgrade
                {
                    ma = MacroAct(ToBWAPIUpgrade((NetworkTerranOptions)maxBuildChoice));
                }
                ma.confidence = maxBuildChoice;
            }
            else {
                ma = MacroAct((MacroCommandType)maxCommandType, { posx,posy });
                ma.confidence = maxCommandScore;
            }
            //UAB_ASSERT(false, "MacroCommand %d with %4.8f, TerranOptionID: %d with %4.8f, ", maxCommandType, maxCommandScore, maxBuildChoice, maxBuildScore);

            //Always keep just 1 in the build queue
            if (_actions.size() > 0) _actions[0] = ma;
            else _actions.push_back(ma);
            //std::cout << "Network Evaluated " << std::to_string(_actions.size()) << " actions\n";
            //Reset output vector
            builderOutputs.fill(0.0f);
            macroCommandTypeOutputs.fill(0.0f);
            tilePosX = 0.0f; //0~256
            tilePosY = 0.0f; //0~256
        }
#endif
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

    void NEATCommander::getVisibleMapSimple(int sectionNum)
    {
        int a = sectionNum;
        if (sectionNum >= maxSections) throw std::overflow_error("sectionNum is bigger than maxSections");
        int startW = sectionsCoords[sectionNum][0];
        int startH = sectionsCoords[sectionNum][1];
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                //Half resolution it
                BWAPI::TilePosition tp = { startW + x * 2,startH + y * 2 };
                BWAPI::WalkPosition wp = { (startW + x * 2) * 4,(startH + y * 2) * 4 }; //WalkPosition is 8 pixels big

                if (!BWAPI::Broodwar->isVisible(tp)) {
                    friendlyMapData[x][y]           = NEAT_TileType_Simple::sFOG;
                    friendlyMapBuildingData[x][y]   = NEAT_TileType_Simple::sFOG;
                    enemyMapData[x][y]              = NEAT_TileType_Simple::sFOG;
                    enemyMapBuildingData[x][y]      = NEAT_TileType_Simple::sFOG;
                    break;
                }
                else
                {
                    //BWAPI::UnitFilter uf = BWAPI::UnitFilter(the.self());
                    BWAPI::Unitset allyUnitsOnTile = BWAPI::Broodwar->getUnitsOnTile(startW + x * 2, startH + y * 2, BWAPI::Filter::IsAlly);
                    BWAPI::Unitset enemyUnitsOnTile = BWAPI::Broodwar->getUnitsOnTile(startW + x * 2, startH + y * 2, BWAPI::Filter::IsEnemy);
                    BWAPI::Unitset mineralOnTile = BWAPI::Broodwar->getUnitsOnTile(startW + x * 2, startH + y * 2, 
                        BWAPI::Filter::GetType == BWAPI::UnitTypes::Resource_Mineral_Field ||
                        BWAPI::Filter::GetType == BWAPI::UnitTypes::Resource_Mineral_Field_Type_2 ||
                        BWAPI::Filter::GetType == BWAPI::UnitTypes::Resource_Mineral_Field_Type_3);
                    BWAPI::Unitset gasOnTile = BWAPI::Broodwar->getUnitsOnTile(startW + x * 2, startH + y * 2, 
                        BWAPI::Filter::GetType == BWAPI::UnitTypes::Resource_Vespene_Geyser);

                    NEAT_TileType_Simple fm     = NEAT_TileType_Simple::sNOTWALKABLE;
                    NEAT_TileType_Simple fmb    = NEAT_TileType_Simple::sNOTWALKABLE;
                    NEAT_TileType_Simple em     = NEAT_TileType_Simple::sNOTWALKABLE;
                    NEAT_TileType_Simple emb    = NEAT_TileType_Simple::sNOTWALKABLE;

                    if (BWAPI::Broodwar->isWalkable(wp)) {
                        fm  = NEAT_TileType_Simple::sWALKABLE;
                        fmb = NEAT_TileType_Simple::sWALKABLE;
                        em  = NEAT_TileType_Simple::sWALKABLE;
                        emb = NEAT_TileType_Simple::sWALKABLE;
                    }
                    if (mineralOnTile.size() > 0)
                    {
                        fm  = NEAT_TileType_Simple::sMINERAL;
                        fmb = NEAT_TileType_Simple::sMINERAL;
                        em  = NEAT_TileType_Simple::sMINERAL;
                        emb = NEAT_TileType_Simple::sMINERAL;
                    }
                    if (gasOnTile.size() > 0)
                    {
                        fm  = NEAT_TileType_Simple::sGAS;
                        fmb = NEAT_TileType_Simple::sGAS;
                        em  = NEAT_TileType_Simple::sGAS;
                        emb = NEAT_TileType_Simple::sGAS;
                    }

                    for (BWAPI::Unit u : allyUnitsOnTile) {
                        if (u->getType().isBuilding())
                        {
                            fmb = NEAT_TileType_Simple::sUNIT;
                        }
                        else
                        {
                            fm = NEAT_TileType_Simple::sUNIT;
                        }
                    }
                    for (BWAPI::Unit u : enemyUnitsOnTile) {
                        if (u->getType().isBuilding())
                        {
                            emb = NEAT_TileType_Simple::sUNIT;
                        }
                        else
                        {
                            em = NEAT_TileType_Simple::sUNIT;
                        }
                    }
                    friendlyMapData[x][y] = fm;
                    friendlyMapBuildingData[x][y] = fmb;
                    enemyMapData[x][y] = em;
                    enemyMapBuildingData[x][y] = emb;
                }
            }
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
    
    bool NEATCommander::canMacro(MacroCommandType command)
    {
        if (command == MacroCommandType::SCAN)
        {
            BWAPI::Unitset comsats = BWAPI::Broodwar->self()->getUnits();
            bool r = false;
            for (auto& u : comsats)
            {
                if (u->getType() == BWAPI::UnitTypes::Terran_Comsat_Station && u->getEnergy() >= 50)
                {
                    r = true;
                }
            }
            return r;
        }
        if (command == MacroCommandType::SPIDERMINE)
        {
            return the.my.completed.count(BWAPI::UnitTypes::Terran_Vulture) > 0 && BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Spider_Mines);
        }
        if (command == MacroCommandType::SIEGEMODE)
        {
            return the.my.completed.count(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode) > 0 && BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode);
        }
        if (command == MacroCommandType::UNSIEGEMODE)
        {
            return the.my.completed.count(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode) > 0 && BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode);
        }
        if (command == MacroCommandType::NUKE)
        {
            return the.my.completed.count(BWAPI::UnitTypes::Terran_Nuclear_Missile) > 0;
        }
        return true;
    }

    bool NEATCommander::canBuild(NetworkTerranOptions option, BWAPI::TilePosition& location)
    {
        if (option < NetworkTerranOptions::Cloaking_Field)
        {
            //unit or building
            BWAPI::UnitType type = ToBWAPIUnit(option);

            if (type.isRefinery()) return BWAPI::Broodwar->canMake(type);
            else if (type.isAddon()) return BWAPI::Broodwar->canMake(type);
            else if (type.isBuilding()) {
                location = the.placer.getBuildLocationNear(Building(type, location), 5);
                return BWAPI::Broodwar->canMake(type) && BWAPI::Broodwar->canBuildHere(location, type);
            }
            else return BWAPI::Broodwar->canMake(type);
        }
        else if (option < NetworkTerranOptions::Apollo_Reactor)
        {
            //tech
            return BWAPI::Broodwar->canResearch(ToBWAPITech(option));
        }
        else
        {
            //upgrade
            return BWAPI::Broodwar->canUpgrade(ToBWAPIUpgrade(option));
        }
    }

    bool NEATCommander::canBuild(NetworkProtossOptions option, BWAPI::TilePosition& location)
    {
        if (option < NetworkProtossOptions::Psionic_Storm && location.isValid())
        {
            //unit or building
            BWAPI::UnitType type = ToBWAPIUnit(option);
            
            if (type.isRefinery()) return BWAPI::Broodwar->canMake(type); 
            else if (type.isBuilding()){
                //If it's not a refinery check if that can built at that location (building placer will adjust the build location if it is a refinery)
                //int numPylons = the.my.completed.count(BWAPI::UnitTypes::Protoss_Pylon);
                if (type.requiresPsi())
                {
                    //Check for pylon
                    //Edit building position if it requires pylon power
                    location = getClosestProtossBuildPosition({ location.x * 32, location.y * 32 }, type);
                    if (location == BWAPI::TilePositions::None || location == BWAPI::TilePositions::Invalid)
                    {
                        return false;
                    }
                }
                return BWAPI::Broodwar->canMake(type) && BWAPI::Broodwar->canBuildHere(location, type);
            }
            else return BWAPI::Broodwar->canMake(type);
        }
        else if (option < NetworkProtossOptions::Protoss_Ground_Armor)
        {
            //tech
            return BWAPI::Broodwar->canResearch(ToBWAPITech(option));
        }
        else
        {
            //upgrade
            return BWAPI::Broodwar->canUpgrade(ToBWAPIUpgrade(option));
        }
    }

    BWAPI::TilePosition NEATCommander::getClosestProtossBuildPosition(BWAPI::Position closestTo, BWAPI::UnitType buildingType) const
    {
        BWAPI::Unit targetPylon = BWAPI::Broodwar->getClosestUnit(closestTo, BWAPI::Filter::GetType == BWAPI::UnitTypes::Protoss_Pylon);
        if (targetPylon == nullptr)
        {
            return BWAPI::TilePositions::None;
        }
        BWAPI::TilePosition pylonTile = targetPylon->getTilePosition();
        BWAPI::TilePosition closestTile = { closestTo.x / 32, closestTo.y / 32 };
        int deltaX = closestTile.x - pylonTile.x;
        int deltaY = closestTile.y - pylonTile.y;
        int distanceSQ = deltaX * deltaX + deltaY * deltaY;
        double distance = std::sqrt((double)distanceSQ);
        double unitDeltaX = deltaX / distance;
        double unitDeltaY = deltaY / distance;
        int maxDistance = 12;
        BWAPI::TilePosition finalTile;

        for (int i = maxDistance; i > 0; i--)
        {
            if (BWAPI::Broodwar->canBuildHere({ (int)(unitDeltaX * i), (int)(unitDeltaY * i) }, buildingType))
            {
                finalTile = { (int)(unitDeltaX * i), (int)(unitDeltaY * i) };
                return finalTile;
            }
        }
        return BWAPI::TilePositions::None;
    }


    BWAPI::UnitType NEATCommander::ToBWAPIUnit(NetworkTerranOptions ut)
    {
        switch (ut) {
        case NetworkTerranOptions::Terran_SCV:
            return BWAPI::UnitTypes::Terran_SCV;
        case NetworkTerranOptions::Terran_Marine:
            return BWAPI::UnitTypes::Terran_Marine;
        case NetworkTerranOptions::Terran_Firebat:
            return BWAPI::UnitTypes::Terran_Firebat;
        case NetworkTerranOptions::Terran_Medic:
            return BWAPI::UnitTypes::Terran_Medic;
        case NetworkTerranOptions::Terran_Ghost:
            return BWAPI::UnitTypes::Terran_Ghost;
        case NetworkTerranOptions::Terran_Vulture:
            return BWAPI::UnitTypes::Terran_Vulture;
        case NetworkTerranOptions::Terran_Siege_Tank_Tank_Mode:
            return BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode;
        case NetworkTerranOptions::Terran_Goliath:
            return BWAPI::UnitTypes::Terran_Goliath;
        case NetworkTerranOptions::Terran_Wraith:
            return BWAPI::UnitTypes::Terran_Wraith;
        case NetworkTerranOptions::Terran_Dropship:
            return BWAPI::UnitTypes::Terran_Dropship;
        case NetworkTerranOptions::Terran_Science_Vessel:
            return BWAPI::UnitTypes::Terran_Science_Vessel;
        case NetworkTerranOptions::Terran_Valkyrie:
            return BWAPI::UnitTypes::Terran_Valkyrie;
        case NetworkTerranOptions::Terran_Battlecruiser:
            return BWAPI::UnitTypes::Terran_Battlecruiser;

        case NetworkTerranOptions::Terran_Command_Center:
            return BWAPI::UnitTypes::Terran_Command_Center;
        case NetworkTerranOptions::Terran_Comsat_Station:
            return BWAPI::UnitTypes::Terran_Comsat_Station;
        case NetworkTerranOptions::Terran_Nuclear_Silo:
            return BWAPI::UnitTypes::Terran_Nuclear_Silo;
        case NetworkTerranOptions::Terran_Supply_Depot:
            return BWAPI::UnitTypes::Terran_Supply_Depot;
        case NetworkTerranOptions::Terran_Refinery:
            return BWAPI::UnitTypes::Terran_Refinery;
        case NetworkTerranOptions::Terran_Barracks:
            return BWAPI::UnitTypes::Terran_Barracks;
        case NetworkTerranOptions::Terran_Engineering_Bay:
            return BWAPI::UnitTypes::Terran_Engineering_Bay;
        case NetworkTerranOptions::Terran_Bunker:
            return BWAPI::UnitTypes::Terran_Bunker;
        case NetworkTerranOptions::Terran_Missile_Turret:
            return BWAPI::UnitTypes::Terran_Missile_Turret;
        case NetworkTerranOptions::Terran_Academy:
            return BWAPI::UnitTypes::Terran_Academy;
        case NetworkTerranOptions::Terran_Factory:
            return BWAPI::UnitTypes::Terran_Factory;
        case NetworkTerranOptions::Terran_Machine_Shop:
            return BWAPI::UnitTypes::Terran_Machine_Shop;
        case NetworkTerranOptions::Terran_Starport:
            return BWAPI::UnitTypes::Terran_Starport;
        case NetworkTerranOptions::Terran_Control_Tower:
            return BWAPI::UnitTypes::Terran_Control_Tower;
        case NetworkTerranOptions::Terran_Science_Facility:
            return BWAPI::UnitTypes::Terran_Science_Facility;
        case NetworkTerranOptions::Terran_Covert_Ops:
            return BWAPI::UnitTypes::Terran_Covert_Ops;
        case NetworkTerranOptions::Terran_Physics_Lab:
            return BWAPI::UnitTypes::Terran_Physics_Lab;
        case NetworkTerranOptions::Terran_Armory:
            return BWAPI::UnitTypes::Terran_Armory;
        default:
            return BWAPI::UnitTypes::Terran_SCV;
        }
    }
    BWAPI::TechType NEATCommander::ToBWAPITech(NetworkTerranOptions tt)
    {
        switch (tt) {
        case NetworkTerranOptions::Cloaking_Field:
            return BWAPI::TechTypes::Cloaking_Field;
        case NetworkTerranOptions::Defensive_Matrix:
            return BWAPI::TechTypes::Defensive_Matrix;
        case NetworkTerranOptions::EMP_Shockwave:
            return BWAPI::TechTypes::EMP_Shockwave;
        case NetworkTerranOptions::Lockdown:
            return BWAPI::TechTypes::Lockdown;
        case NetworkTerranOptions::Optical_Flare:
            return BWAPI::TechTypes::Optical_Flare;
        case NetworkTerranOptions::Personnel_Cloaking:
            return BWAPI::TechTypes::Personnel_Cloaking;
        case NetworkTerranOptions::Scanner_Sweep:
            return BWAPI::TechTypes::Scanner_Sweep;
        case NetworkTerranOptions::Spider_Mines:
            return BWAPI::TechTypes::Spider_Mines;
        case NetworkTerranOptions::Stim_Packs:
            return BWAPI::TechTypes::Stim_Packs;
        case NetworkTerranOptions::Tank_Siege_Mode:
            return BWAPI::TechTypes::Tank_Siege_Mode;
        case NetworkTerranOptions::Yamato_Gun:
            return BWAPI::TechTypes::Yamato_Gun;
        case NetworkTerranOptions::NETWORK_OPTION_COUNT:
            return BWAPI::TechTypes::None;
        default:
            return BWAPI::TechTypes::None;
        }
    }
    BWAPI::UpgradeType NEATCommander::ToBWAPIUpgrade(NetworkTerranOptions tt)
    {
        switch (tt) {
        case NetworkTerranOptions::Apollo_Reactor:
            return BWAPI::UpgradeTypes::Apollo_Reactor;
        case NetworkTerranOptions::Caduceus_Reactor:
            return BWAPI::UpgradeTypes::Caduceus_Reactor;
        case NetworkTerranOptions::Charon_Boosters:
            return BWAPI::UpgradeTypes::Charon_Boosters;
        case NetworkTerranOptions::Colossus_Reactor:
            return BWAPI::UpgradeTypes::Colossus_Reactor;
        case NetworkTerranOptions::Ion_Thrusters:
            return BWAPI::UpgradeTypes::Ion_Thrusters;
        case NetworkTerranOptions::Moebius_Reactor:
            return BWAPI::UpgradeTypes::Moebius_Reactor;
        case NetworkTerranOptions::Ocular_Implants:
            return BWAPI::UpgradeTypes::Ocular_Implants;
        case NetworkTerranOptions::Terran_Infantry_Armor:
            return BWAPI::UpgradeTypes::Terran_Infantry_Armor;
        case NetworkTerranOptions::Terran_Infantry_Weapons:
            return BWAPI::UpgradeTypes::Terran_Infantry_Weapons;
        case NetworkTerranOptions::Terran_Ship_Plating:
            return BWAPI::UpgradeTypes::Terran_Ship_Plating;
        case NetworkTerranOptions::Terran_Ship_Weapons:
            return BWAPI::UpgradeTypes::Terran_Ship_Weapons;
        case NetworkTerranOptions::Terran_Vehicle_Plating:
            return BWAPI::UpgradeTypes::Terran_Vehicle_Plating;
        case NetworkTerranOptions::Terran_Vehicle_Weapons:
            return BWAPI::UpgradeTypes::Terran_Vehicle_Weapons;
        case NetworkTerranOptions::Titan_Reactor:
            return BWAPI::UpgradeTypes::Titan_Reactor;
        case NetworkTerranOptions::U_238_Shells:
            return BWAPI::UpgradeTypes::U_238_Shells;
        case NetworkTerranOptions::NETWORK_OPTION_COUNT:
            return BWAPI::UpgradeTypes::None;
        default:
            return BWAPI::UpgradeTypes::None;
        }
    }
    BWAPI::UnitType NEATCommander::ToBWAPIUnit(NetworkProtossOptions ut) {
        switch (ut) {
        case NetworkProtossOptions::Protoss_Probe:
            return BWAPI::UnitTypes::Protoss_Probe;
        case NetworkProtossOptions::Protoss_Zealot:
            return BWAPI::UnitTypes::Protoss_Zealot;
        case NetworkProtossOptions::Protoss_Dragoon:
            return BWAPI::UnitTypes::Protoss_Dragoon;
        case NetworkProtossOptions::Protoss_High_Templar:
            return BWAPI::UnitTypes::Protoss_High_Templar;
        case NetworkProtossOptions::Protoss_Archon:
            return BWAPI::UnitTypes::Protoss_Archon;
        case NetworkProtossOptions::Protoss_Dark_Templar:
            return BWAPI::UnitTypes::Protoss_Dark_Templar;
        case NetworkProtossOptions::Protoss_Dark_Archon:
            return BWAPI::UnitTypes::Protoss_Dark_Archon;
        case NetworkProtossOptions::Protoss_Reaver:
            return BWAPI::UnitTypes::Protoss_Reaver;
        case NetworkProtossOptions::Protoss_Shuttle:
            return BWAPI::UnitTypes::Protoss_Shuttle;
        case NetworkProtossOptions::Protoss_Observer:
            return BWAPI::UnitTypes::Protoss_Observer;
        case NetworkProtossOptions::Protoss_Scout:
            return BWAPI::UnitTypes::Protoss_Scout;
        case NetworkProtossOptions::Protoss_Corsair:
            return BWAPI::UnitTypes::Protoss_Corsair;
        case NetworkProtossOptions::Protoss_Arbiter:
            return BWAPI::UnitTypes::Protoss_Arbiter;
        case NetworkProtossOptions::Protoss_Carrier:
            return BWAPI::UnitTypes::Protoss_Carrier;
        case NetworkProtossOptions::Protoss_Nexus:
            return BWAPI::UnitTypes::Protoss_Nexus;
        case NetworkProtossOptions::Protoss_Pylon:
            return BWAPI::UnitTypes::Protoss_Pylon;
        case NetworkProtossOptions::Protoss_Assimilator:
            return BWAPI::UnitTypes::Protoss_Assimilator;
        case NetworkProtossOptions::Protoss_Gateway:
            return BWAPI::UnitTypes::Protoss_Gateway;
        case NetworkProtossOptions::Protoss_Forge:
            return BWAPI::UnitTypes::Protoss_Forge;
        case NetworkProtossOptions::Protoss_Photon_Cannon:
            return BWAPI::UnitTypes::Protoss_Photon_Cannon;
        case NetworkProtossOptions::Protoss_Shield_Battery:
            return BWAPI::UnitTypes::Protoss_Shield_Battery;
        case NetworkProtossOptions::Protoss_Cybernetics_Core:
            return BWAPI::UnitTypes::Protoss_Cybernetics_Core;
        case NetworkProtossOptions::Protoss_Citadel_of_Adun:
            return BWAPI::UnitTypes::Protoss_Citadel_of_Adun;
        case NetworkProtossOptions::Protoss_Templar_Archives:
            return BWAPI::UnitTypes::Protoss_Templar_Archives;
        case NetworkProtossOptions::Protoss_Robotics_Facility:
            return BWAPI::UnitTypes::Protoss_Robotics_Facility;
        case NetworkProtossOptions::Protoss_Robotics_Support_Bay:
            return BWAPI::UnitTypes::Protoss_Robotics_Support_Bay;
        case NetworkProtossOptions::Protoss_Observatory:
            return BWAPI::UnitTypes::Protoss_Observatory;
        case NetworkProtossOptions::Protoss_Stargate:
            return BWAPI::UnitTypes::Protoss_Stargate;
        case NetworkProtossOptions::Protoss_Fleet_Beacon:
            return BWAPI::UnitTypes::Protoss_Fleet_Beacon;
        case NetworkProtossOptions::Protoss_Arbiter_Tribunal:
            return BWAPI::UnitTypes::Protoss_Arbiter_Tribunal;
        case NetworkProtossOptions::NETWORK_OPTION_COUNT:
            return BWAPI::UnitTypes::None;
        default:
            return BWAPI::UnitTypes::Protoss_Probe;
        }
    }
    BWAPI::TechType NEATCommander::ToBWAPITech(NetworkProtossOptions tt)
    {
        switch (tt) {
        case NetworkProtossOptions::Psionic_Storm:
            return BWAPI::TechTypes::Psionic_Storm;
        case NetworkProtossOptions::Hallucination:
            return BWAPI::TechTypes::Hallucination;
        case NetworkProtossOptions::Recall:
            return BWAPI::TechTypes::Recall;
        case NetworkProtossOptions::Stasis_Field:
            return BWAPI::TechTypes::Stasis_Field;
        case NetworkProtossOptions::Disruption_Web:
            return BWAPI::TechTypes::Disruption_Web;
        case NetworkProtossOptions::Mind_Control:
            return BWAPI::TechTypes::Mind_Control;
        case NetworkProtossOptions::Maelstrom:
            return BWAPI::TechTypes::Maelstrom;
        case NetworkProtossOptions::NETWORK_OPTION_COUNT:
            return BWAPI::TechTypes::None;
        default:
            return BWAPI::TechTypes::Psionic_Storm;
        }
    }
    BWAPI::UpgradeType NEATCommander::ToBWAPIUpgrade(NetworkProtossOptions ut)
    {
        switch (ut) {
        case NetworkProtossOptions::Protoss_Ground_Armor:
            return BWAPI::UpgradeTypes::Protoss_Ground_Armor;
        case NetworkProtossOptions::Protoss_Air_Armor:
            return BWAPI::UpgradeTypes::Protoss_Air_Armor;
        case NetworkProtossOptions::Protoss_Ground_Weapons:
            return BWAPI::UpgradeTypes::Protoss_Ground_Weapons;
        case NetworkProtossOptions::Protoss_Air_Weapons:
            return BWAPI::UpgradeTypes::Protoss_Air_Weapons;
        case NetworkProtossOptions::Protoss_Plasma_Shields:
            return BWAPI::UpgradeTypes::Protoss_Plasma_Shields;
        case NetworkProtossOptions::Singularity_Charge:
            return BWAPI::UpgradeTypes::Singularity_Charge;
        case NetworkProtossOptions::Leg_Enhancements:
            return BWAPI::UpgradeTypes::Leg_Enhancements;
        case NetworkProtossOptions::Scarab_Damage:
            return BWAPI::UpgradeTypes::Scarab_Damage;
        case NetworkProtossOptions::Reaver_Capacity:
            return BWAPI::UpgradeTypes::Reaver_Capacity;
        case NetworkProtossOptions::Gravitic_Drive:
            return BWAPI::UpgradeTypes::Gravitic_Drive;
        case NetworkProtossOptions::Sensor_Array:
            return BWAPI::UpgradeTypes::Sensor_Array;
        case NetworkProtossOptions::Gravitic_Boosters:
            return BWAPI::UpgradeTypes::Gravitic_Boosters;
        case NetworkProtossOptions::Khaydarin_Amulet:
            return BWAPI::UpgradeTypes::Khaydarin_Amulet;
        case NetworkProtossOptions::Apial_Sensors:
            return BWAPI::UpgradeTypes::Apial_Sensors;
        case NetworkProtossOptions::Gravitic_Thrusters:
            return BWAPI::UpgradeTypes::Gravitic_Thrusters;
        case NetworkProtossOptions::Carrier_Capacity:
            return BWAPI::UpgradeTypes::Carrier_Capacity;
        case NetworkProtossOptions::Khaydarin_Core:
            return BWAPI::UpgradeTypes::Khaydarin_Core;
        case NetworkProtossOptions::Argus_Jewel:
            return BWAPI::UpgradeTypes::Argus_Jewel;
        case NetworkProtossOptions::Argus_Talisman:
            return BWAPI::UpgradeTypes::Argus_Talisman;
        case NetworkProtossOptions::NETWORK_OPTION_COUNT:
            return BWAPI::UpgradeTypes::None;
        default:
            return BWAPI::UpgradeTypes::Protoss_Ground_Armor;
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