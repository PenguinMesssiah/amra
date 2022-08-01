#ifndef GRID2D_TIME_HPP
#define GRID2D_TIME_HPP

// project includes
#include <amra/movingai.hpp>

// system includes
#include <smpl/types.h>
#include <time.h>

// standard includes
#include <memory>

namespace std {

template <>
struct hash<AMRA::MapState>
{
	typedef AMRA::MapState argument_type;
	typedef std::size_t result_type;
	result_type operator()(const argument_type& s) const;
};

} // namespace std

namespace AMRA
{

class Heuristic;

class Grid2D_Time : public Environment
{
public:
	Grid2D_Time(const std::string& mapname, const int budget);

	void SetStart(const int& d1, const int& d2);
	void SetGoal(const int& d1, const int& d2);

	void CreateSearch() override;
	void CreateAStarSearch();
	bool Plan(bool save=false) override;

	void GetSuccs(
		int state_id,
		Resolution::Level level,
		std::vector<int>* succs,
		std::vector<int_fast64_t>* costs_f0,
		std::vector<int_fast64_t>* costs_f1,
		std::vector<int>* action_ids) override;
	bool IsGoal(const int& id) override;
	
	void SaveExpansions(
		int iter, double w1, double w2, int m_weight,
		const std::vector<int>& curr_solution,
		const std::vector<int>& action_ids) override;

	void GetStart(MapState& start);
	void GetGoal(MapState& goal);
	void GetStateFromID(const int& id, MapState& state);

	Resolution::Level GetResLevel(const int& state_id) override;

private:
	std::string m_mapname;
	std::unique_ptr<MovingAI> m_map;

	std::vector<std::shared_ptr<Heuristic> > m_heurs;
	//std::vector<std::pair<Resolution::Level, int> > m_heurs_map;
	//int m_heur_count, m_res_count;
	//Resolution::Level m_default_res;

	int m_s1, m_s2, m_g1, m_g2, m_budget;
	bool m_start_set, m_goal_set;
	std::vector<MapState*> m_states;
	EXPANDS_t m_closed;

	// maps from coords to stateID
	typedef MapState StateKey;
	typedef smpl::PointerValueHash<StateKey> StateHash;
	typedef smpl::PointerValueEqual<StateKey> StateEqual;
	smpl::hash_map<StateKey*, int, StateHash, StateEqual> m_state_to_id;

	MapState* getHashEntry(int state_id) const;
	int getHashEntry(
		const int& d1,
		const int& d2);
	int reserveHashEntry();
	int createHashEntry(
		const int& d1,
		const int& d2,
		int time);
	int getOrCreateState(
		const int& d1,
		const int& d2,
		int time);
	int createGoalState(
		const int& d1,
		const int& d2,
		int time);

	int generateSuccessor(
		const MapState* parent,
		int a1, int a2,
		std::vector<int>* succs,
		std::vector<int_fast64_t>* costs_f0,
		std::vector<int_fast64_t>* costs_f1);
	std::vector<int_fast64_t> cost(
		const MapState* s1,
		const MapState* s2);

	bool convertPath(
		const std::vector<int>& idpath,
		std::vector<MapState>& path);

	void resetAll();
	void resetStartGoalStates();
};

}  // namespace AMRA

#endif  // GRID2D_HPP
