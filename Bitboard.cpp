#include "Bitboard.h"
#include <cstdint>
#include <bit>
#include <iostream>

Bitboard::Bitboard() {
  m_board = 0;
}

Bitboard::Bitboard(uint64_t val) {
  m_board = val;
}

Bitboard Bitboard::operator= (Bitboard op) {
  m_board = op.getBits();
  return m_board;
}

bool Bitboard::operator== (Bitboard op) {
  return m_board == op.getBits();
}

bool Bitboard::operator!= (Bitboard op) {
  return m_board != op.getBits();
}

Bitboard Bitboard::operator>>= (int op) {
  m_board >>= op;
  return m_board;
}

Bitboard Bitboard::operator>> (int op) {
  return m_board >> op;
}

Bitboard Bitboard::operator<<= (int op) {
  m_board <<= op;
  return m_board;
}

Bitboard Bitboard::operator<< (int op) {
  return m_board << op;
}

Bitboard Bitboard::operator^= (Bitboard op) {
  m_board ^= op.getBits();
  return m_board;
}

Bitboard Bitboard::operator^ (Bitboard op) {
  return m_board ^ op.getBits();
}

Bitboard Bitboard::operator&= (Bitboard op) {
  m_board &= op.getBits();
  return m_board;
}

Bitboard Bitboard::operator& (Bitboard op) {
  return m_board & op.getBits();
}

Bitboard Bitboard::operator|= (Bitboard op) {
  m_board |= op.getBits();
  return m_board;
}

Bitboard Bitboard::operator| (Bitboard op) {
  return m_board | op.getBits();
}

Bitboard Bitboard::operator*= (Bitboard op) {
  m_board |= op.getBits();
  return m_board;
}

Bitboard Bitboard::operator* (Bitboard op) {
  return m_board * op.getBits();
}

Bitboard Bitboard::operator~ () {
  return ~m_board;
}

int Bitboard::getLsb() {
  if(m_board == 0) return -1;
  uint64_t bb = m_board & -m_board; // extract the lsb
  // binary search to find index of the bit
  int index = 0;
  if(bb>>32) { bb>>=32; index+=32; }
  if(bb>>16) { bb>>=16; index+=16; }
  if(bb>>8) { bb>>=8; index+=8; }
  if(bb>>4) { bb>>=4; index+=4; }
  if(bb>>2) { bb>>=2; index+=2; }
  if(bb>>1) { bb>>=1; index++; }
  return index;
}

int Bitboard::popLsb() {
  if(m_board == 0) return -1;
  uint64_t bb = m_board & -m_board; // extract the lsb
  m_board ^= bb; // wipe the bit
  // binary search to find index of the bit
  int index = 0;
  if(bb>>32) { bb>>=32; index+=32; }
  if(bb>>16) { bb>>=16; index+=16; }
  if(bb>>8) { bb>>=8; index+=8; }
  if(bb>>4) { bb>>=4; index+=4; }
  if(bb>>2) { bb>>=2; index+=2; }
  if(bb>>1) { bb>>=1; index++; }
  return index;
}

int Bitboard::popcnt() {
  return std::__popcount(m_board);
}

uint64_t Bitboard::getBits() {
  return m_board;
}
