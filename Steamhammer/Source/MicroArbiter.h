#pragma once

#include "Common.h"

#include "MicroManager.h"

namespace UAlbertaBot
{
	class MicroManager;

	class MicroArbiter : public MicroManager
	{
	public:
		MicroArbiter();
		//Movement handle by attacking spellcaster units
		void updateMovement(const UnitCluster& cluster, BWAPI::Unit vanguard);
		void executeMicro(const BWAPI::Unitset& targets, const UnitCluster& cluster);
		int stasisScore(BWAPI::Unit u) const;
		bool aboutToDie(const BWAPI::Unit u) const;

		BWAPI::Unitset getCaster(const UnitCluster& cluster) const;
		bool maybeSpell(BWAPI::Unit templar);
		void updateSpell(const UnitCluster& cluster);
	};
}