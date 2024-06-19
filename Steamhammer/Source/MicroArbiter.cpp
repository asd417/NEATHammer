#include "MicroArbiter.h"

#include "Bases.h"
#include "OpsBoss.h"
#include "The.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

#define UNITTYPE BWAPI::UnitTypes::Protoss_Arbiter
#define SPELLTECHTYPE BWAPI::TechTypes::Stasis_Field
#define CASTERSPELLTYPE CasterSpell::StasisField
#define SCOREFUNCNAME stasisScore
#define SPELLENERGY 100
#define SPELLRANGE 9
#define SPELLRADIUS 2

MicroArbiter::MicroArbiter()
{
}


void MicroArbiter::updateMovement(const UnitCluster& cluster, BWAPI::Unit vanguard)
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

            // Figure out where the defiler should move to.
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

void UAlbertaBot::MicroArbiter::executeMicro(const BWAPI::Unitset& targets, const UnitCluster& cluster)
{
}


int MicroArbiter::stasisScore(BWAPI::Unit u) const
{
    BWAPI::UnitType ut = u->getType();
    if (
        ut == BWAPI::UnitTypes::Protoss_Carrier ||
        ut == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode ||
        ut == BWAPI::UnitTypes::Zerg_Mutalisk ||
        ut == BWAPI::UnitTypes::Zerg_Defiler 
        ) return 100;
    if (
        ut == BWAPI::UnitTypes::Protoss_Zealot ||
        ut == BWAPI::UnitTypes::Protoss_High_Templar ||
        ut == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode
        ) return 80;
    if (ut.airWeapon().targetsAir()) return 50;
    if (ut.groundWeapon().maxRange() > 32) return 40;
    return 30;
}

bool MicroArbiter::aboutToDie(const BWAPI::Unit u) const
{
    return
        // It has low hit points and somebody is shooting at it.
        ((u->getHitPoints() < 30 || u->isPlagued()) && !the.info.getEnemyFireteam(u).empty()) ||

        u->getShields() < 30 && u->getHitPoints() < 30 ||
        // It is in a deadly situation.
        u->isUnderStorm();
}


BWAPI::Unitset MicroArbiter::getCaster(const UnitCluster& cluster) const
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

bool MicroArbiter::maybeSpell(BWAPI::Unit templar)
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
void MicroArbiter::updateSpell(const UnitCluster& cluster)
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
