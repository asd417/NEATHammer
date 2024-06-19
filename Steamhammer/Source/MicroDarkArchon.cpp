#include "MicroManager.h"
#include "MicroDarkArchon.h"

#include "Bases.h"
#include "The.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

#define UNITTYPE BWAPI::UnitTypes::Protoss_Dark_Archon
#define SPELL1TECHTYPE BWAPI::TechTypes::Maelstrom
#define SPELL2TECHTYPE BWAPI::TechTypes::Feedback

#define CASTERSPELLTYPE1 CasterSpell::MaelStrom
#define CASTERSPELLTYPE2 CasterSpell::Feedback

#define MAELSTROMENERGY 100
#define FEEDBACKENERGY 50
#define SPELLRANGE 10
#define SPELLRADIUS 1

void MicroDarkArchon::updateMovement(const UnitCluster& cluster, BWAPI::Unit vanguard)
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
                if ((templar->getEnergy() < MAELSTROMENERGY || templar->getEnergy() < FEEDBACKENERGY)&& cluster.size() > getUnits().size())
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

int MicroDarkArchon::maelstromScore(BWAPI::Unit u) const
{
    if (!u->getType().isOrganic() || u->isBurrowed()) return 0;
    return 100;
}

int MicroDarkArchon::feedbackScore(BWAPI::Unit u) const
{
    if (u->isStasised() || u->getType() == BWAPI::UnitTypes::Zerg_Broodling) return 0;
    return u->getEnergy();
}

// The defiler is probably about to die. It should cast immediately if it is ever going to.
bool MicroDarkArchon::aboutToDie(const BWAPI::Unit templar) const
{
    return templar->getShields() < 30;
}

void UAlbertaBot::MicroDarkArchon::executeMicro(const BWAPI::Unitset& targets, const UnitCluster& cluster)
{
}

UAlbertaBot::MicroDarkArchon::MicroDarkArchon()
{
}
BWAPI::Unitset MicroDarkArchon::getCaster(const UnitCluster& cluster) const
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

bool MicroDarkArchon::maybeSpell(BWAPI::Unit templar)
{
    int limit = SPELLRANGE + SPELLRADIUS;

    const bool dying = aboutToDie(templar);

    // Don't bother to look for units to plague if no enemy is close enough.
    BWAPI::Unit closest = BWAPI::Broodwar->getClosestUnit(templar->getPosition(),
        BWAPI::Filter::IsEnemy && !BWAPI::Filter::IsOrganic,
        limit * 32);

    if (!dying && !closest)
    {
        return false;
    }

    // Look for the box with the best effect.
    int bestMaelstromScore = 0;
    BWAPI::Position bestMaelstromPlace;

    int bestFeedBackScore = 0;
    BWAPI::Unit bestFeedbackTarget;

    for (int tileX = std::max(2, templar->getTilePosition().x - limit); tileX <= std::min(BWAPI::Broodwar->mapWidth() - 3, templar->getTilePosition().x + limit); ++tileX)
    {
        for (int tileY = std::max(2, templar->getTilePosition().y - limit); tileY <= std::min(BWAPI::Broodwar->mapHeight() - 3, templar->getTilePosition().y + limit); ++tileY)
        {
            int m_score = 0;
            int f_score = 0;
            BWAPI::Position place(BWAPI::TilePosition(tileX, tileY));
            const BWAPI::Position offset(SPELLRADIUS * 32, SPELLRADIUS * 32);
            BWAPI::Unitset affected = BWAPI::Broodwar->getUnitsInRectangle(place - offset, place + offset);
            for (BWAPI::Unit u : affected)
            {
                if (u->getPlayer() == BWAPI::Broodwar->self())
                {
                    m_score -= maelstromScore(u);
                }
                else if (u->getPlayer() == BWAPI::Broodwar->enemy())
                {
                    m_score += maelstromScore(u);
                }
                if (feedbackScore(u) > bestFeedBackScore)
                {
                    bestFeedBackScore = f_score;
                    bestFeedbackTarget = u;
                }
            }
            if (m_score > bestMaelstromScore)
            {
                bestMaelstromScore = m_score;
                bestMaelstromPlace = place;
            }
        }
    }
    if (bestFeedBackScore > bestMaelstromScore && bestFeedBackScore > 20)
    {
        setReadyToCast(templar, CasterSpell::Feedback);
        return spell(templar, BWAPI::TechTypes::Feedback, bestFeedbackTarget);
    }
    else if (bestMaelstromScore > 100 || dying && bestMaelstromScore > 0)
    {
        setReadyToCast(templar, CasterSpell::MaelStrom);
        return spell(templar, BWAPI::TechTypes::Maelstrom, bestMaelstromPlace);
    }
    return false;
}
void MicroDarkArchon::updateSpell(const UnitCluster& cluster)
{
    BWAPI::Unitset casters = getCaster(cluster);
    if (casters.empty()) return;
    updateCasters(casters);

    for (BWAPI::Unit caster : casters)
    {
        if (caster->getEnergy() >= MAELSTROMENERGY &&
            BWAPI::Broodwar->self()->hasResearched(SPELL1TECHTYPE) &&
            caster->canUseTech(SPELL1TECHTYPE, caster->getPosition()) &&
            !isReadyToCastOtherThan(caster, CASTERSPELLTYPE1))
        {
            if (!maybeSpell(caster) && isReadyToCast(caster))
            {
                clearReadyToCast(caster);
                return;     // only one defiler may cast per call, to reduce duplication
            }
        }

        if (caster->getEnergy() >= FEEDBACKENERGY &&
            caster->canUseTech(SPELL2TECHTYPE, caster->getPosition()) &&
            !isReadyToCastOtherThan(caster, CASTERSPELLTYPE2))
        {
            if (!maybeSpell(caster) && isReadyToCast(caster))
            {
                clearReadyToCast(caster);
                return;     // only one defiler may cast per call, to reduce duplication
            }
        }
    }
}
