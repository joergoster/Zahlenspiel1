#include <cstdint>
#include <cstring>
#include <list>

#pragma once

constexpr int MAX_PLY = 128;

struct Sequence {
  int count; // instead of l.size()
  bool isActive;
  std::list<int> l;
};

struct PVMoves {
    int         moves[MAX_PLY];
    std::size_t length = 0;

    int*        begin() { return moves; }
    const int*  begin() const { return moves; }
    int*        end() { return moves + length; }
    const int*  end() const { return moves + length; }

    int&        operator[](std::size_t index) { return moves[index]; }
    const int&  operator[](std::size_t index) const { return moves[index]; }

    bool        empty() const { return length == 0; }
    std::size_t size() const { return length; }

    void clear() { length = 0; }

    void push_back(int move) {
        moves[length++] = move;
    }

    void resize(std::size_t newSize) {
        length = newSize;
    }

    void update(int move, const PVMoves* childPv) {

        moves[0] = move;
        length = childPv ? childPv->length : 1;

        if (childPv)
        {
            std::memcpy(moves + 1, childPv->moves, length * sizeof(int));
            length++;
        }
    }
};

// A small stack for collecting the PV
struct Stack {
    PVMoves* pv;
    int ply;
};

