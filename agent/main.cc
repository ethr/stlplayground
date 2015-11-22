#include <string>
#include <memory>
#include <deque>
#include <algorithm>
#include <iostream>
#include <array>
#include <cassert>
#include <queue>
#include <functional>
#include <chrono>
#include <utility>

/**
 * A solver for a very basic puzzle. Take a 4x4 board like the on below.
 * 'a','b','c' are moveable blocks and '*' is an agent who can push the blocks.
 * Essentially when '*' moves either up, down, left or right the tile in the
 * direction gets swapped with the agent. The aim is the move the tiles to the
 * goal state as shown.
 *
 * Example start:
 * |  a |
 * |b c |
 * |   *|
 * Example goal:
 * |abc*|
 * |    |
 * |    |
 *
 * The code below solves includes a slow breadth first solver and a much more
 * efficient A* solver. Originally this was written using Euclidean distance as
 * a heuristic but using Manhattan 'city block' Distance gave much better
 * results in terms of execution time. Also hashing was used to prevent cycling
 * in solutions.
 *
 * Algorithm for A* search:
 * - Pop 'best' current solution from priority queue
 * - For each possible direction
 *   - Test its a valid direction that doesn't loop back to a previous search
 *   state
 *   - Push onto priority queue
 *
 * Compile with:
 *   g++ -Wall main.cc -std=c++11 -O3 && ./a.out
 */

class PuzzleState;
typedef std::shared_ptr<PuzzleState> PuzzleStatePtr;
typedef std::shared_ptr<const PuzzleState> PuzzleStateConstPtr;

class Timer {
  public:

    void start() {
      t1_ =std::chrono::high_resolution_clock::now();
    }

    void stop() {
      t2_ =std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span =
        std::chrono::duration_cast<std::chrono::duration<double>>(t2_ - t1_);
      time_ = time_span.count();
    }

    double time() const {
      return time_;
    }

  private:
    double time_;
    std::chrono::high_resolution_clock::time_point t1_;
    std::chrono::high_resolution_clock::time_point t2_;
};

namespace Move {
  enum type_t {
    LEFT,
    UP,
    RIGHT,
    DOWN,
    UNKNOWN
  };
}

typedef std::array<char, 16> Board;

size_t make_hash(const Board& board) {
  std::string str(board.begin(), board.end());
  return std::hash<std::string>()(str);
}

class PuzzleState {
  public:
    explicit PuzzleState(const Board& state) :
      hash_(make_hash(state)),
      state_(state)
    {}

    PuzzleState(PuzzleStatePtr parent, const Board& state) :
      hash_(make_hash(state)),
      state_(state),
      parent_(parent)
    {}

    const Board& getState() const {
      return state_;
    }

    PuzzleStateConstPtr getParent() const {
      return parent_;
    }

    size_t getHash() const {
      return hash_;
    }
  private:
    size_t hash_;
    Board state_;
    PuzzleStatePtr parent_;
};

std::ostream& operator<<(std::ostream& os, PuzzleState state) {
  const Board& s = state.getState();
  unsigned k = 0;
  for (unsigned i = 0; i < 4; ++i) {
    std::cout << '|';
    for (unsigned j = 0; j < 4; ++j) {
      std::cout << s[k] << ", ";
      ++k;
    }
    std::cout << '|' << std::endl;
  }
  return os;
}

PuzzleStatePtr make_next_state(PuzzleStatePtr parent, Move::type_t move) {
  Board state(parent->getState());
  Board::const_iterator starPos = std::find(state.begin(), state.end(),
      '*');
  size_t index = starPos - state.begin();
  size_t x = index % 4;
  size_t y = index / 4;
  switch (move) {
    case Move::LEFT:
      if (x == 0) {
        return PuzzleStatePtr();
      }
      --x;
      break;
    case Move::RIGHT:
      if (x == 3) {
        return PuzzleStatePtr();
      }
      ++x;
      break;
    case Move::UP:
      if (y == 0) {
        return PuzzleStatePtr();
      }
      --y;
      break;
    case Move::DOWN:
      if (y == 3) {
        return PuzzleStatePtr();
      }
      ++y;
      break;
    default:
      assert(false); // TODO
  }
  size_t newIndex = y * 4 + x;
  std::swap(state[index], state[newIndex]);
  size_t hash = make_hash(state);

  if (parent->getParent()) {
    PuzzleStateConstPtr ansector = parent->getParent();
    do {
      if (hash == ansector->getHash()) {
        return PuzzleStatePtr();
      }
    }
    while (ansector = ansector->getParent());
  }

  return PuzzleStatePtr(new PuzzleState(parent, state));
}

