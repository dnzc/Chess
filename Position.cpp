#include "Position.h"
#include "Move.h"
#include "Bitboard.h"
#include <iostream>
#include <string>
#include <random>
#include <vector>

Position::Position() {

  // init piece positions
  m_board[0] = wr;
  m_board[1] = wn;
  m_board[2] = wb;
  m_board[3] = wq;
  m_board[4] = wk;
  m_board[5] = wb;
  m_board[6] = wn;
  m_board[7] = wr;
  for(int i=8; i<16; ++i) m_board[i] = wp;
  for(int i=16; i<48; ++i) m_board[i] = empty;
  for(int i=48; i<56; ++i) m_board[i] = bp;
  m_board[56] = br;
  m_board[57] = bn;
  m_board[58] = bb;
  m_board[59] = bq;
  m_board[60] = bk;
  m_board[61] = bb;
  m_board[62] = bn;
  m_board[63] = br;

  m_enPassant = 0;

  m_whiteToMove = true;
  m_whiteCastleKingside = true;
  m_whiteCastleQueenside = true;
  m_blackCastleKingside = true;
  m_blackCastleQueenside = true;

  // initialize piece bitboards and symbols
  m_pieces[wp] = 255ull << 8;
  m_pieces[wn] = 66ull;
  m_pieces[wb] = 36ull;
  m_pieces[wr] = 129ull;
  m_pieces[wq] = 8ull;
  m_pieces[wk] = 16ull;
  m_pieces[bp] = 255ull << 48;
  m_pieces[bn] = 66ull << 56;
  m_pieces[bb] = 36ull << 56;
  m_pieces[br] = 129ull << 56;
  m_pieces[bq] = 8ull << 56;
  m_pieces[bk] = 16ull << 56;

  m_whiteOccupancy = 0x000000000000ffff;
  m_blackOccupancy = 0xffff000000000000;

}

Bitboard Position::getWhiteOccupancy() {
  return m_whiteOccupancy;
}

Bitboard Position::getBlackOccupancy() {
  return m_blackOccupancy;
}

Bitboard Position::getPieces(PieceType pt) {
  return m_pieces[pt];
}

Bitboard Position::getEnPassant() {
  return m_enPassant;
}

void Position::removePieces(PieceType pt, Bitboard bb) {
  if(pt<6) m_whiteOccupancy ^= bb;
  else m_blackOccupancy ^= bb;
  m_pieces[pt] ^= bb;
}

PieceType Position::whichPiece(int square) {
  return m_board[square];
}

bool Position::isWhiteToMove() {
  return m_whiteToMove;
}

bool Position::canWhiteCastleKingside() {
  return m_whiteCastleKingside;
}

bool Position::canWhiteCastleQueenside() {
  return m_whiteCastleQueenside;
}
bool Position::canBlackCastleKingside() {
  return m_blackCastleKingside;
}

bool Position::canBlackCastleQueenside() {
  return m_blackCastleQueenside;
}

void Position::makeMove(Move move) {
  // remove target piece if it exists
  PieceType pieceToDie = m_board[move.end];
  if(pieceToDie != empty) {
    Bitboard capturedPiece = 1ull<<move.end;
    m_pieces[pieceToDie] ^= capturedPiece;
    if(!m_whiteToMove) m_whiteOccupancy ^= capturedPiece;// if piece to die is white, i.e. black to move
    else m_blackOccupancy ^= capturedPiece;
  }

  // move the piece
  m_pieces[move.piece] ^= 1ull<<move.start;
  m_pieces[move.piece] |= 1ull<<move.end;
  if(m_whiteToMove) {
    m_whiteOccupancy ^= 1ull<<move.start;
    m_whiteOccupancy |= 1ull<<move.end;
  } else {
    m_blackOccupancy ^= 1ull<<move.start;
    m_blackOccupancy |= 1ull<<move.end;
  }

  // update square info
  m_board[move.end] = m_board[move.start];
  m_board[move.start] = empty;

  m_enPassant = 0;

  // if current move is double pawn push, then update en passant availability
  if(move.piece==wp && move.end-move.start==16) {
    m_enPassant = (1ull<<move.end) | (1ull<<(move.end-8));
  }
  else if(move.piece==bp && move.end-move.start==-16) {
    m_enPassant = (1ull<<move.end) | (1ull<<(move.end+8));
  }
  // if current move is en passant, then remove the piece to be captured
  else if(move.enPassant) {
    Bitboard capturedPawn = 1ull<<move.end;
    if(m_whiteToMove) capturedPawn >>= 8;
    else capturedPawn <<= 8;
    m_pieces[m_whiteToMove ? bp : wp] ^= capturedPawn;
    if(!m_whiteToMove) m_whiteOccupancy ^= capturedPawn;
    else m_blackOccupancy ^= capturedPawn;
  }
  // if current move is a rook on (a1,h1,a8,h8), then remove corresponding castling rights
  else if(move.piece==wr && move.start == 7) m_whiteCastleKingside = false;
  else if(move.piece==wr && move.start == 0) m_whiteCastleQueenside = false;
  else if(move.piece==br && move.start == 63) m_blackCastleKingside = false;
  else if(move.piece==br && move.start == 56) m_blackCastleQueenside = false;
  // if current move is king, then remove corresponding castling rights
  else if(move.piece==wk) { m_whiteCastleKingside = false; m_whiteCastleQueenside = false; }
  else if(move.piece==bk) { m_blackCastleKingside = false; m_blackCastleQueenside = false; }

  // if current move lands on an enemy starting rook square, remove corresponding castling rights
  if(move.end == 7 && move.piece>=6) m_whiteCastleKingside = false;
  else if(move.end == 0 && move.piece>=6) m_whiteCastleQueenside = false;
  else if(move.end == 63 && move.piece<6) m_blackCastleKingside = false;
  else if(move.end == 56 && move.piece<6) m_blackCastleQueenside = false;

  // if current move is castling, then move the rook
  if(move.castle) {
    // white kingside castle
    if(move.start == 4 && move.end == 6) {
      m_pieces[wr] ^= 1ull<<7;
      m_whiteOccupancy ^= 1ull<<7;
      m_pieces[wr] ^= 1ull<<5;
      m_whiteOccupancy ^= 1ull<<5;
    }
    // white queenside castle
    else if(move.start == 4 && move.end == 2) {
      m_pieces[wr] ^= 1ull;
      m_whiteOccupancy ^= 1ull;
      m_pieces[wr] ^= 1ull<<3;
      m_whiteOccupancy ^= 1ull<<3;
    }
    // black kingside castle
    else if(move.start == 60 && move.end == 62) {
      m_pieces[br] ^= 1ull<<63;
      m_blackOccupancy ^= 1ull<<63;
      m_pieces[br] ^= 1ull<<61;
      m_blackOccupancy ^= 1ull<<61;
    }
    // black queenside castle
    else if(move.start == 60 && move.end == 58) {
      m_pieces[br] ^= 1ull<<56;
      m_blackOccupancy ^= 1ull<<56;
      m_pieces[br] ^= 1ull<<59;
      m_blackOccupancy ^= 1ull<<59;
    }
  }

  // flip player to move
  m_whiteToMove = !m_whiteToMove;
}
