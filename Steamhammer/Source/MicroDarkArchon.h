#pragma once

#include "Common.h"

namespace UAlbertaBot
{
    class MicroManager;

    class MicroDarkArchon : public MicroManager
    {
    public:

        MicroDarkArchon();
        BWAPI::Unitset getCaster(const UnitCluster& cluster) const;
        int maelstromScore(BWAPI::Unit u) const;
        int feedbackScore(BWAPI::Unit u) const;
        bool maybeSpell(BWAPI::Unit darkArchon);
        bool aboutToDie(const BWAPI::Unit templar) const;
        void executeMicro(const BWAPI::Unitset& targets, const UnitCluster& cluster);
        void updateMovement(const UnitCluster& cluster, BWAPI::Unit vanguard);

        void updateSpell(const UnitCluster& cluster);
    };
}