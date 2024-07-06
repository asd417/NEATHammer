#pragma once

#include "MicroManager.h"

namespace UAlbertaBot
{
class MicroSciVessel : public MicroManager
{
    bool aboutToDie(const BWAPI::Unit defiler) const;

    int empScore(BWAPI::Unit u) const;
    bool maybeEMP(BWAPI::Unit defiler);

    int irradiateScore(BWAPI::Unit u) const;
    bool maybeIrradiate(BWAPI::Unit vessel);
   
public:
    MicroSciVessel();
    void executeMicro(const BWAPI::Unitset & targets, const UnitCluster & cluster);

    // The different updates are done on different frames to spread out the work.
    void updateMovement(const UnitCluster & cluster, BWAPI::Unit vanguard);
    void updateEMP(const UnitCluster& cluster);
    void updateIrradiate(const UnitCluster& cluster);
};
}
