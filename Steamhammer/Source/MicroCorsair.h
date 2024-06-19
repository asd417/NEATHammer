#pragma once

#include "MicroManager.h"

namespace UAlbertaBot
{
class MicroManager;

class MicroCorsair : public MicroManager
{
public:

	MicroCorsair();
	//Movement handle by attacking spellcaster units
	void executeMicro(const BWAPI::Unitset& targets, const UnitCluster& cluster);
	void assignTargets(const BWAPI::Unitset & airUnits, const BWAPI::Unitset & targets);

	int getAttackPriority(BWAPI::Unit rangedUnit, BWAPI::Unit target);
	BWAPI::Unit getTarget(BWAPI::Unit rangedUnit, const BWAPI::Unitset & targets);
	
	int disruptionScore(BWAPI::Unit u) const;
	bool aboutToDie(const BWAPI::Unit u) const;

	BWAPI::Unitset getCaster(const UnitCluster& cluster) const;
	bool maybeSpell(BWAPI::Unit unit);
	void updateSpell(const UnitCluster& cluster);
};
}