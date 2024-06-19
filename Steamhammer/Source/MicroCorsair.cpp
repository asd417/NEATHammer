#include "MicroCorsair.h"

#include "OpsBoss.h"
#include "The.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

// The splash air-to-air units: Valkyries, corsairs, devourers.

MicroCorsair::MicroCorsair()
{ 
}

void UAlbertaBot::MicroCorsair::executeMicro(const BWAPI::Unitset& targets, const UnitCluster& cluster)
{
    BWAPI::Unitset units = getUnits();
    if (units.empty())
    {
        return;
    }
    assignTargets(units, targets);
}

void MicroCorsair::assignTargets(const BWAPI::Unitset & airUnits, const BWAPI::Unitset & targets)
{
    // The set of potential targets.
    BWAPI::Unitset airTargets;
    std::copy_if(targets.begin(), targets.end(), std::inserter(airTargets, airTargets.end()),
        [=](BWAPI::Unit u) {
        return
            u->isFlying() &&
            !infestable(u);
    });

    for (BWAPI::Unit airUnit : airUnits)
    {
        if (order->isCombatOrder())
        {
            BWAPI::Unit target = getTarget(airUnit, airTargets);
            if (target)
            {
                // A target was found.
                if (Config::Debug::DrawUnitTargets)
                {
                    BWAPI::Broodwar->drawLineMap(airUnit->getPosition(), airUnit->getTargetPosition(), BWAPI::Colors::Purple);
                }

                the.micro.CatchAndAttackUnit(airUnit, target);
            }
            else
            {
                // No target found. Go to the attack position.
                the.micro.AttackMove(airUnit, order->getPosition());
            }
        }
    }
}

// This could return null if no target is worth attacking, but doesn't happen to.
BWAPI::Unit MicroCorsair::getTarget(BWAPI::Unit airUnit, const BWAPI::Unitset & targets)
{
    int bestScore = INT_MIN;
    BWAPI::Unit bestTarget = nullptr;

    for (const auto target : targets)
    {
        const int priority = getAttackPriority(airUnit, target);		// 0..12
        const int range = airUnit->getDistance(target);					// 0..map size in pixels
        const int closerToGoal =										// positive if target is closer than us to the goal
            airUnit->getDistance(order->getPosition()) - target->getDistance(order->getPosition());

        // Skip targets that are too far away to worry about.
        if (range >= 13 * 32)
        {
            continue;
        }

        // Let's say that 1 priority step is worth 160 pixels (5 tiles).
        // We care about unit-target range and target-order position distance.
        int score = 5 * 32 * priority - range;

        // Adjust for special features.
        // A bonus for attacking enemies that are "in front".
        // It helps reduce distractions from moving toward the goal, the order position.
        if (closerToGoal > 0)
        {
            score += 3 * 32;
        }

        // This could adjust for relative speed and direction, so that we don't chase what we can't catch.
        if (airUnit->isInWeaponRange(target))
        {
            score += 4 * 32;
        }
        else if (!target->isMoving())
        {
            score += 24;
        }
        else if (target->isBraking())
        {
            score += 16;
        }
        else if (target->getPlayer()->topSpeed(target->getType()) >= airUnit->getPlayer()->topSpeed(airUnit->getType()))
        {
            score -= 5 * 32;
        }
        
        // Prefer targets that are already hurt.
        if (target->getType().getRace() == BWAPI::Races::Protoss && target->getShields() == 0)
        {
            score += 32;
        }
        if (target->getHitPoints() < target->getType().maxHitPoints())
        {
            score += 24;
        }

        // TODO prefer targets in groups, so they'll all get splashed

        if (score > bestScore)
        {
            bestScore = score;
            bestTarget = target;
        }
    }
    
    return bestTarget;
}

int MicroCorsair::disruptionScore(BWAPI::Unit u) const
{
    if (u->getType().isFlyer()) return 0;
    if (u->getType().airWeapon().targetsAir()) return 100;
    if (u->getType().groundWeapon().maxRange()>32) return 40;
    return 30;
}

bool MicroCorsair::aboutToDie(const BWAPI::Unit u) const
{
    return
        // It has low hit points and somebody is shooting at it.
        ((u->getHitPoints() < 30 || u->isPlagued()) && !the.info.getEnemyFireteam(u).empty()) ||

        u->getShields() < 30 && u->getHitPoints() < 30 ||
        // It is in a deadly situation.
        u->isUnderStorm();
}

// get the attack priority of a target unit
int MicroCorsair::getAttackPriority(BWAPI::Unit airUnit, BWAPI::Unit target)
{
    const BWAPI::UnitType rangedType = airUnit->getType();
    const BWAPI::UnitType targetType = target->getType();

    // Scourge are dangerous and are the worst.
    if (targetType == BWAPI::UnitTypes::Zerg_Scourge)
    {
        return 10;
    }

    // Threats can attack us back.
    if (UnitUtil::TypeCanAttackAir(targetType))    // includes carriers
    {
        // Enemy unit which is far enough outside its range is lower priority.
        if (airUnit->getDistance(target) > 64 + UnitUtil::GetAttackRange(target, airUnit))
        {
            return 8;
        }
        return 9;
    }
    // Certain other enemies are also bad.
    if (targetType == BWAPI::UnitTypes::Terran_Science_Vessel ||
        targetType == BWAPI::UnitTypes::Terran_Dropship ||
        targetType == BWAPI::UnitTypes::Protoss_Shuttle ||
        targetType == BWAPI::UnitTypes::Protoss_Arbiter ||
        targetType == BWAPI::UnitTypes::Zerg_Overlord)
    {
        return 8;
    }

    // Floating buildings are less important than other units.
    if (targetType.isBuilding())
    {
        return 1;
    }

    // Other air units are a little less important.
    return 7;
}



#define UNITTYPE BWAPI::UnitTypes::Protoss_Corsair
#define SPELLTECHTYPE BWAPI::TechTypes::Disruption_Web
#define CASTERSPELLTYPE CasterSpell::DisruptionWeb
#define SCOREFUNCNAME disruptionScore
#define SPELLENERGY 125
#define SPELLRANGE 9
#define SPELLRADIUS 2
BWAPI::Unitset MicroCorsair::getCaster(const UnitCluster& cluster) const
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

bool MicroCorsair::maybeSpell(BWAPI::Unit templar)
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
void MicroCorsair::updateSpell(const UnitCluster& cluster)
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
