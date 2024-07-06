#include "MicroGhosts.h"

#include "Bases.h"
#include "The.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

// The defiler is probably about to die. It should cast immediately if it is ever going to.
bool MicroGhosts::aboutToDie(const BWAPI::Unit ghost) const
{
    return
        // It has low hit points and somebody is shooting at it.
        ((ghost->getHitPoints() < 20 || ghost->isPlagued()) && !the.info.getEnemyFireteam(ghost).empty()) ||
        // It is in a deadly situation.
        ghost->isUnderStorm();
}

int MicroGhosts::lockdownScore(BWAPI::Unit u) const
{
    BWAPI::UnitType type = u->getType();
    
    return type.destroyScore();
}


// We can Lockdown. Look around to see if we should, and if so, do it.
bool MicroGhosts::maybeLockdown(BWAPI::Unit ghost)
{
    // Lockdown has range 8
    const int limit = 8;
    const bool dying = aboutToDie(ghost);

    // Don't bother to look for units to plague if no enemy is close enough.
    BWAPI::Unit closest = BWAPI::Broodwar->getClosestUnit(ghost->getPosition(),
        BWAPI::Filter::IsEnemy,
        limit * 32);

    if (!dying && !closest)
    {
        return false;
    }

    int bestScore = 0;
    BWAPI::Unit bestUnit;
    BWAPI::Unitset targets = BWAPI::Broodwar->getUnitsInRadius(ghost->getPosition(), limit, BWAPI::Filter::GetPlayer != the.self());

    for (const auto& t : targets)
    {
        int s = lockdownScore(t);
        if (s > bestScore)
        {
            bestScore = s;
            bestUnit = t;
        }
    }

    if (bestScore > 100 || dying && bestScore > 0)
    {
        setReadyToCast(ghost, CasterSpell::LOCKDOWN);
        return spell(ghost, BWAPI::TechTypes::Lockdown, bestUnit);
    }

    return false;
}

MicroGhosts::MicroGhosts()
{
}

// Unused but required.
void MicroGhosts::executeMicro(const BWAPI::Unitset& targets, const UnitCluster& cluster)
{
}

// Consume for energy if possible and necessary; otherwise move.
void MicroGhosts::updateMovement(const UnitCluster& cluster, BWAPI::Unit vanguard)
{
    // Collect the defilers and update their states.
    BWAPI::Unitset ghosts = Intersection(getUnits(), cluster.units);
    if (ghosts.empty())
    {
        return;
    }
    updateCasters(ghosts);

    // Control the vessls.
    for (BWAPI::Unit ghost : ghosts)
    {
        if (!isReadyToCast(ghost))
        {
            BWAPI::Position destination;

            // Figure out where the ghost should move to.
            if (vanguard)
            {
                if (ghost->getEnergy() < 100 && cluster.size() > getUnits().size())
                {
                    destination = cluster.center;
                }
                else
                {
                    destination = vanguard->getPosition();
                }
            }
            else
            {
                // Default destination if all else fails: The front defense line.
                destination = the.bases.front();
            }

            if (destination.isValid())
            {
                the.micro.MoveNear(ghost, destination);
            }

        }
    }
}

// Cast Irradiate if possible and useful.
void MicroGhosts::updateLockdown(const UnitCluster& cluster)
{
    if (!the.self()->hasResearched(BWAPI::TechTypes::Lockdown)) return;
    BWAPI::Unitset ghosts = Intersection(getUnits(), cluster.units);
    if (ghosts.empty())
    {
        return;
    }
    updateCasters(ghosts);

    for (BWAPI::Unit ghost : ghosts)
    {
        if (ghost->getEnergy() >= 100 &&
            ghost->canUseTech(BWAPI::TechTypes::Lockdown, ghost->getPosition()) &&
            !isReadyToCastOtherThan(ghost, CasterSpell::LOCKDOWN))
        {
            if (!maybeLockdown(ghost) && isReadyToCast(ghost))
            {
                clearReadyToCast(ghost);
                return;     // only one defiler may cast per call, to reduce duplication
            }
        }
    }
}

