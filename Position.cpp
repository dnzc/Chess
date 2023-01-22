#include "Position.h"
#include "Move.h"
#include "Bitboard.h"
#include "Util.h"
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

  m_plysSince50 = 0;

  // initialize piece bitboards
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

}

Position::Position(std::string FEN) {

  for(int i=0; i<64; ++i) m_board[i] = empty;
  for(int i=0; i<12; ++i) m_pieces[i] = 0;

  int stringIndex = 0;
  int fenBoardIndex = 0;

  char c = FEN[stringIndex];

  // piece placements
  while(c != ' ') {
    if(c=='/') {
      stringIndex++;
      c = FEN[stringIndex];
      continue;
    }
    if(c=='1'||c=='2'||c=='3'||c=='4'||c=='5'||c=='6'||c=='7'||c=='8') {
      fenBoardIndex += (int)(c - '0');
      stringIndex++;
      c = FEN[stringIndex];
      continue;
    }
    // c is one of pnbrqkPNBRQK
    int actualIndex = (fenBoardIndex&7) + (7-(fenBoardIndex>>3))*8; // fen index 0 is actual index 56, fen 1 is actual 57, etc
    std::string s("PNBRQKpnbrqk");
    int pieceType = s.find(c);
    m_board[actualIndex] = (PieceType) pieceType;
    m_pieces[pieceType] |= 1ull<<actualIndex;
    stringIndex++;
    c = FEN[stringIndex];
    fenBoardIndex++;
  }

  // side to move
  stringIndex++;
  c = FEN[stringIndex];
  m_whiteToMove = c == 'w';

  // castling rights
  stringIndex+=2;
  c = FEN[stringIndex];
  m_whiteCastleKingside = false;
  m_whiteCastleQueenside = false;
  m_blackCastleKingside = false;
  m_blackCastleQueenside = false;
  while(c != ' ') {
    switch(c) {
      case 'K':
        m_whiteCastleKingside = true;
        break;
      case 'Q':
        m_whiteCastleQueenside = true;
        break;
      case 'k':
        m_blackCastleKingside = true;
        break;
      case 'q':
        m_blackCastleQueenside = true;
        break;
      default: // c might be -
        break;
    }
    stringIndex++;
    c = FEN[stringIndex];
  }

  // en passant
  stringIndex++;
  c = FEN[stringIndex];
  if(c!='-') {
    int index = (c-'a') + (FEN[stringIndex+1]-'1')*8;
    m_enPassant = 1ull<<index | (m_whiteToMove ? (1ull<<index)<<8 : (1ull<<index)>>8);
    stringIndex++;
  }

  // if halfmove clock not provided, return
  stringIndex += 2;
  if(stringIndex >= FEN.length()) return;

  // halfmove clock
  std::string clock = "";
  c = FEN[stringIndex];
  while(c != ' ') {
    clock += c;
    stringIndex++;
    c = FEN[stringIndex];
  }
  m_plysSince50 = std::stoi(clock);

}

Bitboard Position::getWhiteOccupancy() {
  return m_pieces[wp] | m_pieces[wn] | m_pieces[wb] | m_pieces[wr] | m_pieces[wq] | m_pieces[wk];
}

Bitboard Position::getBlackOccupancy() {
  return m_pieces[bp] | m_pieces[bn] | m_pieces[bb] | m_pieces[br] | m_pieces[bq] | m_pieces[bk];
}

Bitboard Position::getPieces(PieceType pt) {
  return m_pieces[pt];
}

Bitboard Position::getEnPassant() {
  return m_enPassant;
}

