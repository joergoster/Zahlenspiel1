#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <list>

#include "search.h"

constexpr int VALUE_MATE = 1000;
constexpr int VALUE_INFINITE = 1001;

std::uint64_t cutoffs, nodes;
States states;
int n = 40;

// Function prototypes
int short_player(Stack* ss, int alpha, int beta);
int  long_player(Stack* ss, int alpha, int beta);

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

//  int minBestValue = short_player(0, VALUE_INFINITE, 0);
//  std::cout << "Shortest play for n = " << n << " is " << VALUE_MATE - minBestValue << " plies" << std::endl;

//  int maxBestValue = long_player(-VALUE_INFINITE, 0, 0);
//  std::cout << "Longest play for n = " << n << " is " << VALUE_MATE + maxBestValue << " plies" << std::endl;

  nodes = cutoffs = 0;
  auto start1 = std::chrono::high_resolution_clock::now();

  int minmaxBestValue = short_player(ss, 0, VALUE_INFINITE); // short player begins

  auto end1 = std::chrono::high_resolution_clock::now();
  auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);

  std::cout << "Best short-long play for n = " << n << " is " << VALUE_MATE - minmaxBestValue << " plies" << std::endl;
  std::cout << "Time: " << duration1.count() << " ms" << "   Nodes: " << nodes << "   Cutoffs: " << cutoffs << std::endl;
  std::cout << "PV:";
  for (int m : *(ss->pv))
      std::cout << " " << m;
  std::cout << std::endl;

  nodes = cutoffs = 0;
  auto start2 = std::chrono::high_resolution_clock::now();

  int maxminBestValue = long_player(ss, -VALUE_INFINITE, 0); // long player begins

  auto end2 = std::chrono::high_resolution_clock::now();
  auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);

  std::cout << "\nBest long-short play for n = " << n << " is " << VALUE_MATE + maxminBestValue << " plies" << std::endl;
  std::cout << "Time: " << duration2.count() << " ms" << "   Nodes: " << nodes << "   Cutoffs: " << cutoffs << std::endl;
  std::cout << "PV:";
  for (int m : *(ss->pv))
      std::cout << " " << m;
  std::cout << std::endl;

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

      // Now put all active members of this list into a new list
      for (auto& m : states[i].l)
          if (states[m].isActive)
              tempList.push_back(m);

      for (auto& tl : tempList)
           states[tl].isActive = false;

      nodes++;

      score = -long_player(ss+1, -beta, -alpha);

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

      // Now put all active members of this list into a new list
      for (auto& m : states[i].l)
          if (states[m].isActive)
              tempList.push_back(m);

      for (auto& tl : tempList)
           states[tl].isActive = false;

      nodes++;

      score = -short_player(ss+1, -beta, -alpha);

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
