#include "Bitboard.h"
#include <cstdint>

Bitboard::Bitboard() {
  m_bits = 0;
}

uint64_t Bitboard::getBits() {
  return m_bits;
}

bool Bitboard::getBit(int index) {
  return (m_bits >> index) & 1;
}

void Bitboard::setBits(uint64_t bits) {
  m_bits = bits;
}
