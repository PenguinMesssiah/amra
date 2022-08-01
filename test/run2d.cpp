// project includes
#include <amra/grid2d_time.hpp>

// system includes

// standard includes
#include <vector>

using namespace AMRA;

/*
 *@param argv[1] mapfile name/path
 *@param argv[2] global constraint for A* Search  
*/
int main(int argc, char** argv)
{
	std::string mapfile(argv[1]);
	int budget = std::stoi(argv[2]);

	/* Single Step:
	*  run 2D gridworld planning for random starts and goals
	*/
	
	Grid2D_Time grid(mapfile, budget);
	grid.CreateAStarSearch();
	grid.SetStart(125, 45);
	grid.SetGoal(600, 841);
	bool status = grid.Plan(true);
	
	
	/* Continuous Step:
	*  run 2D gridworld planning for random starts and goals

	for (int i = budget; i > 0; i--)
	{
		Grid2D_Time grid(mapfile, i);
		grid.CreateAStarSearch();
		grid.SetStart(25, 5);
		grid.SetGoal(25, 45);
		bool status = grid.Plan(true);
		if(!status)
			break;
	}
	*/
	
}
