#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <list>

#include "search.h"

constexpr int VALUE_MATE = 10000;
constexpr int VALUE_INFINITE = 10001;

std::uint64_t cutoffs, nodes;
States originalStates, states;
int n = 40;

// Function prototypes
int short_player(Stack* ss, int alpha, int beta);
int long_player(Stack* ss, int alpha, int beta);
int short_short_player(Stack* ss, int alpha, int beta);
int long_long_player(Stack* ss, int alpha, int beta);

// Calculate the 'connected' numbers
void generate_integer_divisors_and_multiples() {

  states.reserve(n + 1);

  // Insert dummy entries for n=0 and n=1
  for (int i = 0; i < 2; ++i) {
      states.push_back(Sequence());
  }

  for (int i = 2; i <= n; i++)
  {
      Sequence seq;
      seq.count = 0;
      seq.diff = 0;

      // Adds all integer divisors and multiples in ascending order
      for (int k = 2; k <= n; k++)
      {
          if ((k <= i && i % k == 0) || (k > i && k % i == 0))
          {
              seq.l.push_back(k);
              seq.count++;
          }
      }

      // Activate
      seq.isActive = true;

      states.push_back(seq);
  }

  assert(states.size() == n + 1);
}

// Returns the next best move for short player (highest count first).
// Returns -1 if no more moves available.
int get_next_best_short_move() {

    int bestIndex = -1;
    int bestCount = -1;
    
    for (int i = 2; i < states.size(); i++)
    {
        if (!states[i].isActive || !states[i].count)
            continue;

        if (states[i].count > bestCount)
        {
            bestCount = states[i].count;
            bestIndex = i;
        }
    }
    
    return bestIndex;
}

// Returns the next best move for long player (lowest count first).
// Returns -1 if no more moves available.
int get_next_best_long_move() {

    int bestIndex = -1;
    int bestCount = 100000;
    
    for (int i = states.size() - 1; i >= 2; i--)
    {
        if (!states[i].isActive || !states[i].count)
            continue;

        if (states[i].count < bestCount)
        {
            bestCount = states[i].count;
            bestIndex = i;
        }

        if (bestCount == 1)
            break;
    }
    
    return bestIndex;
}

// For easier output of a list
std::ostream& operator<<(std::ostream& ostr, const std::list<int>& list)
{
  for (const int i : list)
      ostr << ' ' << i;

  return ostr;
}

