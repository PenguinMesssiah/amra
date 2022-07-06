#ifndef ASTAR_HPP
#define ASTAR_HPP

// project includes
#include <amra/types.hpp>

// system includes
#include <smpl/heap/intrusive_heap.h>

// standard includes
#include <vector>
#include <memory>

namespace AStar
{

struct AStarState
{
	int state_id;
	unsigned int g;
	AStarState* bp;
	int actionidx; //What is this action index used for?

	//Heap Element for Storing Heuristics
	struct HeapData : public smpl::heap_element
	{
		AStarState* me;
		unsigned int h;
		unsigned int f;
	}
	HeapData od[1]; // overallocated for additional n heuristics
};

class Environment;
class Heuristic;

class AStar : public Search
{
public:
	AStar(
		Environment* space,
		const std::vector<std::shared_ptr<Heuristic>>& heurs,
		const std::vector<int>& heurs_map,
		int heur_count);
	~AStar();

	int set_start(int start_id) override;
	int set_goal(int goal_id) override;

	int get_n_expands() const override;
	std::string get_expands_str() const;
	void reset() override;

private:
	//Environment & Heuristic Variables
	Environment* m_space = nullptr;

	std::vector<std::shared_ptr<Heuristic>> m_heurs;
	std::vector<std::pair<Resolution::Level, int> > m_heurs_map;
	int m_heur_count;

	//General Variables for AStar
	std::vector<AMRAState*> m_states, m_incons; //What is m_incons used for?
	AStarState *m_goal, *m_start;
	int m_start_id, m_goal_id;

	// Search statistics
	int m_solution_cost;
	int num_heuristics() const { return m_heurs_map.size(); }

};

} // namespace AStar

#endif  // ASTAR_HPP