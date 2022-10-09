#pragma once

#include <cstdint>

class Bitboard {

  public:
    Bitboard();
    uint64_t getBits();
    bool getBit(int index);
    void setBits(uint64_t bits);
    void swapBits(int a, int b);

  private:
    // 64 bits representing each board square
    uint64_t m_bits;

};