int main(int argc, char* argv[]) {

  // Parse command-line argument for n
  if (argc > 1)
  {
      n = std::atoi(argv[1]);

      if (n < 2)
      {
          std::cerr << "Error: n must be at least 2" << std::endl;
          return 1;
      }
  }

  Stack stack[MAX_PLY] = {};
  Stack* ss = stack;

  PVMoves pv; // Will contain the complete PV
  ss->pv = &pv;

  for (int i = 0; i < MAX_PLY; i++)
      (ss+i)->ply = i;

  generate_integer_divisors_and_multiples();

  originalStates = states;
  nodes = cutoffs = 0;
  auto start1 = std::chrono::high_resolution_clock::now();

  int minminBestValue = short_short_player(ss, 0, VALUE_INFINITE);

  auto end1 = std::chrono::high_resolution_clock::now();
  auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
  int nps1 = nodes * 1000 / (duration1.count() + 1);

  std::cout << "\nBest short-short play for n = " << n << " is " << VALUE_MATE - minminBestValue << " plies" << std::endl;
  std::cout << "Time: " << duration1.count() << " ms" << "   Nodes: " << nodes << "   Nodes per second: " << nps1 << "   Cutoffs: " << cutoffs << std::endl;
  std::cout << "PV:";
  for (int m : *(ss->pv))
      std::cout << " " << m;
  std::cout << std::endl;

  states = originalStates;
  nodes = cutoffs = 0;
  auto start2 = std::chrono::high_resolution_clock::now();

  int maxmaxBestValue = long_long_player(ss, -VALUE_INFINITE, 0);

  auto end2 = std::chrono::high_resolution_clock::now();
  auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
  int nps2 = nodes * 1000 / (duration2.count() + 1);

  std::cout << "\nBest long-long play for n = " << n << " is " << VALUE_MATE + maxmaxBestValue << " plies" << std::endl;
  std::cout << "Time: " << duration2.count() << " ms" << "   Nodes: " << nodes << "   Nodes per second: " << nps2 << "   Cutoffs: " << cutoffs << std::endl;
  std::cout << "PV:";
  for (int m : *(ss->pv))
      std::cout << " " << m;
  std::cout << std::endl;


/*
  states = originalStates;
  nodes = cutoffs = 0;
  auto start1 = std::chrono::high_resolution_clock::now();

  int minmaxBestValue = short_player(ss, 0, VALUE_INFINITE); // short player begins

  auto end1 = std::chrono::high_resolution_clock::now();
  auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
  int nps1 = nodes * 1000 / (duration1.count() + 1);

  std::cout << "\nBest short-long play for n = " << n << " is " << VALUE_MATE - minmaxBestValue << " plies" << std::endl;
  std::cout << "Time: " << duration1.count() << " ms" << "   Nodes: " << nodes << "   Nodes per second: " << nps1 << "   Cutoffs: " << cutoffs << std::endl;
  std::cout << "PV:";
  for (int m : *(ss->pv))
      std::cout << " " << m;
  std::cout << std::endl;

  nodes = cutoffs = 0;
  auto start2 = std::chrono::high_resolution_clock::now();

  int maxminBestValue = long_player(ss, -VALUE_INFINITE, 0); // long player begins

  auto end2 = std::chrono::high_resolution_clock::now();
  auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
  int nps2 = nodes * 1000 / (duration2.count() + 1);

  std::cout << "\nBest long-short play for n = " << n << " is " << VALUE_MATE + maxminBestValue << " plies" << std::endl;
  std::cout << "Time: " << duration2.count() << " ms" << "   Nodes: " << nodes << "   Nodes per second: " << nps2 << "   Cutoffs: " << cutoffs << std::endl;
  std::cout << "PV:";
  for (int m : *(ss->pv))
      std::cout << " " << m;
  std::cout << std::endl;*/

  return 0;
}

/// 'Short' player tries to minimize the distance from a hypothetical mate.
/// However, since the mate value is positive for the short player, it is
/// also implemented as a max player.
int short_player(Stack* ss, int alpha, int beta) {

  int score, max = -VALUE_INFINITE;
  int moveCount = 0;
  std::list<int> tempList;

  PVMoves pv;
  (ss+1)->pv = &pv;

  for (int i = 2; i < states.size(); i++)
  {
      if (states[i].isActive == false)
          continue;

      moveCount++;

      // Make the move ...
      // Put all active members of this list into a new list.
      // Needed for undoing the move.
      for (auto& m : states[i].l)
          if (states[m].isActive)
              tempList.push_back(m);

      // Deactivate all members
      for (auto& tl : tempList)
           states[tl].isActive = false;

      // Reduce the count for active lists
      // which have the member in their list.
      for (auto& m : states[i].l)
          for (int j = 2; j < states.size(); j++)
          {
              if (   states[j].isActive
                  && std::count(states[j].l.begin(), states[j].l.end(), m))
              {
                  states[j].count--;
                  states[j].diff++;
              }
          }

      nodes++;

      score = -long_player(ss+1, -beta, -alpha);

      // Unmake the move.
      // Restore the old count
      for (auto& m : states[i].l)
          for (int j = 2; j < states.size(); j++)
          {
              if (states[j].isActive)
              {
                  states[j].count += states[j].diff;
                  states[j].diff = 0;
              }
          }

      // Reactivate the lists
      for (auto& tl : tempList)
           states[tl].isActive = true;

      if (score > max)
      {
          max = score;

          if (score > alpha)
          {
              alpha = score;

              // Update PV
              ss->pv->clear();
              ss->pv->update(i, (ss+1)->pv);
          }
      }

      if (score >= beta)
      {
          cutoffs++;
          return score;
      }

      if (moveCount >= 4) break;

      tempList.clear();
  }

  if (!moveCount)
      max = VALUE_MATE - ss->ply;

  return max;
}