void Position::removePieces(PieceType pt, Bitboard bb) {
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

int Position::getPlysSince50() {
  return m_plysSince50;
}

// GROUP A SKILL - complex user-defined algorithms
void Position::makeMove(Move move) {

  // remove target piece if it exists
  PieceType pieceToDie = m_board[move.end];
  if(pieceToDie != empty) {
    Bitboard capturedPiece = 1ull<<move.end;
    m_pieces[pieceToDie] &= ~capturedPiece;
  }

  // move the piece
  m_pieces[move.piece] &= ~(1ull<<move.start);
  // if a pawn promotion, then update the right bitboard
  if(move.promotion) m_pieces[move.promotion] |= 1ull<<move.end;
  else m_pieces[move.piece] |= 1ull<<move.end;

  // update square info
  m_board[move.end] = (PieceType) (move.promotion ? move.promotion : move.piece);
  m_board[move.start] = empty;

  m_enPassant = 0;

  // if current move is double pawn push, then update en passant availability
  if(move.piece==wp && move.end-move.start==16) {
    m_enPassant = (1ull<<move.end) | (1ull<<(move.end-8));
  } else if(move.piece==bp && move.end-move.start==-16) {
    m_enPassant = (1ull<<move.end) | (1ull<<(move.end+8));
  }
  // if current move is en passant, then remove the piece to be captured
  else if(move.enPassant) {
    Bitboard capturedPawn = 1ull<<move.end;
    if(m_whiteToMove) capturedPawn >>= 8;
    else capturedPawn <<= 8;
    m_pieces[m_whiteToMove ? bp : wp] &= ~capturedPawn;
    m_board[capturedPawn.getLsb()] = empty;
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
      m_pieces[wr] &= ~(1ull<<7);
      m_pieces[wr] |= (1ull<<5);
      m_board[7] = empty;
      m_board[5] = wr;
    }
    // white queenside castle
    else if(move.start == 4 && move.end == 2) {
      m_pieces[wr] &= ~(1ull);
      m_pieces[wr] |= (1ull<<3);
      m_board[0] = empty;
      m_board[3] = wr;
    }
    // black kingside castle
    else if(move.start == 60 && move.end == 62) {
      m_pieces[br] &= ~(1ull<<63);
      m_pieces[br] |= (1ull<<61);
      m_board[63] = empty;
      m_board[61] = br;
    }
    // black queenside castle
    else if(move.start == 60 && move.end == 58) {
      m_pieces[br] &= ~(1ull<<56);
      m_pieces[br] |= (1ull<<59);
      m_board[56] = empty;
      m_board[59] = br;
    }
  }

  // update 50 move rule
  if(pieceToDie!=empty || move.piece==wp || move.piece==bp) m_plysSince50 = 0;
  else m_plysSince50++;

  // flip player to move
  m_whiteToMove = !m_whiteToMove;

}

// makeMove but reversed
// only for use in minimax, since does not restore 50 move counter
void Position::undoMove(Move move, PieceType capturedPiece, Bitboard prevPassant, int prev50) {

  // flip player to move
  m_whiteToMove = !m_whiteToMove;

  // 50 move rule
  m_plysSince50 = prev50;

  // if current move is castling, then move the rook back
  if(move.castle) {
    // white kingside castle
    if(move.start == 4 && move.end == 6) {
      m_pieces[wr] |= (1ull<<7);
      m_pieces[wr] &= ~(1ull<<5);
      m_board[7] = wr;
      m_board[5] = empty;
      m_whiteCastleKingside = true;
    }
    // white queenside castle
    else if(move.start == 4 && move.end == 2) {
      m_pieces[wr] |= 1ull;
      m_pieces[wr] &= ~(1ull<<3);
      m_board[0] = wr;
      m_board[3] = empty;
      m_whiteCastleQueenside = true;
    }
    // black kingside castle
    else if(move.start == 60 && move.end == 62) {
      m_pieces[br] |= (1ull<<63);
      m_pieces[br] &= ~(1ull<<61);
      m_board[63] = br;
      m_board[61] = empty;
      m_blackCastleKingside = true;
    }
    // black queenside castle
    else if(move.start == 60 && move.end == 58) {
      m_pieces[br] |= (1ull<<56);
      m_pieces[br] &= ~(1ull<<59);
      m_board[56] = br;
      m_board[59] = empty;
      m_blackCastleQueenside = true;
    }
  }

  // if current move is double pawn push, then update en passant availability
  if(move.piece==wp && move.end-move.start==16) {
    m_enPassant = prevPassant;
  } else if(move.piece==bp && move.end-move.start==-16) {
    m_enPassant = prevPassant;
  }
  // if current move is en passant, then add back the captured pawn
  else if(move.enPassant) {
    Bitboard capturedPawn = 1ull<<move.end;
    if(m_whiteToMove) capturedPawn >>= 8;
    else capturedPawn <<= 8;
    m_pieces[m_whiteToMove ? bp : wp] |= capturedPawn;
    m_board[capturedPawn.getLsb()] = m_whiteToMove ? bp : wp;
  }

  // update square info
  m_board[move.end] = capturedPiece;
  m_board[move.start] = (PieceType)move.piece;

  // move the piece back
  m_pieces[move.piece] |= (1ull<<move.start);
  if(move.promotion) m_pieces[move.promotion] &= ~(1ull<<move.end);
  else m_pieces[move.piece] &= ~(1ull<<move.end);

  // add back target piece if it exists
  if(capturedPiece != empty) {
    Bitboard piece = 1ull<<move.end;
    m_pieces[capturedPiece] |= piece;
  }

}
