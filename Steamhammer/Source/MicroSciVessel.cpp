#include "MicroSciVessel.h"

#include "Bases.h"
#include "The.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

// NOTE
// The computations to decide whether and where to swarm and plague are expensive.
// Don't have too many defilers at the same time, or you might time out.
// A natural fix would be to designate one or a few defilers as "active" and
// keep the others in reserve.

// The defilers in this cluster.
// This will rarely return more than one defiler.
BWAPI::Unitset MicroSciVessel::getVessels(const UnitCluster & cluster) const
{
    BWAPI::Unitset v;

    for (BWAPI::Unit unit : getUnits())
    {
        if (unit->getType() == BWAPI::UnitTypes::Terran_Science_Vessel &&
            cluster.units.contains(unit))
        {
            v.insert(unit);
        }
    }

    return v;
}

// The defiler is probably about to die. It should cast immediately if it is ever going to.
bool MicroSciVessel::aboutToDie(const BWAPI::Unit vessel) const
{
    return
        // It has low hit points and somebody is shooting at it.
        ((vessel->getHitPoints() < 40 || vessel->isPlagued()) && !the.info.getEnemyFireteam(vessel).empty()) ||

        // It is in a deadly situation.
        
        vessel->isUnderStorm();
}

// We need to consume and have it researched. Look around for food.
// For now, we consume zerglings.
// NOTE This doesn't take latency into account, so it issues the consume order
//      repeatedly until the latency time has elapsed. It looks funny in game,
//      but there don't seem to be any bad effects.
//bool MicroDefilers::maybeConsume(BWAPI::Unit defiler, BWAPI::Unitset & food)
//{
//    // If there is a zergling in range, snarf it down.
//    // Consume has a range of 1 tile = 32 pixels.
//    for (BWAPI::Unit zergling : food)
//    {
//        if (defiler->getDistance(zergling) <= 32 &&
//            defiler->canUseTechUnit(BWAPI::TechTypes::Consume, zergling))
//        {
//            // BWAPI::Broodwar->printf("consume!");
//            (void) the.micro.UseTech(defiler, BWAPI::TechTypes::Consume, zergling);
//            food.erase(zergling);
//            return true;
//        }
//    }
//
//    return false;
//}

int MicroSciVessel::empScore(BWAPI::Unit u) const
{
    BWAPI::UnitType type = u->getType();
    if (type == BWAPI::UnitTypes::Zerg_Broodling) return 0;
    if (type.maxEnergy() != 0)
    {
        // Workers cannot do damage under dark swarm.
        return type.maxEnergy();
    }
    if (type.maxShields() != 0)
    {
        // Even if it is static defense, it cannot do damage under swarm
        // (unless it is a bunker with firebats inside, a case that we ignore).
        return type.maxShields();
    }
    return 0;
}

// We can cast dark swarm. Do it if it makes sense.
// There are a lot of cases when we might want to swarm. For example:
// - Swarm defensively if the enemy has air units and we have ground units.
// - Swarm if we have melee units and the enemy has ranged units.
// - Swarm offensively to take down cannons/bunkers/sunkens.
// So far, we only implement a simple case: We're attacking toward enemy
// buildings, and the enemy is short of units that can cause damage under swarm.
// The buildings guarantee that the enemy can't simply run away without further
// consequence.

