// project includes
#include <amra/uav_env.hpp>
#include <amra/heuristic.hpp>
#include <amra/constants.hpp>
#include <amra/amra.hpp>
#include <amra/wastar.hpp>

// system includes
#include <smpl/console/console.h>

// standard includes
#include <fstream>
#include <sstream>

auto std::hash<AMRA::UAVState>::operator()(
    const argument_type& s) const -> result_type
{
    size_t seed = 0;
    boost::hash_combine(seed, boost::hash_range(s.coord.begin(), s.coord.end()));
    return seed;
}

auto split(std::string& s, char delim) -> std::vector<std::string>
{
    std::string item;
    std::stringstream ss(s);
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

namespace AMRA
{

UAVEnv::UAVEnv(const std::string& mapname)
:
m_mapname(mapname),
m_start_set(false),
m_goal_set(false)
{
    m_map = std::make_unique<MovingAI>(mapname);

    // reset everything
    for (auto* s : m_states) {
        if (s != NULL) {
            delete s;
            s = nullptr;
        }
    }
    m_state_to_id.clear();
}

void UAVEnv::SetStart(ContState& startState)
{
    assert(!m_start_set);

    DiscState coords =
    {
        (int)startState[0],
        (int)startState[1],
        (int)startState[2],
        (int)startState[3]
    };

    m_start_id = getOrCreateState(coords);
    m_start_set = true;

    auto* state = getHashEntry(m_start_id);
    printf("Set start (id = %d): %d, %d, %d, %d\n", m_start_id, state->coord[0], state->coord[1], state->coord[2], state->coord[3]);
    printf("Resolution::Level: %d\n", state->level);
}

void UAVEnv::SetGoal(ContState& goalState)
{
    assert(!m_goal_set);

    DiscState coords =
    {
        (int)goalState[0],
        (int)goalState[1],
        (int)goalState[2],
        (int)goalState[3]
    };

    m_goal_id = getOrCreateState(coords);
    m_goal_set = true;

    auto* state = getHashEntry(m_goal_id);
    printf("Set goal (id = %d): %d, %d, %d, %d\n", m_goal_id, state->coord[0], state->coord[1], state->coord[2], state->coord[3]);
    printf("Resolution::Level: %d\n", state->level);
}

void UAVEnv::ReadMprims(std::string& mprimfile)
{
    m_actions.clear();

    std::string line;
    char split_char = ' ';
    std::vector<std::string> tokens;

    int numIntermediatePoses = 0;
    int mprimCount = 0;

    // read line
    std::ifstream mprimFileStream(mprimfile);
    while (std::getline(mprimFileStream, line))
    {
        std::istringstream lineStream(line);
        std::string field;

        Action currentAction;

        // parse current line
        while (lineStream >> field)
        {
            // std::cout << "field: " << field << std::endl;
            if (field == "resolution_m:")
            {
                std::string resolution;
                lineStream >> resolution;
                double res1 = std::stod(resolution);
                lineStream >> resolution;
                double res2 = std::stod(resolution);
            }
            else if (field == "numberofangles:")
            {
                std::string num;
                lineStream >> num;
                m_totalAngles = std::stoi(num);
            }
            else if (field == "totalnumberofprimitives:")
            {
                std::string prims;
                lineStream >> prims;
                m_totalPrims = std::stoi(prims);
                m_primsPerAngle = m_totalPrims / m_totalAngles;
            }
            else if (field == "numberofhighresprimitives:")
            {
                std::string high_res_prims;
                lineStream >> high_res_prims;
                m_numHighResPrims = std::stoi(high_res_prims);
            }
            else if (field == "numberofmidresprimitives:")
            {
                std::string mid_res_prims;
                lineStream >> mid_res_prims;
                m_numMidResPrims = std::stoi(mid_res_prims);
            }
            else if (field == "primID:")
            {
                std::string id;
                lineStream >> id;
                // currentAction.reset();
                currentAction.primID = std::stoi(id);
            }
            else if (field == "startangle_c:")
            {
                std::string angle;
                lineStream >> angle;
            }
            else if (field == "endpose_c:")
            {
                std::string dummy;
                lineStream >> dummy;
                lineStream >> dummy;
                lineStream >> dummy;
                lineStream >> dummy;
            }
            else if (field == "start_velocity:")
            {
                std::string dummy;
                lineStream >> dummy;
            }
            else if (field == "end_velocity:")
            {
                std::string dummy;
                lineStream >> dummy;
            }
            else if (field == "duration:")
            {
                std::string dummy;
                lineStream >> dummy;
            }
            else if (field == "intermediateposes:")
            {
                std::string num;
                lineStream >> num;
                numIntermediatePoses = std::stoi(num);
            }
            else
            {
                // intermediate continuous states
                int count = 0;
                do {
                    auto poses = split(line, split_char);
                    ContState state = {
                        std::stod(poses[0]),
                        std::stod(poses[1]),
                        std::stod(poses[2]),
                        std::stod(poses[3])
                    };
                    currentAction.intermediateStates.push_back(state);
                    if (count++ == numIntermediatePoses - 1)
                        break;
                } while (std::getline(mprimFileStream, line));
                storeAction(currentAction);
                ++mprimCount;
                break;
            }
        }
    }
    assert(mprimCount == m_totalPrims);
}

void UAVEnv::storeAction(Action& action)
{
    auto last_int_state = action.intermediateStates.back();
    auto first_int_state = action.intermediateStates.front();
    action.start = {
        (int)first_int_state[0],
        (int)first_int_state[1],
        (int)first_int_state[2],
        (int)first_int_state[3]
    };
    action.end = {
        (int)last_int_state[0],-
        (int)last_int_state[1],
        (int)last_int_state[2],
        (int)last_int_state[3]
    };
    m_actions.push_back(action);
}

/// For every angle, first 9 mprims are of resolution 3m and next 9 are of
/// resolution 9m. For e.g., if disc_angle = 2, primID 0 to 7 (i.e. action idx
/// 2*18+0 = 36 to 2*18+8 = 44) are mprims of 3m resolution.
int UAVEnv::getActionIdx(int& disc_angle, int& primID)
{
    return m_primsPerAngle * disc_angle + primID;
}

void UAVEnv::CreateSearch()
{
    m_heurs.emplace_back(new EuclideanDist(this));
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

    for (int i = 0; i < m_heurs_map.size(); ++i) {
        m_closed[i].clear(); // init expansions container
    }

    m_search = std::make_unique<AMRAStar>(
        this, m_heurs, m_heurs_map,
        m_heur_count, m_res_count);
    m_search->reset();
}

bool UAVEnv::Plan(bool save)
{
    return false;
}

void UAVEnv::GetSuccs(
    int state_id,
    Resolution::Level level,
    std::vector<int>* succs,
    std::vector<unsigned int>* costs)
{
    assert(state_id >= 0);
    succs->clear();
    costs->clear();

    UAVState* parent = getHashEntry(state_id);
    assert(parent);
    assert(m_map->IsTraversible(parent->coord.at(0), parent->coord.at(1)));
    m_closed[static_cast<int>(level)].push_back(parent);

    // goal state should be absorbing
    if (state_id == GetGoalID()) {
        SMPL_INFO("Expanding goal state (id = %d)!", GetGoalID());
        return;
    }

    int grid_res;
    int primid_start;
    switch (level)
    {
        case Resolution::ANCHOR:
        case Resolution::HIGH: {
            grid_res = 1;
            break;
        }
        case Resolution::MID: {
            grid_res = MIDRES_MULT;
            primid_start = 0;
            break;
        }
        case Resolution::LOW: {
            grid_res = LOWRES_MULT;
            primid_start = 9;
            break;
        }
    }

    // Apply motion primitives to generate successors of resolution grid_res.
    // If anchor, also generate successors for other resolutions.
    int parent_disc_angle = parent->coord.at(2);
    for (int primid = primid_start; primid < primid_start + 8; ++primid)
    {
        int actionidx = getActionIdx(parent_disc_angle, primid);
        auto action = m_actions.at(actionidx);

        // collision-check action

        // successor state
        DiscState succCoords = {
            parent->coord.at(0) + action.end.at(0), // x
            parent->coord.at(1) + action.end.at(1), // y
            action.end.at(2),                       // theta
            action.end.at(3)                        // velocity
        };

        if (!m_map->IsTraversible(succCoords.at(0), succCoords.at(1))) {
            printf("  successor (%d, %d) + (%d, %d) = (%d, %d) not traversable\n",
                parent->coord.at(0), parent->coord.at(1),
                action.end.at(0), action.end.at(1),
                succCoords.at(0), succCoords.at(1));
            continue;
        }

        int succ_state_id = getOrCreateState(succCoords);
        succs->push_back(succ_state_id);
        costs->push_back(10); // TODO: add action costs

        printf("  successor (%d, %d) + (%d, %d) = (%d, %d) generated\n",
                parent->coord.at(0), parent->coord.at(1),
                action.end.at(0), action.end.at(1),
                succCoords.at(0), succCoords.at(1));
    }
}

bool UAVEnv::IsGoal(const int& id)
{
    return false;
}

void UAVEnv::SaveExpansions(
    int iter, double w1, double w2,
    const std::vector<int>& curr_solution)
{
    printf("UAVEnv::SaveExpansions not implemented\n");
}

void UAVEnv::GetStart(MapState& start)
{
    printf("Must not be called as MapState not un UAVEnv\n");
    assert(false);
}

void UAVEnv::GetGoal(MapState& goal)
{
    printf("Must not be called as MapState not un UAVEnv\n");
    assert(false);
}

void UAVEnv::GetStateFromID(const int& id, MapState& state)
{
    printf("Must not be called as MapState not un UAVEnv\n");
    assert(false);
}

Resolution::Level UAVEnv::GetResLevel(const int& state_id)
{
    UAVState s; return s.level;
}

// Return a pointer to the data for the input the state id
// if it exists, else return nullptr
UAVState* UAVEnv::getHashEntry(int state_id) const
{
    if (state_id < 0 || state_id >= (int)m_states.size()) {
        return nullptr;
    }
    return m_states[state_id];
}

// Return the state id of the state with the given data or -1 if the
// state has not yet been allocated.
int UAVEnv::getHashEntry(DiscState& inCoords)
{
    UAVState state;
    state.coord = inCoords;

    auto sit = m_state_to_id.find(&state);
    if (sit == m_state_to_id.end()) {
        return -1;
    }
    return sit->second;
}

int UAVEnv::getOrCreateState(DiscState& inCoords)
{
    int state_id = getHashEntry(inCoords);
    if (state_id < 0) {
        state_id = createHashEntry(inCoords);
    }
    return state_id;
}

int UAVEnv::createHashEntry(DiscState& inCoords)
{
    int state_id = reserveHashEntry();
    UAVState* entry = getHashEntry(state_id);

    entry->coord = inCoords;

    int d1 = inCoords.at(0); // x
    int d2 = inCoords.at(1); // y

    assert(NUM_RES == 2);
    if (d1 % LOWRES_MULT == 0 && d2 % LOWRES_MULT == 0) {
        entry->level = Resolution::LOW;
    } else if (d1 % MIDRES_MULT == 0 && d2 % MIDRES_MULT == 0) {
        entry->level = Resolution::MID;
    } else {
        printf("d1, d2 = %d, %d\n", d1, d2);
        assert(false && "Aaaaa");
    }

    // map state -> state id
    m_state_to_id[entry] = state_id;

    return state_id;
}

int UAVEnv::reserveHashEntry()
{
    UAVState* entry = new UAVState;
    entry->coord.resize(2, 0);

    int state_id = (int)m_states.size();

    // map state id -> state
    m_states.push_back(entry);

    return state_id;
}


}  // namespace AMRA
