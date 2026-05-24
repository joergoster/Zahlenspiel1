#include <chrono>
#include <cstdint>
#include <iostream>
#include <list>

#include "search.h"

constexpr int n = 48;
constexpr int VALUE_MATE = 1000;
constexpr int VALUE_INFINITE = 1001;

std::uint64_t cutoffs, nodes;
Sequence allSequences[n+2]; // to avoid access with [n-2]

// Function prototypes
int short_player(Stack* ss, int alpha, int beta);
int  long_player(Stack* ss, int alpha, int beta);

// Calculate the 'connected' numbers
void generate_integer_divisors_and_multiples(int x) {

  allSequences[x].count = 0;

  // Adds all integer divisors and multiples in ascending order
  for (int k = 2; k <= n; k++)
  {
      if ((k <= x && x % k == 0) || (k > x && k % x == 0))
      {
          allSequences[x].l.push_back(k);
          allSequences[x].count++;
      }
  }

  // Activate
  allSequences[x].isActive = true;
}

// For easier output of a list
std::ostream& operator<<(std::ostream& ostr, const std::list<int>& list)
{
  for (const int i : list)
      ostr << ' ' << i;

  return ostr;
}

int main() {

  Stack stack[MAX_PLY] = {};
  Stack* ss = stack;

  PVMoves pv; // Will contain the complete PV
  ss->pv = &pv;

  for (int i = 0; i < MAX_PLY; i++)
      (ss+i)->ply = i;

  for (int i = 2; i <= n; i++)
      generate_integer_divisors_and_multiples(i);


//  int minBestValue = short_player(0, VALUE_INFINITE, 0);
//  std::cout << "Shortest play for n = " << n << " is " << VALUE_MATE - minBestValue << " plies" << std::endl;

//  int maxBestValue = long_player(-VALUE_INFINITE, 0, 0);
//  std::cout << "Longest play for n = " << n << " is " << VALUE_MATE + maxBestValue << " plies" << std::endl;

  nodes = cutoffs = 0;
  auto start1 = std::chrono::high_resolution_clock::now();
  int minmaxBestValue = short_player(ss, 0, VALUE_INFINITE);
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
  int maxminBestValue = long_player(ss, -VALUE_INFINITE, 0);
  auto end2 = std::chrono::high_resolution_clock::now();
  auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);

  std::cout << "Best long-short play for n = " << n << " is " << VALUE_MATE + maxminBestValue << " plies" << std::endl;
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

  for (int i = 2; i <= n; i++)
  {
      if (allSequences[i].isActive == false)
          continue;

      moveCount++;

      // Now put all active members of this list into a new list
      for (auto& m : allSequences[i].l)
          if (allSequences[m].isActive)
              tempList.push_back(m);

      for (auto& tl : tempList)
           allSequences[tl].isActive = false;

      nodes++;

      score = -long_player(ss+1, -beta, -alpha);

      for (auto& tl : tempList)
           allSequences[tl].isActive = true;

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

  for (int i = n; i >= 2; i--)
  {
      if (allSequences[i].isActive == false)
          continue;

      moveCount++;

      // Now put all active members of this list into a new list
      for (auto& m : allSequences[i].l)
          if (allSequences[m].isActive)
              tempList.push_back(m);

      for (auto& tl : tempList)
           allSequences[tl].isActive = false;

      nodes++;

      score = -short_player(ss+1, -beta, -alpha);

      for (auto& tl : tempList)
           allSequences[tl].isActive = true;

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
