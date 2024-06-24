#pragma once

#include "MacroCommand.h"

namespace UAlbertaBot
{

//namespace MacroActs
//{
//    enum {Unit, Tech, Upgrade, Command, Default};
//}

enum MacroActs {
    Unit,
    Tech,
    Upgrade,
    Command,
    Default
};

class MacroAct 
{
    //MacroActs::enum
    //size_t				_type;
    MacroActs           _type;

    BWAPI::UnitType		_unitType;
    BWAPI::TechType		_techType;
    BWAPI::UpgradeType	_upgradeType;
    MacroCommand		_macroCommandType;
    BWAPI::Unit         _parent;

    BWAPI::TilePosition _tileLocation;      // Used widely by NEATCommander

    

    void                initializeUnitTypesByName();
    BWAPI::UnitType     getUnitTypeFromString(const std::string & s) const;
    
public:
    double confidence;
    MacroAct();
    MacroAct(BWAPI::UnitType t);
    MacroAct(BWAPI::UnitType t, BWAPI::Unit parent);

    MacroAct(BWAPI::UnitType t, BWAPI::TilePosition tile);
    MacroAct(BWAPI::TechType t);
    MacroAct(BWAPI::UpgradeType t);
    MacroAct(MacroCommand t, const BWAPI::TilePosition& tile);


    MacroAct(MacroCommandType t);
    MacroAct(MacroCommandType t, int amount);
    MacroAct(MacroCommandType t, BWAPI::UnitType type);

    bool    isUnit()			const;
    bool	isWorker()			const;
    bool    isCombatUnit()      const;
    bool    isTech()			const;
    bool    isUpgrade()			const;
    bool    isCommand()			const;
    bool    isBuilding()		const;
    bool	isAddon()			const;
    bool	isMorphedBuilding()	const;
    bool    isRefinery()		const;
    bool	isSupply()			const;
    bool    isGasSteal()        const;
    
    size_t type() const;

    BWAPI::UnitType getUnitType() const;
    BWAPI::TechType getTechType() const;
    BWAPI::UpgradeType getUpgradeType() const;
    MacroCommand getCommandType() const;
    BWAPI::TilePosition getTileLocation() const;

    int supplyRequired() const;
    int mineralPrice()   const;
    int gasPrice()       const;

    BWAPI::UnitType whatBuilds() const;
    std::string getName() const;

    bool isProducer(BWAPI::Unit unit) const;
    void getCandidateProducers(std::vector<BWAPI::Unit> & candidates) const;
    bool hasEventualProducer() const;
    bool hasPotentialProducer() const;
    bool hasTech() const;

    bool canProduce(BWAPI::Unit producer) const;
    void produce(BWAPI::Unit producer) const;

    std::string toString();
};
}