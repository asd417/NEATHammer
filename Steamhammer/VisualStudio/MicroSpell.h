#pragma once
#include <functional>
#include "Common.h"
#include "BWAPI.h"

#include "Bases.h"
#include "The.h"
#include "UnitUtil.h"

namespace UAlbertaBot
{
	class UnitCluster;
	class MicroSpell {
	public:
		MicroSpell(
			int _radius,
			BWAPI::TechType _tech,
			BWAPI::UnitType _casterType,
			std::function<BWAPI::Unitset& ()> _getUnits,
			std::function<int(BWAPI::Unit)> _scoreFunction,
			std::function<bool(const BWAPI::Unit)> _aboutToDie, 
			std::function<void(const BWAPI::Unitset&)> _updateCasters,
			std::function<bool(BWAPI::Unit, CasterSpell)> _isReadyToCastOtherThan);

		void updateSpell(const UnitCluster& cluster);

	private:
		BWAPI::Unitset getCasterUnits(const UnitCluster& cluster) const;
		int energyCost;
		BWAPI::TechType techType;
		BWAPI::UnitType casterType;
		CasterSpell casterSpell;
		std::function<bool(BWAPI::Unit, CasterSpell)>isReadyToCastOtherThan;
		std::function<void(const BWAPI::Unitset&)> updateCasters;
		std::function<bool(const BWAPI::Unit)> aboutToDie;
		std::function<int(BWAPI::Unit)> scoreFunction;
		std::function<BWAPI::Unitset& ()> getUnits;
		int spellRange;
		int spellRadius;
		
	};
}