// Score units, pick the ones with higher scores and try to swarm there.
// Swarm has a range of 9 and covers a 6x6 area (according to Liquipedia) or 5x5 (according to BWAPI).
//bool MicroDefilers::maybeSwarm(BWAPI::Unit defiler)
//{
//    // Swarm has range 9. We look a little beyond that range for targets.
//    const int limit = 14;
//
//    const bool dying = aboutToDie(defiler);
//
//    // Usually, swarm only if there is an enemy building to cover or ranged unit to neutralize.
//    // (The condition for ranged units is not comprehensive.)
//    // NOTE A carrier does not have a ground weapon, but its interceptors do. Swarm under interceptors.
//    // If the defiler is about to die, swarm may still be worth it even if it covers nothing.
//    if (!dying &&
//        BWAPI::Broodwar->getUnitsInRadius(defiler->getPosition(), limit * 32,
//        BWAPI::Filter::IsEnemy && (BWAPI::Filter::IsBuilding ||
//                                   BWAPI::Filter::IsFlyer && BWAPI::Filter::GroundWeapon != BWAPI::WeaponTypes::None ||
//                                   BWAPI::Filter::GetType == BWAPI::UnitTypes::Terran_Marine ||
//                                   BWAPI::Filter::GetType == BWAPI::UnitTypes::Protoss_Dragoon ||
//                                   BWAPI::Filter::GetType == BWAPI::UnitTypes::Zerg_Hydralisk)
//                                   ).empty())
//    {
//        return false;
//    }
//
//    // Look for the box with the best effect.
//    // NOTE This is not really the calculation we want. Better would be to find the box
//    // that nullifies the most enemy fire where we want to attack, no matter where the fire is from.
//    // NOTE We want a dying defiler to swarm over itself only once, not repeatedly!
//    int bestScore = 0;
//    BWAPI::Position bestPlace = defiler->isUnderDarkSwarm() ? BWAPI::Positions::None : defiler->getPosition();
//    for (int tileX = std::max(3, defiler->getTilePosition().x - limit); tileX <= std::min(BWAPI::Broodwar->mapWidth() - 4, defiler->getTilePosition().x + limit); ++tileX)
//    {
//        for (int tileY = std::max(3, defiler->getTilePosition().y - limit); tileY <= std::min(BWAPI::Broodwar->mapHeight() - 4, defiler->getTilePosition().y + limit); ++tileY)
//        {
//            int score = 0;
//            bool hasRangedEnemy = false;
//            BWAPI::Position place(BWAPI::TilePosition(tileX, tileY));
//            const BWAPI::Position offset(3 * 32, 3 * 32);
//            BWAPI::Unitset affected = BWAPI::Broodwar->getUnitsInRectangle(place - offset, place + offset);
//            for (BWAPI::Unit u : affected)
//            {
//                if (u->isUnderDarkSwarm())
//                {
//                    // Reduce overlapping swarms.
//                    continue;
//                }
//                
//                if (u->getPlayer() == BWAPI::Broodwar->self())
//                {
//                    score += swarmScore(u);
//                }
//                else if (u->getPlayer() == BWAPI::Broodwar->enemy())
//                {
//                    score -= swarmScore(u);
//                    if (u->getType().isBuilding() && !u->getType().isAddon())
//                    {
//                        score += 2;		// enemy buildings under swarm are targets
//                    }
//                    if (u->getType().groundWeapon() != BWAPI::WeaponTypes::None &&
//                        u->getType().groundWeapon().maxRange() > 32)
//                    {
//                        hasRangedEnemy = true;
//                    }
//                }
//            }
//            if (hasRangedEnemy && score > bestScore)
//            {
//                bestScore = score;
//                bestPlace = place;
//            }
//        }
//    }
//
//    if (bestScore > 0)
//    {
//        // BWAPI::Broodwar->printf("swarm score %d at %d,%d", bestScore, bestPlace.x, bestPlace.y);
//    }
//
//    // NOTE If bestScore is 0, then bestPlace may be the defiler's location (set above).
//    if ((bestScore > 10 || dying) && bestPlace.isValid())
//    {
//        setReadyToCast(defiler, CasterSpell::DarkSwarm);
//        return spell(defiler, BWAPI::TechTypes::Dark_Swarm, bestPlace);
//    }
//
//    return false;
//}

// How valuable is it to plague this unit?
// The caller worries about whether it is our unit or the enemy's.
//int MicroDefilers::plagueScore(BWAPI::Unit u) const
//{
//    if (u->isPlagued() || u->isBurrowed() || u->isInvincible())
//    {
//        return 0;
//    }
//
//    // How many HP will it lose? Assume that it may go down to 1 HP (simple and almost right).
//    int endHP = std::max(1, u->getHitPoints() - 295);
//    int score = u->getHitPoints() - endHP;
//
//    // If it's cloaked, give it a bonus--a bigger bonus if it is not detected.
//    if (u->isVisible() && !u->isDetected())
//    {
//        score += 100;
//        if (u->getType() == BWAPI::UnitTypes::Terran_Ghost)
//        {
//            // Help defend against nukes by revealing the ghost.
//            // (Nukes are the only good reason to make a ghost versus zerg.)
//            score += 100;
//        }
//    }
//    else if (u->isCloaked())        // cloaked but detected
//    {
//        score += 40;		        // because plague will keep it detected
//    }
//    // If it's a static defense building, give it a bonus.
//    else if (UnitUtil::IsStaticDefense(u->getType()))
//    {
//        score += score / 2;
//    }
//    // If it's a building other than static defense, give it a discount.
//    else if (u->getType().isBuilding())
//    {
//        if (u->getType().getRace() == BWAPI::Races::Terran &&
//            endHP < u->getType().maxHitPoints() / 3 &&
//            u->getHitPoints() >= u->getType().maxHitPoints() / 3)
//        {
//            // Will put the terran building into the red, so it starts to burn down.
//            // Count the full score.
//        }
//        else
//        {
//            // Will only hurt the building modestly.
//            // Discount the score.
//            score = score / 3;
//        }
//    }
//    // If it's a carrier interceptor, give it a bonus. We like plague on interceptor.
//    else if (u->getType() == BWAPI::UnitTypes::Protoss_Interceptor)
//    {
//        score += 40;
//    }
//
//    return score;
//}

