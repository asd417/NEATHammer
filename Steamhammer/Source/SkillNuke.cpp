#include "SkillNuke.h"
#include "The.h"
namespace UAlbertaBot
{
	SkillNuke::SkillNuke() : Skill("nuke")
	{

	}
	void SkillNuke::getData(GameRecord& record, const std::string& line)
	{
	}
	std::string SkillNuke::putData() const
	{
		return std::string();
	}
	bool SkillNuke::enabled() const
	{
		return the.self()->getRace() == BWAPI::Races::Terran;
	}
	void SkillNuke::initialize()
	{
	}
	void SkillNuke::update()
	{
	}
	//Both feasible() and good() has to pass to execute nuke
	bool SkillNuke::feasible() const
	{
		return the.my.completed.count(BWAPI::UnitTypes::Terran_Ghost) > 0;
	}
	bool SkillNuke::good() const
	{
		return the.my.completed.count(BWAPI::UnitTypes::Terran_Nuclear_Silo) > 0;
	}
	void SkillNuke::execute()
	{

	}
	void SkillNuke::draw() const
	{
	}
}