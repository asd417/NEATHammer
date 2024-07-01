#include "NEATCommander.h"
#include "Config.h"
#include "MacroAct.h"
#include "The.h"
#include "BuildingManager.h"
#include "CombatCommander.h"
#include "Logger.h"
#include <windows.h> 
#include <fstream>
#include <sstream>

/// <summary>
/// Check https://gamefaqs.gamespot.com/pc/25418-starcraft/faqs/2473 for some infos.
/// These arent always accurate though
/// </summary>

namespace UAlbertaBot
{
    void logToStream(std::stringstream& out, const char* fmt, ...)
    {
        va_list arg;

        va_start(arg, fmt);
        //vfprintf(log_file, fmt, arg);
        char buff[256];
        vsnprintf_s(buff, 256, fmt, arg);
        va_end(arg);

        out << buff;
    }
    NEATCommander& NEATCommander::Instance()
    {
        static NEATCommander instance;
        
        return instance;
    }
    
    NEATCommander::NEATCommander() : gen{ rd() }, dis{ 0.0f, 1.0f } {
        initialized = false;
        mapWidth = BWAPI::Broodwar->mapWidth();
        mapHeight = BWAPI::Broodwar->mapHeight();
        //We'll look 32x32 section
        // try dividing
        //for example 256x256
        int dividedWidth = mapWidth / 32; //get 8
        int dividedHeight = mapHeight / 32;

        //Game always start with 1 command center. 
        fitness = -Config::NEAT::BuildingScore;

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
        if (!network) return; // Only score fitness if network is initialized
        if (!network->isValid()) return;
        //UAB_ASSERT(fitness > 0.0f, "Negative Fitness: %s", std::to_string(fitness).c_str());
        fitness += add;
        fitness = fitness >= 0.0f ? fitness : 0.0f;
    }

