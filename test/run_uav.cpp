// project includes
#include <amra/uav_env.hpp>

// system includes

// standard includes
#include <vector>

using namespace AMRA;

int main(int argc, char** argv)
{
    std::string mapfile(argv[1]);

    UAVEnv uav_env(mapfile);

    std::string mprimfile = "../dat/mprim/mhi_3m_9m.mprim";
    uav_env.ReadMprims(mprimfile);

    uav_env.CreateSearch();

    ContState goal = { 90.3, 90.4, 0.0, 0.0 };
    uav_env.SetGoal(goal);

    ContState start = { 3.5, 9.5, 0.0, 0.0 };
    uav_env.SetStart(start);

    uav_env.Plan(true);
}
