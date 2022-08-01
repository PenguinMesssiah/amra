// project includes
#include <amra/grid2d_time.hpp>
#include <amra/heuristic.hpp>
#include <amra/dubins.hpp>
#include <amra/dijkstra.hpp>
#include <amra/constants.hpp>
#include <amra/amra.hpp>
#include <amra/arastar.hpp>
#include <amra/helpers.hpp>

// system includes
#include <smpl/console/console.h>

// standard includes
#include <fstream>
#include <stdexcept>

auto std::hash<AMRA::MapState>::operator()(
	const argument_type& s) const -> result_type
{
	size_t seed = 0;
	boost::hash_combine(seed, boost::hash_range(s.coord.begin(), s.coord.end()));
	//boost::hash_combine(seed, s.time);
	return seed;
}

namespace AMRA
{

Grid2D_Time::Grid2D_Time(const std::string& mapname, const int budget)
:
m_mapname(mapname),
m_start_set(false),
m_goal_set(false),
m_budget(100)
{
	m_map    = std::make_unique<MovingAI>(mapname);
	m_budget = budget;

	// reset everything
	for (MapState* s : m_states) {
		if (s != NULL) {
			delete s;
			s = nullptr;
		}
	}
	m_state_to_id.clear();
}

void Grid2D_Time::CreateSearch()
{
	/*
	if (GRID == 4) {
		m_heurs.emplace_back(new ManhattanDist(this));
	}
	else if (GRID == 8) {
		m_heurs.emplace_back(new EuclideanDist(this));
	}
	else {
		throw std::runtime_error("Invalid 2D grid. Must be 4- or 8-connected!");
	}
	m_heurs_map.emplace_back(Resolution::ANCHOR, 0); // anchor always goes first
	m_heurs_map.emplace_back(Resolution::HIGH, 0);
	m_res_count = 1; // inadmissible resolution count
	m_heur_count = 1;

	if (NUM_RES > 1)
	{
		m_heurs_map.emplace_back(Resolution::MID, 0);
		m_res_count++;
	}
	if (NUM_RES == 3)
	{
		m_heurs_map.emplace_back(Resolution::LOW, 0);
		m_res_count++;
	}

	if (DUBINS)
	{
		m_heur_count++;
		m_heurs.emplace_back(new Dubins(this));
		m_heurs_map.emplace_back(Resolution::HIGH, m_heurs.size()-1);
	}
	if (DIJKSTRA)
	{
		m_heur_count++;
		m_heurs.emplace_back(new Dijkstra(this, m_map.get()));
		m_heurs_map.emplace_back(Resolution::HIGH, m_heurs.size()-1);
	}

	for (int i = 0; i < m_heurs_map.size(); ++i) {
		m_closed[i].clear(); // init expansions container
	}

	m_search = std::make_unique<AMRAStar>(
		this, m_heurs, m_heurs_map,
		m_heur_count, m_res_count);
	m_search->reset();
	*/
}

void Grid2D_Time::CreateAStarSearch()
{
	// m_heurs.emplace_back(new EuclideanDist(this));
	m_heurs.emplace_back(new ManhattanDist(this));
	m_search = std::make_unique<ARAStar>(this, m_heurs.at(0));
	m_search->reset();
	//m_default_res = Resolution::HIGH;
}

void Grid2D_Time::SetStart(const int& d1, const int& d2)
{
	assert(!m_start_set);
	assert(m_map->IsTraversible(d1,d2));
	m_start_id = getOrCreateState(d1, d2, 0);
	m_start_set = true;

	m_s1 = d1;
	m_s2 = d2;
	printf("Start: {%d, %d},\t", m_s1, m_s2);
}

void Grid2D_Time::SetGoal(const int& d1, const int& d2)
{
	assert(!m_goal_set);
	assert(m_map->IsTraversible(d1,d2));
	m_goal_id = getOrCreateState(d1, d2, -1);
	m_goal_set = true;

	m_g1 = d1;
	m_g2 = d2;
	printf("Goal: {%d, %d},\n", m_g1, m_g2);
}

bool Grid2D_Time::Plan(bool save)
{
	int d1s, d2s, d1g, d2g;
	if (!m_start_set)
	{
		// set random start
		m_map->GetRandomState(d1s, d2s);
		m_start_id = getOrCreateState(d1s, d2s, 0);
		m_start_set = true;

		m_s1 = d1s;
		m_s2 = d2s;
		printf("Start: {%d, %d},\t", m_s1, m_s2);
	}

	if (!m_goal_set)
	{
		// set random goal
		do {
			m_map->GetRandomState(d1g, d2g);
		}
		while (d1g == d1s && d2g == d2s);

		m_goal_id = getOrCreateState(d1g, d2g, -1);
		m_goal_set = true;

		m_g1 = d1g;
		m_g2 = d2g;
		printf("Goal: {%d, %d},\n", m_g1, m_g2);
	}

	m_search->set_start(m_start_id);
	m_search->set_goal(m_goal_id);

	if (DIJKSTRA)
	{
		auto robot = getHashEntry(m_start_id);
		auto goal = getHashEntry(m_goal_id);
		m_heurs.back()->Init(robot->coord, goal->coord);
	}
	
	std::vector<int> solution;
	std::vector<int> action_ids;
    int solcost;
    
    bool result = m_search->replan(&solution, &action_ids, &solcost);
    if(!solution.empty())
    {
    	int path_length = getHashEntry(solution.at(solution.size()-1))->time;
    	printf("path Length = %d\n", path_length);	
    }
    
	if (result && save)
	{
		std::vector<MapState> solpath;
		convertPath(solution, solpath);

		// for (const auto& s: solpath) {
		// 	printf("%d,%d,%d\n", s.coord.at(0), s.coord.at(1), s.coord.at(2));
		// }
		// m_map->SavePath(solpath);

		// double initial_t, final_t;
		// int initial_c, final_c, total_e;
		// m_search->GetStats(initial_t, final_t, initial_c, final_c, total_e);

		// std::string filename(__FILE__);
		// auto found = filename.find_last_of("/\\");
		// filename = filename.substr(0, found + 1) + "../../dat/STATS.csv";

		// bool exists = FileExists(filename);
		// std::ofstream STATS;
		// STATS.open(filename, std::ofstream::out | std::ofstream::app);
		// if (!exists)
		// {
		// 	STATS << "TotalExpansions,"
		// 			<< "InitialSolutionTime,FinalSolutionTime,"
		// 			<< "InitialSolutionCost,FinalSolutionCost\n";
		// }
		// STATS << total_e << ','
		// 		<< initial_t << ','
		// 		<< final_t << ','
		// 		<< initial_c << ','
		// 		<< final_c << '\n';
		// STATS.close();

		//return true;
	}

	return solcost > 0;
}

void Grid2D_Time::GetSuccs(
	int state_id,
	Resolution::Level level,
	std::vector<int>* succs,
	std::vector<unsigned int>* costs,
	std::vector<int>* action_ids)
{
	assert(state_id >= 0);
	succs->clear();
	costs->clear();

	MapState* parent = getHashEntry(state_id);
	assert(parent);
	assert(m_map->IsTraversible(parent->coord.at(0), parent->coord.at(1)));
	m_closed[static_cast<int>(1)].push_back(parent);

	//printf("\nParent_t = %d", parent->time);
	// goal state should be absorbing
	if (state_id == GetGoalID()) {
		// SMPL_INFO("Expanding goal state (id = %d)!", GetGoalID());
		return;
	}

	for (int a1 = -1; a1 <= 1; ++a1)
	{
		for (int a2 = -1; a2 <= 1; ++a2)
		{
			// ignore ordinal directions for 4-connected grid
			if (GRID == 4 && std::abs(a1 * a2) == 1) {
				continue;
			}
			if (a1 == 0 && a2 == 0) {
				continue;
			}

			int succ_id = generateSuccessor(parent, a1, a2, succs, costs);
		}
	}
}

bool Grid2D_Time::IsGoal(const int& id)
{
	MapState state, goal;
	GetStateFromID(id, state);
	GetGoal(goal);

	return state.coord.at(0) == goal.coord.at(0) &&
			state.coord.at(1) == goal.coord.at(1) &&
			state.time <= m_budget;

	// return (id == m_goal_id) && (state == goal);
}

void Grid2D_Time::SaveExpansions(
	int iter, double w1, double w2,
	const std::vector<int>& curr_solution,
	const std::vector<int>& action_ids)
{
	m_map->SaveExpansions(iter, w1, w2, m_closed, true, m_budget);
	//m_map->SaveExpansions(iter, w1, w2, m_closed, true);
	for (int i = 0; i < 1; ++i) {
		m_closed[i].clear(); // init expansions container
	}

	std::vector<MapState> solpath;
	convertPath(curr_solution, solpath);
	m_map->SavePath(solpath, iter);
}

void Grid2D_Time::GetStart(MapState& start)
{
	GetStateFromID(m_start_id, start);
}

void Grid2D_Time::GetGoal(MapState& goal)
{
	GetStateFromID(m_goal_id, goal);
}

void Grid2D_Time::GetStateFromID(const int& id, MapState& state)
{
	MapState* hashentry = getHashEntry(id);
	state = *hashentry;
}

Resolution::Level Grid2D_Time::GetResLevel(const int& state_id)
{
	auto s = getHashEntry(state_id);
	assert(s);
	return s->level;
}

int Grid2D_Time::generateSuccessor(
	const MapState* parent,
	int a1, int a2,
	std::vector<int>* succs,
	std::vector<unsigned int>* costs)
{
	int parent_t = parent->time;
	if (parent_t + 1 > m_budget) {
		// printf("Hit Hard Constraint\n");
		return -1;
	}

	int succ_d1 = parent->coord.at(0) + a1;
	int succ_d2 = parent->coord.at(1) + a2;

	if (!m_map->IsTraversible(succ_d1, succ_d2)) {
		return -1;
	}

	//printf("Goal = (%d,%d)\n", m_g1, m_g2);
	//printf("Exp = (%d,%d,%d)\n", succ_d1, succ_d2);

	//Checking for Identical State Already Visited
	int status = getHashEntry(succ_d1, succ_d2);
	if(status !=  -1)
	{
    	MapState* temp = getHashEntry(status);

    	if(temp->time >= 0 && temp->time < parent_t+1)
    	{
        	//printf("Cell Already Visited {%d,%d}\n"
        	return -1;    
    	}
	}

	int succ_state_id = getOrCreateState(succ_d1, succ_d2, parent_t + 1);
	MapState* successor = getHashEntry(succ_state_id);

	succs->push_back(succ_state_id);
	costs->push_back(cost(parent, successor));

	return succ_state_id;
}

bool Grid2D_Time::convertPath(
	const std::vector<int>& idpath,
	std::vector<MapState>& path)
{
	std::vector<MapState> opath;

	if (idpath.empty()) {
		return true;
	}

	// attempt to handle paths of length 1...do any of the sbpl planners still
	// return a single-point path in some cases?
	if (idpath.size() == 1)
	{
		auto state_id = idpath[0];

		if (state_id == GetGoalID())
		{
			auto* entry = getHashEntry(GetStartID());
			if (!entry)
			{
				SMPL_ERROR("Failed to get state entry for state %d", GetStartID());
				return false;
			}
			opath.push_back(*entry);
		}
		else
		{
			auto* entry = getHashEntry(state_id);
			if (!entry)
			{
				SMPL_ERROR("Failed to get state entry for state %d", state_id);
				return false;
			}
			opath.push_back(*entry);
		}
	}

	if (idpath[0] == GetGoalID())
	{
		SMPL_ERROR("Cannot extract a non-trivial path starting from the goal state");
		return false;
	}

	// grab the first point
	{
		auto* entry = getHashEntry(idpath[0]);
		if (!entry)
		{
			SMPL_ERROR("Failed to get state entry for state %d", idpath[0]);
			return false;
		}
		opath.push_back(*entry);
	}

	// grab the rest of the points
	for (size_t i = 1; i < idpath.size(); ++i)
	{
		auto prev_id = idpath[i - 1];
		auto curr_id = idpath[i];

		if (prev_id == GetGoalID())
		{
			SMPL_ERROR("Cannot determine goal state predecessor state during path extraction");
			return false;
		}

		auto* entry = getHashEntry(curr_id);
		if (!entry)
		{
			SMPL_ERROR("Failed to get state entry state %d", curr_id);
			return false;
		}
		opath.push_back(*entry);
	}
	path = std::move(opath);
	return true;
}

// Return a pointer to the data for the input the state id
// if it exists, else return nullptr
MapState* Grid2D_Time::getHashEntry(int state_id) const
{
	if (state_id < 0 || state_id >= (int)m_states.size()) {
		return nullptr;
	}

	return m_states[state_id];
}

// Return the state id of the state with the given data or -1 if the
// state has not yet been allocated.
int Grid2D_Time::getHashEntry(
		const int& d1,
		const int& d2)
{
	MapState state;
	state.coord.resize(2, 0);
	state.coord.at(0) = d1;
	state.coord.at(1) = d2;

	auto sit = m_state_to_id.find(&state);
	if (sit == m_state_to_id.end()) {
		return -1;
	}
	return sit->second;
}

int Grid2D_Time::reserveHashEntry()
{
	MapState* entry = new MapState;
	entry->coord.resize(2, 0);

	int state_id = (int)m_states.size();

	// map state id -> state
	m_states.push_back(entry);

	// // map planner state -> graph state
	// int* pinds = new int[NUMOFINDICES_STATEID2IND];
	// std::fill(pinds, pinds + NUMOFINDICES_STATEID2IND, -1);
	// StateID2IndexMapping.push_back(pinds);

	return state_id;
}

int Grid2D_Time::createHashEntry(
		const int& d1,
		const int& d2,
		int  time)
{
	int state_id = reserveHashEntry();
	MapState* entry = getHashEntry(state_id);

	entry->coord.at(0) = d1;
	entry->coord.at(1) = d2;
	entry->time = time;
	
	/*
	if (NUM_RES == 3 &&
			(d1 % LOWRES_MULT == 0 && d2 % LOWRES_MULT == 0))
	{
		entry->level = Resolution::LOW;
	}
	else if (NUM_RES >= 2 &&
			(d1 % MIDRES_MULT == 0 && d2 % MIDRES_MULT == 0)) {
		entry->level = Resolution::MID;
	}
	else {
		entry->level = Resolution::HIGH;
	}
	*/

	// map state -> state id
	m_state_to_id[entry] = state_id;

	return state_id;
}

int Grid2D_Time::getOrCreateState(
		const int& d1,
		const int& d2,
		int time)
{
	int state_id = getHashEntry(d1, d2);
	if (state_id < 0) {
		state_id = createHashEntry(d1, d2, time);
	}
	return state_id;
}

unsigned int Grid2D_Time::cost(
	const MapState* s1,
	const MapState* s2)
{
	/*
		return the value of the cost between s1 and s2
		of the function i want to minimise
		i.e. the function wrt which i want to return the least cost path
	*/
	if (COSTMAP)
	{
		int dir1 = sgn(s2->coord.at(0) - s1->coord.at(0));
		int dir2 = sgn(s2->coord.at(1) - s1->coord.at(1));

		int h, w;
		auto map = m_map->GetMap();
		h = m_map->GetH();
		w = m_map->GetW();

		unsigned int cost = 0;
		for (int d1 = s1->coord.at(0) + dir1, d2 = s1->coord.at(1) + dir2;
					d1 != s2->coord.at(0) + dir1 || d2 != s2->coord.at(1) + dir2;
						d1 += dir1, d2 += dir2)
		{
			cost += map[GETMAPINDEX(d1, d2, h, w)];
		}
		return (cost * COST_MULT);
	}
	else
	{
		double dist = 0.0;
		for (size_t i = 0; i < s1->coord.size(); ++i) {
			dist += std::pow(s1->coord.at(i) - s2->coord.at(i), 2);
		}
		dist = std::sqrt(dist);
		return (dist * COST_MULT);
	}
}

}  // namespace AMRA