    void NEATCommander::InitializeNetwork()
    {
        json networkJson;
        int id = -1;
        if (!Config::NEAT::LoadNetworkFromJSON)
        {
            bool retry = true;
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
                    networkType = r[0]["NetworkType"];
                    networkJson = json::parse(std::string(r[0]["Network"]));
                }
                catch (std::exception e) {
                    //std::cout << "Error while retrieving new genome to evaluate: " << e.what() << "\n";
                    //std::cout << "\tWaiting for 3000 miliseconds\n";
                    //UAB_ASSERT(false, "Error while retrieving new genome to evaluate: %s", e.what());
                    r.clear();
                    id = -1;
                    Sleep(Config::NEAT::RetryTimer * 1000);
                }
            }
        }
        else 
        {
            std::ifstream file(Config::NEAT::NetworkJSON);
            if (!file.is_open()) {
                throw std::exception("Could not open the file");
            }
            std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            // Close the file
            file.close();

            // Parse the JSON data
            try {
                networkJson = json::parse(file_contents);
            }
            catch (json::parse_error& e) {
                throw std::exception("Error while parsing network JSON");
            }
            //Should automate this... But it looks like we'll stick to NEAT with simple feedforward so this should be fine
            networkType = NetworkType::FEEDFORWARD;
        }
        
        try {
            if (!network) delete network;
            if (networkType == NetworkType::FEEDFORWARD)
            {
                network = new RecurrentNetwork(networkJson["input_keys"], networkJson["output_keys"]);
            }
            else if (networkType == NetworkType::RECURRENT)
            {
                network = new FeedForwardNetwork(networkJson["input_keys"], networkJson["output_keys"]);
            }
            else
            {
                throw std::exception("INVALID NETWORK TYPE. CHOOSE FEEDFORWARD OR RECURRENT");
            }
            for (const json& ne : networkJson["node_evals"]) {
                network->AddNodeEval(ne);
            }
            if (network->IsNodeEvalEmpty()) throw std::exception("Faulty Gene");
            network->FinishInitializing();
            constexpr int outputVectorTotal = (int)NetworkTerranOptions::NETWORK_OPTION_COUNT + (int)MacroCommandType::QueueBarrier + 2;
            if (outputVectorTotal != network->getOutputCount()) throw std::exception("Output Vector Mismatch");
        }
        catch (std::exception e) {
            std::cout << "Error creating Network Structure: " << e.what() << "\n";
            BWAPI::Broodwar->leaveGame();
        }
        if (Config::NEAT::LogOutputVector)
        {
            //Mark new network
            std::stringstream logStream;
            for (int i = 0; i < 60; i++)logToStream(logStream, "%4.5f\t", 1.0f);
            logToStream(logStream, "\n");
            for (int i = 0; i < 60; i++)logToStream(logStream, "%4.5f\t", 1.0f);
            logToStream(logStream, "\n");
            for (int i = 0; i < 60; i++)logToStream(logStream, "%4.5f\t", 1.0f);
            logToStream(logStream, "\n");
            for (int i = 0; i < 60; i++)logToStream(logStream, "%4.5f\t", 1.0f);
            logToStream(logStream, "\n");
            for (int i = 0; i < 60; i++)logToStream(logStream, "%4.5f\t", 1.0f);
            logToStream(logStream, "\n");
            logToStream(logStream, "\n");
            std::ofstream logFStream;
            logFStream.open(Config::NEAT::OutputLogFileName, std::ofstream::app);
            logFStream << logStream.rdbuf();
            logFStream.flush();
            logFStream.close();
        }
        genomeID = id;
        initialized = true;
        
    }
    void NEATCommander::sendFitnessToTrainServer()
    {
        if (Config::NEAT::Train && !Config::NEAT::LoadNetworkFromJSON)
        {
            if (!network->isValid()) fitness = -1;
            DBKeySpace dbkeys{};
            dbkeys.push_back("Fitness");
            dbkeys.push_back("isWinner");

            DBConnector db{ Config::NEAT::TrainingServerIP.c_str(), dbkeys };
            
            try {
                db.addToData("Fitness", fitness);
                db.addToData("isWinner", _winner);

                std::string url = "/genome/" + std::to_string(genomeID) + "/fitness";
                std::cout << url << "\n";
                db.sendData(url.c_str());
            }
            catch (std::exception e) {
                std::cout << "Error while sending fitness value to the training server: " << e.what() << "\n";
            }
            delete network;
        }
    }
    int NEATCommander::getWorkerCount(BWAPI::Unitset& allMyUnits)
    {
        int r = 0;
        for (auto u : allMyUnits) {
            if (u->getType().isWorker())
            {
                r++;
            }
        }
        return r;
    }

    //Fitness functions
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
                scoreFitness(Config::NEAT::BuildingScore);
            }
            else if(type.isWorker())
            {
                //Some Motivation to build army
                //scoreFitness(type.buildScore());
            }
            else
            {
                //Huge Motivation to build army
                scoreFitness(Config::NEAT::UnitCompleteScore);
            }
        }
        
    }
    void NEATCommander::onUnitDestroy(BWAPI::Unit unit)
    {
        //Enemy killed. update all kill counts
        //This could technically count friendly kills.
        if (unit->getPlayer() != the.self())
        {
            auto all = BWAPI::Broodwar->getAllUnits();
            for (auto& u : all)
            {
                if (!u->getType().isWorker() && u->getPlayer() == the.self())
                {
                    //Make pointer value into int
                    killMap[(int)u] = u->getKillCount();
                }
            }
        }
        //To prevent bot from learning to score high fitness by dropping nuke on friendly unit,
        //add a small fitness penalty for allied units died.
        if (unit->getPlayer() == the.self() && !unit->getType().isBuilding() && !unit->getType().isWorker())
        {
            //scoreFitness(-Config::NEAT::ArmyKillScore / 2);
        }
    }
    void NEATCommander::onUnitHide(BWAPI::Unit unit)
    {
    }
    void NEATCommander::onUnitShow(BWAPI::Unit unit)
    {
        //Used detector to reveal invisible units
        //if (unit->getPlayer() == the.enemy()) scoreFitness(Config::NEAT::EnemyShowScore);
    }

    void NEATCommander::onEnd(bool isWinner)
    {
        int globalKills = 0;
        for (auto it = killMap.begin(); it != killMap.end(); it++)
        {
            globalKills += it->second;
        }
        scoreFitness(globalKills * Config::NEAT::ArmyKillScore);
        _winner = isWinner;
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
            BWAPI::Broodwar->drawTextScreen(x, y, "%2.8f", bo);
            y += 12;
            c++;
            if (c == 20)
            {
                x += 120;
                y = startY;
            }
            if (c == 40)
            {
                x += 120;
                y = startY;
            }
        }

        for (const double& bo : builderOutputs)
        {
            BWAPI::Broodwar->drawTextScreen(x, y, "%2.8f", bo);
            y += 12;
            c++;
            if (c == 20)
            {
                x += 120;
                y = startY;
            }
            if (c == 40)
            {
                x += 120;
                y = startY;
            }
        }
        
        for (const double& bo : macroCommandTypeOutputs)
        {
            BWAPI::Broodwar->drawTextScreen(x, y, "%2.8f", bo);
            y += 12;
            c++;
            if (c == 20)
            {
                x += 120;
                y = startY;
            }
            if (c == 40)
            {
                x += 120;
                y = startY;
            }
        }
        BWAPI::Broodwar->drawTextScreen(x, y, "%2.8f", tilePosX);
        y += 12;
        BWAPI::Broodwar->drawTextScreen(x, y, "%2.8f", tilePosY);

    }
    void NEATCommander::setTimeManager(TimerManager* t)
    {
        timer = t;
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
        
        int mSupply = BWAPI::Broodwar->self()->supplyTotal();
        int cSupply = BWAPI::Broodwar->self()->supplyUsed();

        //What if we remove the gas and mineral from fitness score?
        //double deltaMineral = min > lastMineral ? min - lastMineral : 0;
        //double deltaGas = gas > lastGas ? gas - lastGas : 0;
        //lastMineral = min;
        //lastGas = gas;
        //scoreFitness(deltaMineral / Config::NEAT::FitnessScore_Gas_Divider);
        //scoreFitness(deltaGas / Config::NEAT::FitnessScore_Gas_Divider);

        int ctime = frame;
        int sectionCoordW = sectionsCoords[curSection][0];
        int sectionCoordH = sectionsCoords[curSection][1];
        BWAPI::Unitset myUnits = the.self()->getUnits();
        int workerCount = getWorkerCount(myUnits);
        int enemyRace = BWAPI::Broodwar->enemy()->getRace();
        switch (enemyRace)
        {
        case BWAPI::Races::Zerg:
            enemyRace = 0;
            break;
        case BWAPI::Races::Terran:
            enemyRace = 1;
            break;
        case BWAPI::Races::Protoss:
            enemyRace = 2;
            break;
        case BWAPI::Races::Unknown:
            enemyRace = 3;
            break;
        }

        //getVisibleMap(curSection);
        getVisibleMapSimple(curSection);

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

                        //inputVector.push_back(dis(gen));
                    }
                    else
                    {
                        //yi >= 16
                        //Friendly building map encoding
                        inputVector.push_back(friendlyMapBuildingData[xi][yi - 16]);

                        //inputVector.push_back(dis(gen));
                    }
                }
                else
                {
                    if (yi < 16)
                    {
                        //enemy map encoding
                        inputVector.push_back(enemyMapData[xi - 16][yi]);

                        //inputVector.push_back(dis(gen));
                    }
                    else
                    {
                        //enemy building map encoding
                        inputVector.push_back(enemyMapBuildingData[xi - 16][yi - 16]);

                        //inputVector.push_back(dis(gen));
                    }
                }
            }
        }
        inputVector[0] = double(mSupply) / double(200);
        inputVector[1] = double(cSupply) / double(200);
        inputVector[2] = double(min)/ double(10000);
        inputVector[3] = double(gas)/ double(10000);
        inputVector[4] = double(ctime)/double(171432);
        inputVector[5] = double(ctime) / double(171432);
        inputVector[6] = double(sectionCoordW)/double(mapWidth);
        inputVector[7] = double(sectionCoordH)/double(mapHeight);
        inputVector[10] = double(workerCount)/double(200);
        inputVector[11] = double(enemyRace)/double(3);

        //Log input vector
        if (Config::NEAT::LogInputVector)
        {
            //Draw all inputs
            std::stringstream logStream;
            logToStream(logStream, "\n");
            for (int xi = 0; xi < 32; xi++)
            {
                for (int yi = 0; yi < 32; yi++)
                {
                    logToStream(logStream, "%4.5f\t", inputVector[xi*32 + yi]);
                }
                logToStream(logStream, "\n");
            }
            logToStream(logStream, "\n");

            std::ofstream logFStream;
            logFStream.open(Config::NEAT::InputLogFileName.c_str(), std::ofstream::app);
            logFStream << logStream.rdbuf();
            logFStream.flush();
            logFStream.close();

        }

        network->Activate(inputVector);
        //Read output nodes 
        // 
        // Network can output:
        //  Type of thing to build,
        //  MacroCommandType, 
        //  Tile position used by some of the macro command types and building build position
        // 
        //BuilderOutput and MacroCommandTypeOutput will be chosen through strongest node output 

        for (int i = 0; i < (int)NetworkTerranOptions::NETWORK_OPTION_COUNT; i++)
        {
            builderOutputs[i] += network->getOutputVector()[i];
        }
        for (int i = 0; i < (int)MacroCommandType::QueueBarrier; i++)
        {
            macroCommandTypeOutputs[i] += network->getOutputVector()[(int)NetworkTerranOptions::NETWORK_OPTION_COUNT + i];
        }
        constexpr int outputVectorOffset = (int)NetworkTerranOptions::NETWORK_OPTION_COUNT + (int)MacroCommandType::QueueBarrier;
        tilePosX += network->getOutputVector()[outputVectorOffset];
        tilePosY += network->getOutputVector()[outputVectorOffset + 1]; //70 output nodes
        
        //We scanned through the whole map
        curSection++;
        curSection = curSection % maxSections;
        //Looked through all sections. Make command
        if (Config::NEAT::PrintNetworkOutput) {
            drawDebug(180, 10);
        }
        if (curSection == 0)
        {
            //This part apparently can cause a lot of lag...
            tilePosX = tilePosX / double(maxSections);
            tilePosY = tilePosY / double(maxSections);
            int posx = int(tilePosX * double(mapWidth));
            int posy = int(tilePosY * double(mapHeight));
            posx = std::clamp(posx, 1, mapWidth-4);
            posy = std::clamp(posy, 1, mapHeight-4);
            BWAPI::TilePosition buildPos = { posx,posy };

            //Log output
            if (Config::NEAT::LogOutputVector)
            {
                //Draw all outputs
                std::stringstream logStream;
                for (int i = 0; i < (int)NetworkTerranOptions::NETWORK_OPTION_COUNT; i++)logToStream(logStream, "%4.5f\t", builderOutputs[i] / double(maxSections));
                for (int i = 0; i < 60 - (int)NetworkTerranOptions::NETWORK_OPTION_COUNT; i++)logToStream(logStream, "%4.5f\t", 0.0f);
                logToStream(logStream, "\n");
                for (int i = 0; i < 60; i++)logToStream(logStream, "%4.5f\t", 0.0f);
                logToStream(logStream, "\n");
                for (int i = 0; i < (int)MacroCommandType::QueueBarrier; i++)logToStream(logStream, "%4.5f\t", macroCommandTypeOutputs[i] / double(maxSections));
                for (int i = 0; i < 60 - (int)MacroCommandType::QueueBarrier; i++)logToStream(logStream, "%4.5f\t", 0.0f);
                logToStream(logStream, "\n");
                for (int i = 0; i < 60; i++)logToStream(logStream, "%4.5f\t", 0.0f);
                logToStream(logStream, "\n");
                logToStream(logStream, "%4.5f\t", tilePosX);
                logToStream(logStream, "%4.5f\t", tilePosY);
                for (int i = 0; i < 58; i++)logToStream(logStream, "%4.5f\t", 0.0f);
                logToStream(logStream, "\n");
                logToStream(logStream, "\n");
                std::ofstream logFStream;
                logFStream.open(Config::NEAT::OutputLogFileName.c_str(), std::ofstream::app);
                logFStream << logStream.rdbuf();
                logFStream.flush();
                logFStream.close();
            }

            int maxBuildChoice = 0;
            double maxBuildScore = 0.0f;
            if (timer != nullptr)
            {
                timer->startTimer(TimerManager::net_ev1);
            }
            for (int i = 0; i < (int)NetworkTerranOptions::NETWORK_OPTION_COUNT; i++)
            {
                if (builderOutputs[i] > maxBuildScore && canBuild((NetworkTerranOptions)i, buildPos, min, gas))
                {
                    maxBuildChoice = i;
                    maxBuildScore = builderOutputs[i];
                }
            }
            if (timer != nullptr)
            {
                timer->stopTimer(TimerManager::net_ev1);
            }
            if (timer != nullptr)
            {
                timer->startTimer(TimerManager::net_ev2);
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
            if (timer != nullptr)
            {
                timer->stopTimer(TimerManager::net_ev2);
            }
            if (timer != nullptr)
            {
                timer->startTimer(TimerManager::net_ev3);
            }
            MacroAct ma;
            bool failed = false;
            // compare score between build option score and macro command type score
            if (maxBuildScore >= maxCommandScore)
            {
                //BuildOutput has stronger signal than MacroCommandUnitType signal
                if ((NetworkTerranOptions)maxBuildChoice < NetworkTerranOptions::Cloaking_Field) //unit or building
                {
                    BWAPI::UnitType t = ToBWAPIUnit((NetworkTerranOptions)maxBuildChoice);
                    if (t.isBuilding())
                    {
                        //UAB_ASSERT(t.isBuilding() && t == BWAPI::UnitTypes::Terran_Supply_Depot || t == BWAPI::UnitTypes::Terran_Command_Center || t == BWAPI::UnitTypes::Terran_Refinery, "Tried to build %s", t.c_str());
                        Building b = Building(t, buildPos);
                        //found may be None
                        //BWAPI::TilePosition found = the.placer.getBuildLocationNear(b, 2);
                        BWAPI::TilePosition found = BuildingManager::Instance().getBuildingLocation(b);
                        if (!found.isValid()) failed = true;
                        //UAB_ASSERT(!buildPos.isValid(), "Trying to build at %d, %d", buildPos.x, buildPos.y);
                        if (failed && Config::NEAT::LogOutputDecision)
                        {
                            Logger::LogAppendToFile(Config::NEAT::DecisionLogFileName.c_str(), "Tried to build %s at %d, %d and failed\n", t.c_str(), buildPos.x, buildPos.y);
                        }
                        else
                        {
                            Logger::LogAppendToFile(Config::NEAT::DecisionLogFileName.c_str(), "Building %s at %d, %d\n", t.c_str(), found.x, found.y);
                        }
                        
                        ma = MacroAct(ToBWAPIUnit((NetworkTerranOptions)maxBuildChoice), buildPos);
                    }
                    else
                    {
                        ma = MacroAct(ToBWAPIUnit((NetworkTerranOptions)maxBuildChoice));
                    }
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
            if (timer != nullptr)
            {
                timer->stopTimer(TimerManager::net_ev3);
            }
            
            //Always keep just 1 in the build queue
            if (!failed)
            {
                if (_actions.size() > 0) _actions[0] = ma;
                else _actions.push_back(ma);
            }
            //std::cout << "Network Evaluated " << std::to_string(_actions.size()) << " actions\n";
            //Reset output vector
            builderOutputs.fill(0.0f);
            macroCommandTypeOutputs.fill(0.0f);
            tilePosX = 0.0f; //0~256
            tilePosY = 0.0f; //0~256
            //when using recurrent network, resets the network
            network->Reset();
        }
    }

    void NEATCommander::getVisibleMapSimple(int sectionNum)
    {
        int a = sectionNum;
        if (sectionNum >= maxSections) throw std::overflow_error("sectionNum is bigger than maxSections");
        int startW = sectionsCoords[sectionNum][0];
        int startH = sectionsCoords[sectionNum][1];
        //Half resolution it
        for (int xi = 0; xi < 16; xi++)
        {
            for (int yi = 0; yi < 16; yi++)
            {
                int x = xi * 2;
                int y = yi * 2;
                BWAPI::TilePosition tp = { startW + x,startH + y};

                //Get center of 2x2 tile
                BWAPI::WalkPosition wp = { (startW + x + 1) * 4,(startH + y + 1) * 4}; //WalkPosition is 8 pixels big

                if (!BWAPI::Broodwar->isVisible(tp)) {
                    friendlyMapData[xi][yi]           = NEAT_TileType_Simple::sFOG;
                    friendlyMapBuildingData[xi][yi]   = NEAT_TileType_Simple::sFOG;
                    enemyMapData[xi][yi]              = NEAT_TileType_Simple::sFOG;
                    enemyMapBuildingData[xi][yi]      = NEAT_TileType_Simple::sFOG;
                }
                else
                {
                    //BWAPI::UnitFilter uf = BWAPI::UnitFilter(the.self());
                    BWAPI::Unitset units = BWAPI::Broodwar->getUnitsInRectangle({ (startW + x) * 32, (startH + y) * 32 }, { (startW + x + 2) * 32 - 1, (startH + y + 2) * 32 - 1 });
     
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

                    for (const auto& u : units)
                    {
                        const auto type = u->getType();
                        if (
                            type == BWAPI::UnitTypes::Resource_Mineral_Field ||
                            type == BWAPI::UnitTypes::Resource_Mineral_Field_Type_2 ||
                            type == BWAPI::UnitTypes::Resource_Mineral_Field_Type_3)
                        {
                            fmb = NEAT_TileType_Simple::sMINERAL;
                            fm = NEAT_TileType_Simple::sMINERAL;
                            emb = NEAT_TileType_Simple::sMINERAL;
                            em = NEAT_TileType_Simple::sMINERAL;
                        }
                        else if(type == BWAPI::UnitTypes::Resource_Vespene_Geyser)
                        {
                            fmb =   NEAT_TileType_Simple::sGAS;
                            fm  =   NEAT_TileType_Simple::sGAS;
                            emb =   NEAT_TileType_Simple::sGAS;
                            em  =   NEAT_TileType_Simple::sGAS;
                        }
                        else if (u->getPlayer() == the.self())
                        {
                            if (type.isBuilding())
                            {
                                fmb = NEAT_TileType_Simple::sUNIT;
                            }
                            else
                            {
                                fm = NEAT_TileType_Simple::sUNIT;
                            }
                        }
                        else
                        {
                            if (type.isBuilding())
                            {
                                emb = NEAT_TileType_Simple::sUNIT;
                            }
                            else
                            {
                                em = NEAT_TileType_Simple::sUNIT;
                            }
                        }
                    }

                    friendlyMapData[xi][yi] = double(fm) / double(NEAT_TileType_Simple::sMAX);
                    friendlyMapBuildingData[xi][yi] = double(fmb) / double(NEAT_TileType_Simple::sMAX);
                    enemyMapData[xi][yi] = double(em) / double(NEAT_TileType_Simple::sMAX);
                    enemyMapBuildingData[xi][yi] = double(emb) / double(NEAT_TileType_Simple::sMAX);
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

        BWAPI::Unitset resources = BWAPI::Broodwar->getUnitsInRectangle(
            { startW * 32, startH * 32 },
            { (startW + 32) * 32 - 1, (startH + 32) * 32 - 1 },
            BWAPI::Filter::GetType == BWAPI::UnitTypes::Resource_Mineral_Field ||
            BWAPI::Filter::GetType == BWAPI::UnitTypes::Resource_Mineral_Field_Type_2 ||
            BWAPI::Filter::GetType == BWAPI::UnitTypes::Resource_Mineral_Field_Type_3 ||
            BWAPI::Filter::GetType == BWAPI::UnitTypes::Resource_Vespene_Geyser);

        for (auto& r : resources)
        {
            BWAPI::TilePosition tp = r->getTilePosition();
            NEAT_TileType tt;
            switch (r->getType())
            {
            case BWAPI::UnitTypes::Resource_Mineral_Field:
                tt = NEAT_TileType::MINERAL;
            case BWAPI::UnitTypes::Resource_Mineral_Field_Type_2:
                tt = NEAT_TileType::MINERAL;
            case BWAPI::UnitTypes::Resource_Mineral_Field_Type_3:
                tt = NEAT_TileType::MINERAL;
            case BWAPI::UnitTypes::Resource_Vespene_Geyser:
                tt = NEAT_TileType::GAS;

            }
            friendlyMapData[(tp.x - startW) / 2][(tp.y - startH) / 2] = tt;
            friendlyMapBuildingData[(tp.x - startW) / 2][(tp.y - startH) / 2] = tt;
            enemyMapData[(tp.x - startW) / 2][(tp.y - startH) / 2] = tt;
            enemyMapBuildingData[(tp.x - startW) / 2][(tp.y - startH) / 2] = tt;
        }

        BWAPI::BestFilter highKillScore = BWAPI::Highest<BWAPI::Unit>(BWAPI::Filter::DestroyScore);
        BWAPI::BestFilter highBuildScore = BWAPI::Highest<BWAPI::Unit>(BWAPI::Filter::BuildScore);
        //Half resolution it
        for (int xi = 0; xi < 16; xi++)
        {
            for (int yi = 0; yi < 16; yi++)
            {
                int x = xi * 2;
                int y = yi * 2;
                BWAPI::TilePosition tp = { startW + x,startH + y };

                //Get center of 2x2 tile
                BWAPI::WalkPosition wp = { (startW + x + 1) * 4,(startH + y + 1) * 4 }; //WalkPosition is 8 pixels big

                if (!BWAPI::Broodwar->isVisible(tp)) {
                    friendlyMapData[xi][yi] = NEAT_TileType::FOG;
                    friendlyMapBuildingData[xi][yi] = NEAT_TileType::FOG;
                    enemyMapData[xi][yi] = NEAT_TileType::FOG;
                    enemyMapBuildingData[xi][yi] = NEAT_TileType::FOG;
                }
                else
                {
                    //BWAPI::UnitFilter uf = BWAPI::UnitFilter(the.self());
                    BWAPI::Unit fu = BWAPI::Broodwar->getBestUnit(highBuildScore, BWAPI::Filter::IsAlly && !BWAPI::Filter::IsBuilding, { (startW + x + 1) * 32, (startH + y + 1) * 32 }, 45);
                    BWAPI::Unit fb = BWAPI::Broodwar->getBestUnit(highBuildScore, BWAPI::Filter::IsAlly && BWAPI::Filter::IsBuilding, { (startW + x + 1) * 32, (startH + y + 1) * 32 }, 45);
                    
                    BWAPI::Unit eu = BWAPI::Broodwar->getBestUnit(highKillScore, BWAPI::Filter::IsEnemy && !BWAPI::Filter::IsBuilding, { (startW + x + 1) * 32, (startH + y + 1) * 32 }, 45);
                    BWAPI::Unit eb = BWAPI::Broodwar->getBestUnit(highKillScore, BWAPI::Filter::IsEnemy && BWAPI::Filter::IsBuilding, { (startW + x + 1) * 32, (startH + y + 1) * 32 }, 45);

                    NEAT_TileType fm     = NEAT_TileType::NOTWALKABLE;
                    NEAT_TileType fmb    = NEAT_TileType::NOTWALKABLE;
                    NEAT_TileType em     = NEAT_TileType::NOTWALKABLE;
                    NEAT_TileType emb    = NEAT_TileType::NOTWALKABLE;

                    if (BWAPI::Broodwar->isWalkable(wp)) {
                        fm  = NEAT_TileType::WALKABLE;
                        fmb = NEAT_TileType::WALKABLE;
                        em  = NEAT_TileType::WALKABLE;
                        emb = NEAT_TileType::WALKABLE;
                    }
                    if (fu) fm = getTileType(fu->getType());
                    if (fb) fmb = getTileType(fb->getType());
                    if (eu) em = getTileType(eu->getType());
                    if (eb) emb = getTileType(eb->getType());
                    friendlyMapData[xi][yi] = double(fm) / double(NEAT_TileType::MAX);
                    friendlyMapBuildingData[xi][yi] = double(fmb) / double(NEAT_TileType::MAX);

                    enemyMapData[xi][yi] = double(em) / double(NEAT_TileType::MAX);
                    enemyMapBuildingData[xi][yi] = double(emb) / double(NEAT_TileType::MAX);
                }
            }
        }

    }
    
    bool NEATCommander::canMacro(MacroCommandType command)
    {
        if (command == MacroCommandType::Aggressive)
        {
            return !CombatCommander::Instance().getAggression();
        }
        if (command == MacroCommandType::Defensive)
        {
            return CombatCommander::Instance().getAggression();
        }
        if (command == MacroCommandType::StartGas || command == MacroCommandType::StopGas)
        {
            return the.my.completed.count(BWAPI::UnitTypes::Terran_Refinery) > 1;
        }
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
        /*if (command == MacroCommandType::NUKE)
        {
            return the.my.completed.count(BWAPI::UnitTypes::Terran_Nuclear_Missile) > 0;
        }*/
        return true;
    }

    bool NEATCommander::canBuild(NetworkTerranOptions option, BWAPI::TilePosition& location, int mineral, int gas)
    {
        
        if (option < NetworkTerranOptions::Cloaking_Field)
        {
            //unit or building
            BWAPI::UnitType type = ToBWAPIUnit(option);
            
            if (type.isRefinery()) {
                auto tp = the.placer.getRefineryPosition();
                return tp != BWAPI::TilePositions::None;
            }
            if (Config::NEAT::LogOutputDecision)
            {
                Logger::LogAppendToFile(Config::NEAT::DecisionLogFileName.c_str(), "Frame %d: %s, Current mineral: %d, gas: %d\n", frame, type.c_str(), mineral, gas);
            }
            return type.mineralPrice() < mineral && type.gasPrice() < gas && 
                BWAPI::Broodwar->canMake(type) && 
                type.supplyRequired() + BWAPI::Broodwar->self()->supplyUsed() <= BWAPI::Broodwar->self()->supplyTotal();
        }
        else if (option < NetworkTerranOptions::Apollo_Reactor)
        {
            //tech
            BWAPI::TechType t = ToBWAPITech(option);
            return t.mineralPrice() < mineral && t.gasPrice() < gas && BWAPI::Broodwar->canResearch(t);
            
        }
        else
        {
            //upgrade
            BWAPI::UpgradeType u = ToBWAPIUpgrade(option);

            return u.mineralPrice() < mineral && u.gasPrice() < gas && BWAPI::Broodwar->canUpgrade(u);
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
            throw std::exception("AI evaluated invalid unit type");
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
            return NEAT_TileType::NOTWALKABLE;
        }
        
    }
}