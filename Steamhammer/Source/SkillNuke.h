#include "Skill.h"
namespace UAlbertaBot
{
	class SkillNuke : public Skill {
        bool _began;

	public:
		SkillNuke();

        void getData(GameRecord& record, const std::string& line);
        std::string putData() const;

        bool enabled() const;
        void initialize();
        void update();
        bool feasible() const;
        bool good() const;
        void execute();
        void draw() const;
	};
}