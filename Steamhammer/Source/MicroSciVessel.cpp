#include "MicroSciVessel.h"

#include "Bases.h"
#include "The.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

// The defiler is probably about to die. It should cast immediately if it is ever going to.
bool MicroSciVessel::aboutToDie(const BWAPI::Unit vessel) const
{
    return
        // It has low hit points and somebody is shooting at it.
        ((vessel->getHitPoints() < 50 || vessel->isPlagued()) && !the.info.getEnemyFireteam(vessel).empty()) ||

        // It is in a deadly situation.
        vessel->isUnderStorm();
}

int MicroSciVessel::empScore(BWAPI::Unit u) const
{
    BWAPI::UnitType type = u->getType();
    if (type == BWAPI::UnitTypes::Zerg_Broodling) return 0;
    return type.maxEnergy() + type.maxShields();
}

// We can EMP. Look around to see if we should, and if so, do it.
bool MicroSciVessel::maybeEMP(BWAPI::Unit vessel)
{
    // EMP has range 8 and affects a 6x6 box. We look a little beyond that range for targets.
    const int limit = 11;

    const bool dying = aboutToDie(vessel);

    // Don't bother to look for units to plague if no enemy is close enough.
    BWAPI::Unit closest = BWAPI::Broodwar->getClosestUnit(vessel->getPosition(),
        BWAPI::Filter::IsEnemy,
        limit * 32);

    if (!dying && !closest)
    {
        return false;
    }

    // Look for the box with the best effect.
    int bestScore = 0;
    BWAPI::Position bestPlace;
    for (int tileX = std::max(2, vessel->getTilePosition().x-limit); tileX <= std::min(BWAPI::Broodwar->mapWidth()-3, vessel->getTilePosition().x+limit); ++tileX)
    {
        for (int tileY = std::max(2, vessel->getTilePosition().y - limit); tileY <= std::min(BWAPI::Broodwar->mapHeight()-3, vessel->getTilePosition().y+limit); ++tileY)
        {
            int score = 0;
            BWAPI::Position place(BWAPI::TilePosition(tileX, tileY));
            const BWAPI::Position offset(3 * 32, 3 * 32);
            BWAPI::Unitset affected = BWAPI::Broodwar->getUnitsInRectangle(place - offset, place + offset);
            for (BWAPI::Unit u : affected)
            {
                if (u->getPlayer() == BWAPI::Broodwar->self())
                {
                    score -= empScore(u);
                }
                else if (u->getPlayer() == BWAPI::Broodwar->enemy())
                {
                    score += empScore(u);
                }
            }
            if (score > bestScore)
            {
                bestScore = score;
                bestPlace = place;
            }
        }
    }
    BWAPI::Unit closestToBestPlace = BWAPI::Broodwar->getClosestUnit(bestPlace, BWAPI::Filter::GetPlayer != the.self(), 11);
    if (bestScore > 100 || dying && bestScore > 0)
    {
        setReadyToCast(vessel, CasterSpell::EMP);
        return spell(vessel, BWAPI::TechTypes::EMP_Shockwave, closestToBestPlace);
    }

    return false;
}


int MicroSciVessel::irradiateScore(BWAPI::Unit u) const
{
    BWAPI::UnitType type = u->getType();
    if (type.isOrganic())
    {
        // Higher score for tanky units. This will likely cause other nearby units with low health to die
        return type.maxHitPoints();
    }
    return 0;
}


