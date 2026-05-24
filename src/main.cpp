#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <list>

#include "search.h"

constexpr int n = 48;
constexpr int VALUE_MATE = 1000;
constexpr int VALUE_INFINITE = 1001;

std::uint64_t cutoffs, nodes;
std::array<Sequence, n+1> allSequences; // index 0 unused, 1..n used

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

// Get next number with highest count in range [start, end]
// Uses round-robin selection among tied numbers
auto getNextHighestCount = [](int start, int end) -> int {
    static int lastIndex = start - 1;
    
    int bestNum = -1;
    int bestCount = -1;
    std::list<int> candidates;
    
    // First pass: find the best count
    for (int i = start; i <= end; i++)
    {
        if (allSequences[i].isActive && allSequences[i].count > bestCount)
        {
            bestCount = allSequences[i].count;
            candidates.clear();
            candidates.push_back(i);
        }
        else if (allSequences[i].isActive && allSequences[i].count == bestCount)
        {
            candidates.push_back(i);
        }
    }
    
    if (candidates.empty())
        return -1;
    
    // Second pass: find the next candidate after lastIndex (round-robin)
    for (int candidate : candidates)
    {
        if (candidate > lastIndex)
        {
            lastIndex = candidate;
            return candidate;
        }
    }
    
    // Wrap around: return the first candidate
    bestNum = candidates.front();
    lastIndex = bestNum;
    return bestNum;
};

int main() {

  std::array<Stack, MAX_PLY> stack = {};
  Stack* ss = stack.data();

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

  for (int moveCount_iter = 0; moveCount_iter < 4; moveCount_iter++)
  {
      int i = getNextHighestCount(2, n);
      if (i == -1 || allSequences[i].isActive == false)
          break;

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
