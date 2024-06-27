#pragma once

#include "Common.h"
#include "InformationManager.h"
#include "WorkerManager.h"
#include "BuildOrder.h"
#include "BuildOrderQueue.h"

namespace UAlbertaBot
{
typedef std::pair<MacroAct, size_t> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;

struct Strategy
{
    std::string _name;
    BWAPI::Race _race;
    std::string _openingGroup;
    BuildOrder  _buildOrder;

    Strategy()
        : _name("None")
        , _race(BWAPI::Races::None)
        , _openingGroup("")
    {
    }

    Strategy(const std::string & name, const BWAPI::Race & race, const std::string & openingGroup, const BuildOrder & buildOrder)
        : _name(name)
        , _race(race)
        , _openingGroup(openingGroup)
        , _buildOrder(buildOrder)
    {
    }
};

class StrategyManager 
{
    StrategyManager();

    //Left as example
    const	MetaPairVector		    getTerranBuildOrderGoal();

    bool							canPlanBuildOrderNow() const;
public:
    static	StrategyManager &	    Instance();
    void							queryNetworkEvaluation();
};

}