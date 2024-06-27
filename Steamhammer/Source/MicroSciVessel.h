#pragma once

#include "MicroManager.h"

namespace UAlbertaBot
{
class MicroSciVessel : public MicroManager
{
    // NOTE
    // This micro manager controls all defilers plus any units assigned as defiler food.
    // That means its set of units can include both defilers and zerglings.

    BWAPI::Unitset getVessels(const UnitCluster & cluster) const;

    bool aboutToDie(const BWAPI::Unit defiler) const;

    int empScore(BWAPI::Unit u) const;
    bool maybeEMP(BWAPI::Unit defiler);
   
public:
    MicroSciVessel();
    void executeMicro(const BWAPI::Unitset & targets, const UnitCluster & cluster);

    // The different updates are done on different frames to spread out the work.
    void updateMovement(const UnitCluster & cluster, BWAPI::Unit vanguard);
};
}
