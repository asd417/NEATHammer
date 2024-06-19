#pragma once

#include "Common.h"

namespace UAlbertaBot
{
class MicroManager;

class MicroHighTemplar : public MicroManager
{
public:

    MicroHighTemplar();
    BWAPI::Unitset getCaster(const UnitCluster& cluster) const;
    int stormScore(BWAPI::Unit u) const;
    bool maybeSpell(BWAPI::Unit templar);
    bool aboutToDie(const BWAPI::Unit templar) const;
    void executeMicro(const BWAPI::Unitset & targets, const UnitCluster & cluster);
    void updateMovement(const UnitCluster& cluster, BWAPI::Unit vanguard);

    void updateSpell(const UnitCluster& cluster);
};
}