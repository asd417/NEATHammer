#include "MicroSpell.h"

using namespace UAlbertaBot;                                                

MicroSpell::MicroSpell(
		int _radius, 
		BWAPI::TechType _tech,
		BWAPI::UnitType _casterType,
        std::function<BWAPI::Unitset& ()> _getUnits,
		std::function<int(BWAPI::Unit)> _scoreFunction,
		std::function<bool(const BWAPI::Unit)> _aboutToDie,
        std::function<void(const BWAPI::Unitset&)> _updateCasters, 
        std::function<bool(BWAPI::Unit, CasterSpell)> _isReadyToCastOtherThan) :
	techType{ _tech }, 
	casterType{ _casterType }, 
    getUnits{_getUnits},
	scoreFunction{ _scoreFunction },
	aboutToDie{ _aboutToDie }, 
    updateCasters{_updateCasters}, 
    isReadyToCastOtherThan{ _isReadyToCastOtherThan }
{
	energyCost = techType.energyCost();
	spellRange = techType.getWeapon().maxRange();
	spellRadius = _radius;
}

BWAPI::Unitset MicroSpell::getCasterUnits(const UnitCluster& cluster) const
{
    BWAPI::Unitset set;
    for (BWAPI::Unit unit : getUnits())
    {
        if (unit->getType() == casterType &&
            cluster.units.contains(unit))
        {
            set.insert(unit);
        }
    }
    return set;
}

// Cast spell if possible and useful.
void MicroSpell::updateSpell(const UnitCluster& cluster)
{
    BWAPI::Unitset templars = getCasterUnits(cluster);
    if (templars.empty())
    {
        return;
    }
    updateCasters(templars);

    for (BWAPI::Unit templar : templars)
    {
        if (templar->getEnergy() >= 75 &&
            BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Psionic_Storm) &&
            templar->canUseTech(BWAPI::TechTypes::Psionic_Storm, templar->getPosition()) &&
            !isReadyToCastOtherThan(templar, ))
        {
            if (!maybeStorm(templar) && isReadyToCast(templar))
            {
                clearReadyToCast(templar);
                return;     // only one defiler may cast per call, to reduce duplication
            }
        }
    }
}