/// 'Long player' tries to maximize the distance from "getting mated".
/// It is also a max player
int long_player(Stack* ss, int alpha, int beta) {

  int score, max = -VALUE_INFINITE;
  int moveCount = 0;
  std::list<int> tempList;

  PVMoves pv;
  (ss+1)->pv = &pv;

  for (int i = states.size() - 1; i >= 2; i--)
  {
      if (states[i].isActive == false)
          continue;

      moveCount++;

      // Make the move ...
      // Put all active members of this list into a new list.
      // Needed for undoing the move.
      for (auto& m : states[i].l)
          if (states[m].isActive)
              tempList.push_back(m);

      // Deactivate all members
      for (auto& tl : tempList)
           states[tl].isActive = false;

      // Reduce the count for active lists
      // which have the member in their list.
      for (auto& m : states[i].l)
          for (int j = 2; j < states.size(); j++)
          {
              if (   states[j].isActive
                  && std::count(states[j].l.begin(), states[j].l.end(), m))
              {
                  states[j].count--;
                  states[j].diff++;
              }
          }

      nodes++;

      score = -short_player(ss+1, -beta, -alpha);

      // Unmake the move.
      // Restore the old count
      for (auto& m : states[i].l)
          for (int j = 2; j < states.size(); j++)
          {
              if (states[j].isActive)
              {
                  states[j].count += states[j].diff;
                  states[j].diff = 0;
              }
          }

      // Reactivate the lists
      for (auto& tl : tempList)
           states[tl].isActive = true;

      if (score > max)
      {
          max = score;

          if (score > alpha)
          {
              alpha = score;

              // Update PV
              ss->pv->clear();
              ss->pv->update(i, (ss+1)->pv);
          }
      }

      if (score >= beta)
      {
          cutoffs++;
          return score;
      }

      tempList.clear();
  }

  if (!moveCount)
      max = -VALUE_MATE + ss->ply;

  return max;
}

/// 'Short Short' player finds the shortest possible play.
int short_short_player(Stack* ss, int alpha, int beta) {

  int score, max = -VALUE_INFINITE;
  int moveCount = 0;

  PVMoves pv;
  (ss+1)->pv = &pv;
  ss->pv->clear();

  int i = get_next_best_short_move();

  if (i == -1)
      return VALUE_MATE - ss->ply;

  moveCount++;

  // Make the move ...
  // Deactivate all members
  for (auto& m : states[i].l)
      states[m].isActive = false;

  // Reduce the count for active lists
  // which have the member in their list.
  for (auto& m : states[i].l)
      for (int j = 2; j < states.size(); j++)
      {
          if (   states[j].isActive
              && std::find(states[j].l.begin(), states[j].l.end(), m) != states[j].l.end())
          {
              states[j].count--;
              states[j].l.remove(m);
          }
      }

  nodes++;

  score = short_short_player(ss+1, alpha, beta);

  if (score > max)
  {
      max = score;

      if (score > alpha)
      {
          alpha = score;

          // Update PV
          ss->pv->clear();
          ss->pv->update(i, (ss+1)->pv);
      }
  }

  return max;
}

/// 'Long Long' player finds the longest possible play.
int long_long_player(Stack* ss, int alpha, int beta) {

  int score, max = -VALUE_INFINITE;
  int moveCount = 0;

  PVMoves pv;
  (ss+1)->pv = &pv;

  int i = get_next_best_long_move();

  if (i == -1)
      return -VALUE_MATE + ss->ply;

  moveCount++;

  // Make the move ...
  // Deactivate all members
  for (auto& m : states[i].l)
      states[m].isActive = false;

  // Reduce the count for active lists
  // which have the member in their list.
  for (auto& m : states[i].l)
      for (int j = 2; j < states.size(); j++)
      {
          if (   states[j].isActive
              && std::find(states[j].l.begin(), states[j].l.end(), m) != states[j].l.end())
          {
              states[j].count--;
              states[j].l.remove(m);
          }
      }

  nodes++;

  score = long_long_player(ss+1, alpha, beta);

  if (score > max)
  {
      max = score;

      if (score > alpha)
      {
          alpha = score;

          // Update PV
          ss->pv->clear();
          ss->pv->update(i, (ss+1)->pv);
      }
  }

  return max;
}