// We can plague. Look around to see if we should, and if so, do it.
bool MicroSciVessel::maybeIrradiate(BWAPI::Unit vessel)
{
    // EMP has range 8 and affects a 6x6 box. We look a little beyond that range for targets.
    const int limit = 11;

    const bool dying = aboutToDie(vessel);

    // Don't bother to look for units to plague if no enemy is close enough.
    BWAPI::Unit closest = BWAPI::Broodwar->getClosestUnit(vessel->getPosition(),
        BWAPI::Filter::IsEnemy,
        limit * 32);

    if (!dying && !closest)
    {
        return false;
    }

    // Look for the box with the best effect.
    int bestScore = 0;
    BWAPI::Position bestPlace;
    for (int tileX = std::max(2, vessel->getTilePosition().x - limit); tileX <= std::min(BWAPI::Broodwar->mapWidth() - 3, vessel->getTilePosition().x + limit); ++tileX)
    {
        for (int tileY = std::max(2, vessel->getTilePosition().y - limit); tileY <= std::min(BWAPI::Broodwar->mapHeight() - 3, vessel->getTilePosition().y + limit); ++tileY)
        {
            int score = 0;
            BWAPI::Position place(BWAPI::TilePosition(tileX, tileY));
            const BWAPI::Position offset(3 * 32, 3 * 32);
            BWAPI::Unitset affected = BWAPI::Broodwar->getUnitsInRectangle(place - offset, place + offset);
            for (BWAPI::Unit u : affected)
            {
                if (u->getPlayer() == BWAPI::Broodwar->self())
                {
                    score -= irradiateScore(u);
                }
                else if (u->getPlayer() == BWAPI::Broodwar->enemy())
                {
                    score += irradiateScore(u);
                }
            }
            if (score > bestScore)
            {
                bestScore = score;
                bestPlace = place;
            }
        }
    }
    BWAPI::Unit closestToBestPlace = BWAPI::Broodwar->getClosestUnit(bestPlace, BWAPI::Filter::GetPlayer != the.self(), 11);
    if (bestScore > 100 || dying && bestScore > 0)
    {
        setReadyToCast(vessel, CasterSpell::IRRADIATE);
        return spell(vessel, BWAPI::TechTypes::Irradiate, closestToBestPlace);
    }

    return false;
}

MicroSciVessel::MicroSciVessel()
{ 
}

// Unused but required.
void MicroSciVessel::executeMicro(const BWAPI::Unitset & targets, const UnitCluster & cluster)
{
}

// Consume for energy if possible and necessary; otherwise move.
void MicroSciVessel::updateMovement(const UnitCluster & cluster, BWAPI::Unit vanguard)
{
    // Collect the defilers and update their states.
    BWAPI::Unitset vessels = Intersection(getUnits(), cluster.units);
    if (vessels.empty())
    {
        return;
    }
    updateCasters(vessels);
    // Control the vessels.
    for (BWAPI::Unit vessel : vessels)
    {
        if (isReadyToCast(vessel)) continue;
        
        BWAPI::Position destination;

        // Figure out where the vessel should move to.
        if (vanguard)
        {
            if (vessel->getEnergy() < 100 && cluster.size() > getUnits().size())
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
            // BWAPI::Broodwar->printf("defiler %d at %d,%d will move near %d,%d",
            //	defiler->getID(), defiler->getPosition().x, defiler->getPosition().y, destination.x, destination.y);
            the.micro.MoveNear(vessel, destination);
        }
        
    }
}
//
// Cast EMP if possible and useful.
void MicroSciVessel::updateEMP(const UnitCluster & cluster)
{
    if (!the.self()->hasResearched(BWAPI::TechTypes::EMP_Shockwave)) return;
    BWAPI::Unitset vessels = Intersection(getUnits(), cluster.units);
    if (vessels.empty())
    {
        return;
    }
    updateCasters(vessels);

    for (BWAPI::Unit vessel : vessels)
    {
        if (vessel->getEnergy() >= 100 &&
            vessel->canUseTech(BWAPI::TechTypes::EMP_Shockwave, vessel->getPosition()) &&
            !isReadyToCastOtherThan(vessel, CasterSpell::EMP))
        {
            if (!maybeEMP(vessel) && isReadyToCast(vessel))
            {
                clearReadyToCast(vessel);
                return;     // only one defiler may cast per call, to reduce duplication
            }
        }
    }
}
// Cast Irradiate if possible and useful.
void MicroSciVessel::updateIrradiate(const UnitCluster& cluster)
{
    if (!the.self()->hasResearched(BWAPI::TechTypes::Irradiate)) return;
    BWAPI::Unitset vessels = Intersection(getUnits(), cluster.units);
    if (vessels.empty())
    {
        return;
    }
    updateCasters(vessels);

    for (BWAPI::Unit vessel : vessels)
    {
        if (!vessel->isBurrowed() &&
            vessel->getEnergy() >= 100 &&
            vessel->canUseTech(BWAPI::TechTypes::Irradiate, vessel->getPosition()) &&
            !isReadyToCastOtherThan(vessel, CasterSpell::IRRADIATE))
        {
            if (!maybeIrradiate(vessel) && isReadyToCast(vessel))
            {
                clearReadyToCast(vessel);
                return;     // only one defiler may cast per call, to reduce duplication
            }
        }
    }
}