PuzzleStatePtr breadthFirst(const Board& start, const Board& goal) {
  size_t goalHash = make_hash(goal);
  std::deque<PuzzleStatePtr> search;
  PuzzleStatePtr startState(new PuzzleState(start));
  if (startState->getHash() == goalHash) {
    return startState;
  }
  search.push_front(startState);
  unsigned attempts = 0;
  while (!search.empty()) {
    PuzzleStatePtr parent = search.back();
    search.pop_back();
    for (int m = 0; m < 4; ++m) {
      PuzzleStatePtr next = make_next_state(parent,
          static_cast<Move::type_t>(m));
      if (next) {
        if (next->getHash() == goalHash) {
          return next;
        }
        else {
          search.push_front(next);
        }
      }
    }
    ++attempts;
  }
  return PuzzleStatePtr();
}

typedef std::pair<PuzzleStatePtr, unsigned> PuzzleAndScore;

unsigned calc_score(const PuzzleStatePtr& puzzle, const Board& goal) {
  const Board& state = puzzle->getState();
  unsigned ret = 0;
  for (size_t i = 0; i < goal.size(); ++i) {
    char c = goal[i];
    if (state[i] != c) {
      size_t indexP = std::find(state.begin(), state.end(), c) - state.begin();
      //ret += (indexP - i) * (indexP - i);
      ret += std::abs(indexP - i); // Manhattan Distance
    }
  }
  return ret;
}

struct Comp {
  bool operator()(const PuzzleAndScore& lhs, const PuzzleAndScore& rhs) const {
    return lhs.second > rhs.second;
  }
};

PuzzleStatePtr aStar(const Board& start, const Board& goal) {
  size_t goalHash = make_hash(goal);
  std::priority_queue<PuzzleAndScore,
    std::vector<PuzzleAndScore>, Comp> search;
  PuzzleStatePtr startState(new PuzzleState(start));
  if (startState->getHash() == goalHash) {
    return startState;
  }
  search.push(std::make_pair(startState, calc_score(startState, goal)));
  unsigned attempts = 0;
  while (!search.empty()) {
    PuzzleAndScore best = search.top();
    search.pop();
    for (int m = 0; m < 4; ++m) {
      PuzzleStatePtr next = make_next_state(best.first,
          static_cast<Move::type_t>(m));
      if (next) {
        if (next->getHash() == goalHash) {
          return next;
        }
        else {
          search.push(std::make_pair(next, calc_score(next, goal)));
        }
      }
    }
    ++attempts;
  }
  return PuzzleStatePtr();
}

PuzzleStatePtr do_searh(const char* start, const char* goal) {
  Board startBoard;
  std::copy(start, start + 16, startBoard.begin());
  Board goalBoard;
  std::copy(goal, goal + 16, goalBoard.begin());
  PuzzleStatePtr finish = aStar(startBoard, goalBoard);
  //PuzzleStatePtr finish = breadthFirst(startBoard, goalBoard);
  return finish;
}

void print_search(PuzzleStatePtr finish) {
  if (finish) {
    std::cout << "Finish!" << std::endl;
    std::vector<PuzzleStateConstPtr> solution;
    PuzzleStateConstPtr stage = finish;
    do {
      solution.push_back(stage);
    }
    while (stage = stage->getParent());
    std::for_each(solution.rbegin(), solution.rend(),
        [=] (const PuzzleStateConstPtr& stage) {
          std::cout << *stage << std::endl;
        }
    );
  }
  else {
    std::cout << "Failed" << std::endl;
  }
}

int main() {
  //                  "1234567890123456"
  const char* start = "a   *    b c    ";
  const char*  goal = "abc*            ";
  Timer timer;
  timer.start();
  for (unsigned i = 0; i < 1000; ++i) {
    do_searh(start, goal);
  }
  timer.stop();
  std::cout << "Time taken: " << timer.time() << "ms" << std::endl;
}
