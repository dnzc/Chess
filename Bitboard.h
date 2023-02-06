#pragma once

#include <cstdint>

// GROUP B SKILL: simple OOP
class Bitboard {
  public:
    Bitboard();
    Bitboard(uint64_t val);
    uint64_t getBits();
    int getLsb();
    int popLsb();
    int popcnt();

    Bitboard operator= (Bitboard op);
    bool operator== (Bitboard op);
    bool operator!= (Bitboard op);
    Bitboard operator~ ();
    Bitboard operator>> (int op);
    Bitboard operator>>= (int op);
    Bitboard operator<< (int op);
    Bitboard operator<<= (int op);
    Bitboard operator^ (Bitboard op);
    Bitboard operator^= (Bitboard op);
    Bitboard operator& (Bitboard op);
    Bitboard operator&= (Bitboard op);
    Bitboard operator| (Bitboard op);
    Bitboard operator|= (Bitboard op);
    Bitboard operator* (Bitboard op);
    Bitboard operator*= (Bitboard op);
    
  private:
    // BITBOARD INDEX SYSTEM:
    //
    //     8 | 56 57 58 59 60 61 62 63
    //     7 | 48 49 50 51 52 53 54 55
    //     6 | 40 41 42 43 44 45 46 47
    //     5 | 32 33 34 35 36 37 38 39
    //     4 | 24 25 26 27 28 29 30 31
    //     3 | 16 17 18 19 20 21 22 23
    //     2 |  8  9 10 11 12 13 14 15
    //     1 |  0  1  2  3  4  5  6  7
    // rank^  ------------------------
    // file>    a  b  c  d  e  f  g  h
    uint64_t m_board;

};
