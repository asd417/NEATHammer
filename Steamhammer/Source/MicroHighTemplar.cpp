#include "MicroManager.h"
#include "MicroHighTemplar.h"

#include "Bases.h"
#include "The.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

#define UNITTYPE BWAPI::UnitTypes::Protoss_High_Templar
#define SPELLTECHTYPE BWAPI::TechTypes::Psionic_Storm
#define CASTERSPELLTYPE CasterSpell::PsionicStorm
#define SCOREFUNCNAME stormScore
#define SPELLENERGY 75
#define SPELLRANGE 9
#define SPELLRADIUS 1
void MicroHighTemplar::updateMovement(const UnitCluster& cluster, BWAPI::Unit vanguard)
{
    // Collect the defilers and update their states.
    BWAPI::Unitset templars = getCaster(cluster);
    if (templars.empty())
    {
        return;
    }
    updateCasters(templars);

    // Control the defilers.
    for (BWAPI::Unit templar : templars)
    {
        bool canMove = !isReadyToCast(templar);

        if (canMove)
        {
            BWAPI::Position destination;

            // Figure out where the templar should move to.
            if (vanguard)
            {
                if (templar->getEnergy() < SPELLENERGY && cluster.size() > getUnits().size())
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
                the.micro.MoveNear(templar, destination);
            }
        }
    }
}

// How valuable is it to plague this unit?
// The caller worries about whether it is our unit or the enemy's.
int MicroHighTemplar::stormScore(BWAPI::Unit u) const
{
    //Storm does no damage to stasised units
    //Storm does no damage to buildings
    //Storm does not stack
    if (u->isStasised() || u->getType().isBuilding() || u->isUnderStorm()) return 0;

    // How many HP will it lose? 
    int score = std::min(112, u->getHitPoints());

    // Bonus point if under maelstrom
    if (u->isMaelstrommed())
    {
        score += 100;
    }

    //Storm ignores armor. Therefore prioritize armored target
    if (u->getType().armor() > 0)
    {
        score += 40;
    }

    // If it's cloaked, give it a bonus--a bigger bonus if it is not detected.
    if (u->isVisible() && !u->isDetected())
    {
        score += 100;
        if (u->getType() == BWAPI::UnitTypes::Terran_Ghost)
        {
            // Help defend against nukes by revealing the ghost.
            // (Nukes are the only good reason to make a ghost versus zerg.)
            score += 100;
        }
    }
    else if (u->isCloaked())        // cloaked but detected
    {
        score += 40;		        // because plague will keep it detected
    }
    
    // If it's a carrier interceptor, give it a bonus. We like plague on interceptor.
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Interceptor)
    {
        score += 40;
    }

    return score;
}

// The defiler is probably about to die. It should cast immediately if it is ever going to.
bool MicroHighTemplar::aboutToDie(const BWAPI::Unit templar) const
{
    return
        // It has low hit points and somebody is shooting at it.
        ((templar->getHitPoints() < 30 || templar->isPlagued()) && !the.info.getEnemyFireteam(templar).empty()) ||

        // It is in a deadly situation.
        templar->isUnderStorm();
}

void UAlbertaBot::MicroHighTemplar::executeMicro(const BWAPI::Unitset& targets, const UnitCluster& cluster)
{
}


UAlbertaBot::MicroHighTemplar::MicroHighTemplar()
{
}
BWAPI::Unitset MicroHighTemplar::getCaster(const UnitCluster& cluster) const
{
    BWAPI::Unitset casterSet;
    for (BWAPI::Unit unit : getUnits())
    {
        if (unit->getType() == UNITTYPE &&
            cluster.units.contains(unit))
        {
            casterSet.insert(unit);
        }
    }
    return casterSet;
}

bool MicroHighTemplar::maybeSpell(BWAPI::Unit templar)
{
    int limit = SPELLRANGE + SPELLRADIUS;

    const bool dying = aboutToDie(templar);

    // Don't bother to look for units to plague if no enemy is close enough.
    BWAPI::Unit closest = BWAPI::Broodwar->getClosestUnit(templar->getPosition(),
        BWAPI::Filter::IsEnemy && !BWAPI::Filter::IsStasised,
        limit * 32);

    if (!dying && !closest)
    {
        return false;
    }

    // Look for the box with the best effect.
    int bestScore = 0;
    BWAPI::Position bestPlace;
    for (int tileX = std::max(2, templar->getTilePosition().x - limit); tileX <= std::min(BWAPI::Broodwar->mapWidth() - 3, templar->getTilePosition().x + limit); ++tileX)
    {
        for (int tileY = std::max(2, templar->getTilePosition().y - limit); tileY <= std::min(BWAPI::Broodwar->mapHeight() - 3, templar->getTilePosition().y + limit); ++tileY)
        {
            int score = 0;
            BWAPI::Position place(BWAPI::TilePosition(tileX, tileY));
            const BWAPI::Position offset(SPELLRADIUS * 32, SPELLRADIUS * 32);
            BWAPI::Unitset affected = BWAPI::Broodwar->getUnitsInRectangle(place - offset, place + offset);
            for (BWAPI::Unit u : affected)
            {
                if (u->getPlayer() == BWAPI::Broodwar->self())
                {
                    score -= SCOREFUNCNAME(u);
                }
                else if (u->getPlayer() == BWAPI::Broodwar->enemy())
                {
                    score += SCOREFUNCNAME(u);
                }
            }
            if (score > bestScore)
            {
                bestScore = score;
                bestPlace = place;
            }
        }
    }
    if (bestScore > 100 || dying && bestScore > 0)
    {
        setReadyToCast(templar, CASTERSPELLTYPE);
        return spell(templar, SPELLTECHTYPE, bestPlace);
    }
    return false;
}
void MicroHighTemplar::updateSpell(const UnitCluster& cluster)
{
    BWAPI::Unitset casters = getCaster(cluster);
    if (casters.empty()) return;
    updateCasters(casters);

    for (BWAPI::Unit caster : casters)
    {
        if (caster->getEnergy() >= SPELLENERGY &&
            BWAPI::Broodwar->self()->hasResearched(SPELLTECHTYPE) &&
            caster->canUseTech(SPELLTECHTYPE, caster->getPosition()) &&
            !isReadyToCastOtherThan(caster, CASTERSPELLTYPE))
        {
            if (!maybeSpell(caster) && isReadyToCast(caster))
            {
                clearReadyToCast(caster);
                return;     // only one defiler may cast per call, to reduce duplication
            }
        }
    }
}
