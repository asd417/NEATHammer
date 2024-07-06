#pragma once

#include "MicroManager.h"

namespace UAlbertaBot
{
    class MicroGhosts : public MicroManager
    {
        bool aboutToDie(const BWAPI::Unit defiler) const;

        int lockdownScore(BWAPI::Unit u) const;
        bool maybeLockdown(BWAPI::Unit vessel);

    public:
        MicroGhosts();
        void executeMicro(const BWAPI::Unitset& targets, const UnitCluster& cluster);

        // The different updates are done on different frames to spread out the work.
        void updateMovement(const UnitCluster& cluster, BWAPI::Unit vanguard);
        void updateLockdown(const UnitCluster& cluster);
    };
}