// We can plague. Look around to see if we should, and if so, do it.
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

    if (bestScore > 0)
    {
        // BWAPI::Broodwar->printf("plague score %g at %d,%d", bestScore, bestPlace.x, bestPlace.y);
    }

    if (bestScore > 100 || dying && bestScore > 0)
    {
        setReadyToCast(vessel, CasterSpell::EMP);
        return spell(vessel, BWAPI::TechTypes::Plague, bestPlace);
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

    if (bestScore > 0)
    {
        // BWAPI::Broodwar->printf("plague score %g at %d,%d", bestScore, bestPlace.x, bestPlace.y);
    }

    if (bestScore > 100 || dying && bestScore > 0)
    {
        setReadyToCast(vessel, CasterSpell::EMP);
        return spell(vessel, BWAPI::TechTypes::Plague, bestPlace);
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
    BWAPI::Unitset vessels = getVessels(cluster);
    if (vessels.empty())
    {
        return;
    }
    updateCasters(vessels);

    // BWAPI::Broodwar->printf("cluster size %d, defilers %d, energy %d", cluster.size(), defilers.size(), (*defilers.begin())->getEnergy());

    // Collect the food.
    // The food may be far away, not in the current cluster.
    BWAPI::Unitset food;
    for (BWAPI::Unit unit : getUnits())
    {
        if (unit->getType() != BWAPI::UnitTypes::Zerg_Defiler)
        {
            food.insert(unit);
        }
    }

    //BWAPI::Broodwar->printf("defiler update movement, %d food", food.size());

    // Control the defilers.
    for (BWAPI::Unit vessel : vessels)
    {
        bool canMove = !isReadyToCast(vessel);
        if (!canMove)
        {
            // BWAPI::Broodwar->printf("defiler can't move, ready to cast");
        }
        else if (vessel->isIrradiated() && vessel->getEnergy() < 90 && vessel->canBurrow())
        {
            // BWAPI::Broodwar->printf("defiler is irradiated");
            //canMove = false;
            //the.micro.Burrow(defiler);
        }

        if (canMove && vessel->getEnergy() < 150 &&
            BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Consume) &&
            !food.empty())
        {
            //canMove = !maybeConsume(defiler, food);
        }

        if (canMove)
        {
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
            else
            {
                // BWAPI::Broodwar->printf("defiler wants to move, has nowhere to go");
            }
        }
        else
        {
            // BWAPI::Broodwar->printf("defiler cannot move");
        }
    }
}
//
// Cast EMP if possible and useful.
void MicroSciVessel::updateEMP(const UnitCluster & cluster)
{
    BWAPI::Unitset vessels = getVessels(cluster);
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
    BWAPI::Unitset vessels = getVessels(cluster);
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
//
//// Cast plague if possible and useful.
//void MicroDefilers::updatePlague(const UnitCluster & cluster)
//{
//    BWAPI::Unitset defilers = getDefilers(cluster);
//    if (defilers.empty())
//    {
//        return;
//    }
//    updateCasters(defilers);
//
//    for (BWAPI::Unit defiler : defilers)
//    {
//        if (!defiler->isBurrowed() &&
//            defiler->getEnergy() >= 150 &&
//            BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Plague) &&
//            defiler->canUseTech(BWAPI::TechTypes::Plague, defiler->getPosition()) &&
//            !isReadyToCastOtherThan(defiler, CasterSpell::Plague))
//        {
//            if (!maybePlague(defiler) && isReadyToCast(defiler))
//            {
//                clearReadyToCast(defiler);
//                return;     // only one defiler may cast per call, to reduce duplication
//            }
//        }
//    }
//}
