#include "MoveGenerator.h"
#include "Position.h"
#include "Move.h"
#include "Bitboard.h"
#include <iostream>
#include <string>
#include <random>
#include <vector>

MoveGenerator::MoveGenerator() {

  initKnightMoveTable();
  initKingMoveTable();

  initPushMasks();

  initRookBishopBlockerMasks(true); // rook move masks
  initRookBishopBlockerMasks(false); // bishop move masks

  initRookBishopMoveTable(true);  // rook move table
  initRookBishopMoveTable(false); // bishop move table

  //findRookBishopMagics(true); // rook magics
  //findRookBishopMagics(false); // bishop magics

}

// GROUP B SKILL: simple user-defined algorithms
// calculate+store knight move lookup table
void MoveGenerator::initKnightMoveTable() {
  // knight moves
  int dx[] = {2, 2, -2, -2, 1, 1, -1, -1};
  int dy[] = {1, -1, 1, -1, 2, -2, 2, -2};
  // for each square
  for(int i=0; i<64; ++i) {
    m_knightMoves[i] = 0;
    for(int j=0; j<8; ++j) {
      // add (dx,dy) to position then check if valid
      int x = (int)(i&7) + dx[j];
      int y = (int)(i>>3) + dy[j];
      if(0<=x && 0 <= y && x < 8 && y < 8) m_knightMoves[i] |= 1ull<<(8*y+x);
    }
  }
}

// GROUP B SKILL: simple user-defined algorithms
// calculate+store king move lookup table
void MoveGenerator::initKingMoveTable() {
  // king moves
  int dx[] = {-1, -1, -1, 0, 1, 1, 1, 0};
  int dy[] = {-1, 0, 1, 1, 1, 0, -1, -1};
  // for each square
  for(int i=0; i<64; ++i) {
    m_kingMoves[i] = 0;
    for(int j=0; j<8; ++j) {
      // add (dx,dy) to position then check if valid
      int x = (int)(i&7) + dx[j];
      int y = (int)(i>>3) + dy[j];
      if(0<=x && 0 <= y && x < 8 && y < 8) m_kingMoves[i] |= 1ull<<(8*y+x);
    }
  }
}

// GROUP B SKILL: simple user-defined algorithms
// calculate+store blocker masks, for rooks or bishops
void MoveGenerator::initRookBishopBlockerMasks(bool isRook) {
  // for each square
  for(int i=0; i<64; ++i) {
    if(isRook) m_rookMasks[i] = 0;
    else m_bishopMasks[i] = 0;

    int rookXChanges[] = {1, -1, 0, 0};
    int rookYChanges[] = {0, 0, 1, -1};
    int bishopXChanges[] = {1, 1, -1, -1};
    int bishopYChanges[] = {1, -1, 1, -1};
    for(int k=0; k<4; ++k) {
      // move in direction specified by k, until arriving at the board edge
      int x = i&7;
      int y = i>>3;
      int dx = (isRook ? rookXChanges[k] : bishopXChanges[k]);
      int dy = (isRook ? rookYChanges[k] : bishopYChanges[k]);
      while(true) {
        x+=dx;
        y+=dy;
        if(isRook && ( dy==0 && (x<1||x>6) || dx==0 && (y<1||y>6) ) ) break; // if rook, then still possible to be on edge of board with valid moves
        if(!isRook  && (x < 1 || x > 6 || y < 1 || y > 6)) break; // if bishop, then no valid moves once reached edge
        // set the bit at position (x,y)
        int index = y*8 + x;
        if(isRook) m_rookMasks[i] |= (1ull<<index);
        else m_bishopMasks[i] |= (1ull<<index);
      }
    }
  }
}

// GROUP B SKILL: simple user-defined algorithms
// calculate+store sliding piece moves between two squares
void MoveGenerator::initPushMasks() {
  // for each square
  for(int i=0; i<64; ++i) {
    // each of the 8 directions (first 4 rook, last 4 bishop)
    int dxs[] = {0, 0, 1, -1, 1, 1, -1, -1};
    int dys[] = {1, -1, 0, 0, 1, -1, -1, 1};
    for(int k=0; k<8; ++k) {
      // move in direction specified by k, until arriving at the board edge
      int x = i&7;
      int y = i>>3;
      int dx = dxs[k];
      int dy = dys[k];
      Bitboard bb = 1ull<<i;
      while(true) {
        x+=dx;
        y+=dy;
        if( x<0 || x>7 || y<0 || y>7) break;
        int index = y*8 + x;
        bb |= 1ull<<index;
        if(k<4) // if rook
          m_rookPushMasks[i][index] = bb & ~(1ull<<i) & ~(1ull<<index); // remove start and end squares
        else
          m_bishopPushMasks[i][index] = bb & ~(1ull<<i) & ~(1ull<<index); // remove start and end squares
      }
    }
  }
}

// GROUP B SKILL: simple user-defined algorithms
// given a mask of possible blocker positions, returns the index-th configuration of blockers (sorting lexicographically)
Bitboard MoveGenerator::getBlockerBoard(Bitboard mask, int index) {
  // turn index into binary then distribute its bits among where the mask indicates the bits should go
  Bitboard blockerBoard = mask;
  int bitIndex = 0;
  for(int sq = 0; sq<64; ++sq) {
    if( ((mask>>sq)&1) == 1 ) {
      // set the sq-th bit of the board to (bitIndex-th bit of j)
      Bitboard val = (index>>bitIndex)&1; // val = bitIndex-th bit of j
      blockerBoard ^= (1ull << sq); // clear the sq-th bit
      blockerBoard |= (val<<sq); // or the sq-th bit with val, so that sq-th bit = val
      bitIndex++;
    }
  }
  return blockerBoard;
}

// GROUP B SKILL: simple user-defined algorithms
// given an arrangement of blockers, calculate the set of (pseudo-)legal moves
Bitboard MoveGenerator::getRookBishopMoveBoard(bool isRook, Bitboard blockerBoard, int square) {
  Bitboard moveBoard = blockerBoard;
  int rookXChanges[] = {1, -1, 0, 0};
  int rookYChanges[] = {0, 0, 1, -1};
  int bishopXChanges[] = {1, 1, -1, -1};
  int bishopYChanges[] = {1, -1, 1, -1};
  for(int k=0; k<4; ++k) {
    // move in direction specified by k, until the first blocker is met
    int x = square&7;
    int y = square>>3;
    int dx = (isRook ? rookXChanges[k] : bishopXChanges[k]);
    int dy = (isRook ? rookYChanges[k] : bishopYChanges[k]);
    bool foundBlocker = false;
    while(true) {
      x+=dx;
      y+=dy;
      if(x < 0 || x > 7 || y < 0 || y > 7) break;
      int index = y*8 + x;
      if(foundBlocker) moveBoard &= ~(1ull<<index); // wipe the bit
      else moveBoard |= (1ull<<index); // set the bit
      if( ((blockerBoard>>index)&1) == 1) foundBlocker = true;
    }
  }
  return moveBoard;
}

// GROUP A SKILL: complex user-defined algorithms
void MoveGenerator::findRookBishopMagics(bool isRook) {
  std::cout << "finding magics...\n";
  // number of bits needed to store all possible blocker configurations, which equals the max number of potential blockers
  int bits = (isRook ? 12 : 9); // a rook has at most 12 potential blockers, bishop has at most 9
  int size = 1<<bits; // at most 2^bits possible blocker configurations: i.e. 4096 for rook, 512 for bishop

  // GROUP A SKILL: hashing
  // generate all possible blocker configurations
  for(int i=0; i<64; ++i) {
    // try magic numbers until one works i.e. hashes with no collisions
    while(true) {
      // generate a random magic bitboard with a low density of 1s
      // note: assumes std::rand is 32-bit
      Bitboard random1 = ((long long)rand() << 32) | rand();
      Bitboard random2 = ((long long)rand() << 32) | rand();
      Bitboard random3 = ((long long)rand() << 32) | rand();
      Bitboard trialNum = random1 & random2 & random3;

      bool fail = false;
      Bitboard moveTable[size];
      bool used[size];
      for(int j=0; j<size; ++j) {
        used[j] = false;
      }
      for(int j=0; j<size; ++j) {
        Bitboard blockerBoard = getBlockerBoard(isRook ? m_rookMasks[i] : m_bishopMasks[i], j);
        int index = (trialNum.getBits() * blockerBoard.getBits()) >> (64 - bits); // use first 12 (if rook, 9 if bishop) bits as magic index
        Bitboard moveBoard = getRookBishopMoveBoard(isRook, blockerBoard, i);
        // if two blocker boards generate the same magic index but different move boards, then fail
        if(used[index] && moveBoard != moveTable[index]) {
          fail = true;
          break;
        } else {
          used[index] = true;
          moveTable[index] = moveBoard;
        }
      }
      if(!fail) {
        std::cout << trialNum.getBits() << "\n";
        break;
      }
    }

  }  
}

// GROUP B SKILL: simple user-defined algorithms
void MoveGenerator::initRookBishopMoveTable(bool isRook) {
  // number of bits needed to store all possible blocker configurations, which equals the max number of potential blockers
  int bits = (isRook ? 12 : 9); // a rook has at most 12 potential blockers, bishop has at most 9
  int size = 1<<bits; // at most 2^bits possible blocker configurations: i.e. 4096 for rook, 512 for bishop
  
  // for each square
  for(int i=0; i<64; ++i) {
    Bitboard magicNum = isRook ? m_rookMagics[i] : m_bishopMagics[i];
    // for every blocker configuration
    for(int j=0; j<size; ++j) {
      Bitboard blockerBoard = getBlockerBoard(isRook ? m_rookMasks[i] : m_bishopMasks[i], j);

      int index = (magicNum.getBits() * blockerBoard.getBits()) >> (64 - bits); // magic index
      Bitboard moveBoard = getRookBishopMoveBoard(isRook, blockerBoard, i);
      // write to the table
      if(isRook) m_rookMoves[i][index] = moveBoard;
      else m_bishopMoves[i][index] = moveBoard;
    }
  }
}

Bitboard MoveGenerator::rookMoves(int square, Bitboard occupancy) {
  Bitboard blockerBoard = occupancy & m_rookMasks[square];
  Bitboard index = (m_rookMagics[square] * blockerBoard) >> (64 - 12);
  return m_rookMoves[square][index.getBits()];
}

Bitboard MoveGenerator::bishopMoves(int square, Bitboard occupancy) {
  Bitboard blockerBoard = occupancy & m_bishopMasks[square];
  Bitboard index = (m_bishopMagics[square] * blockerBoard) >> (64 - 9);
  return m_bishopMoves[square][index.getBits()];
}

Bitboard MoveGenerator::queenMoves(int square, Bitboard occupancy) {
  // a queen can be treated as a rook and bishop on the same square
  return rookMoves(square, occupancy) | bishopMoves(square, occupancy);
}

// GROUP B SKILL: simple user-defined algorithms
Bitboard MoveGenerator::pawnPushes(Bitboard pawns, bool isWhite, Bitboard occupancy) {
  // bitwise shift by +-8 to get places the pawns could advance to
  Bitboard moves = isWhite ? pawns<<8 : pawns>>8;
  moves &= ~occupancy; // can't push to occupied square

  // factor in double pawn moves: if can move to third or sixth rank, then it must be the pawn's first move
  moves |= isWhite ? (moves&thirdRank)<<8 : (moves&sixthRank)>>8;
  moves &= ~occupancy; // can't push to occupied square

  return moves;
}

Bitboard MoveGenerator::pawnAttacks(Bitboard pawns, bool isWhite) {
  // bitwise shift left by 7 (or -9 if black) to get the captures to the left
  Bitboard leftAttacks = isWhite ? pawns<<7 : pawns>>9;
  leftAttacks &= notHFile; // a left capture can't be on the H file, fixes wraparound issues

  Bitboard rightAttacks = isWhite ? pawns<<9 : pawns>>7;
  rightAttacks &= notAFile;

  return leftAttacks | rightAttacks;
}

Bitboard MoveGenerator::enPassantCaptures(Bitboard pawns) {
  Bitboard left = (pawns>>1) & notHFile; // a left capture can't be on the H file, fixes wraparound issues
  Bitboard right = (pawns<<1) & notAFile;
  return left | right;
}

// GROUP B SKILL: simple user-defined algorithms
Bitboard MoveGenerator::getDangerSquares(Position& position) {
  Bitboard dangerSquares = 0;

  bool isWhite = position.isWhiteToMove();
  Bitboard occ = position.getWhiteOccupancy() | position.getBlackOccupancy();
  occ &= ~position.getPieces(isWhite ? wk : bk); // ignore our king

  // enemy pawn attacks
  dangerSquares |= pawnAttacks(position.getPieces(isWhite ? bp : wp), !isWhite);

  // for each other piece type
  for(int t=wn; t<=wk; ++t) {
    PieceType type = (PieceType) (t + isWhite*6); // enemy piece type
    Bitboard i = position.getPieces(type);
    while(i.getBits()) {
      int index = i.popLsb();
      switch(t) {
        case wn:
          dangerSquares |= m_knightMoves[index];
          break;
        case wb:
          dangerSquares |= bishopMoves(index, occ);
          break;
        case wr:
          dangerSquares |= rookMoves(index, occ);
          break;
        case wq:
          dangerSquares |= queenMoves(index, occ);
          break;
        case wk:
          dangerSquares |= m_kingMoves[index];
          break;
      }
    }
  }

  return dangerSquares;
}

// GROUP B SKILL: simple user-defined algorithms
Bitboard MoveGenerator::getCheckingPieces(Position& position) {
  Bitboard checkers = 0;
  bool isWhite = position.isWhiteToMove();
  Bitboard occ = position.getWhiteOccupancy() | position.getBlackOccupancy();
  int kingSquare = position.getPieces(position.isWhiteToMove() ? wk : bk).getLsb();

  // for each piece type, pretend there is that piece type on the king square, then see if that piece cancapture an actual enemy piece of that type
  // ignore kings because king can't check the other king
  checkers |= pawnAttacks(1ull<<kingSquare, isWhite) & position.getPieces(isWhite ? bp : wp);
  checkers |= m_knightMoves[kingSquare] & position.getPieces(isWhite ? bn : wn);
  checkers |= bishopMoves(kingSquare, occ) & position.getPieces(isWhite ? bb : wb);
  checkers |= rookMoves(kingSquare, occ) & position.getPieces(isWhite ? br : wr);
  checkers |= queenMoves(kingSquare, occ) & position.getPieces(isWhite ? bq : wq) ;

  return checkers;
}

// GROUP A SKILL: complex user-defined algorithms
std::vector<Move> MoveGenerator::genMoves(Position& position, bool onlyCaptures) {

  std::vector<Move> moveList;
  bool isWhite = position.isWhiteToMove();
  Bitboard own = isWhite ? position.getWhiteOccupancy() : position.getBlackOccupancy();
  Bitboard enemy = isWhite ? position.getBlackOccupancy() : position.getWhiteOccupancy();
  Bitboard occ = own|enemy;
  int kingSquare = position.getPieces(position.isWhiteToMove() ? wk : bk).getLsb();

  Bitboard checks = getCheckingPieces(position);
  Bitboard pushMask = 0xffffffffffffffff; // valid squares to move that block a check - if not in check, then this is all squares
  Bitboard captureMask = 0xffffffffffffffff; // valid squares to move that capture a checking piece - if not in check, then this is all squares
  // if in single check
  if(checks.popcnt() == 1) {
    captureMask = checks;
    pushMask = m_rookPushMasks[kingSquare][checks.getLsb()] | m_bishopPushMasks[kingSquare][checks.getLsb()];
  }

  // king moves (only one king)
  Bitboard moves = m_kingMoves[kingSquare];
  Bitboard dangerSquares = getDangerSquares(position);
  moves &= ~own;
  moves &= ~dangerSquares;
  if(onlyCaptures) moves &= enemy;
  while(moves.getBits()) {
    int end = moves.popLsb();
    Move m = Move(kingSquare, end, isWhite ? wk : bk, false, false, false);
    moveList.push_back(m);
  }
  // castling
  if(!onlyCaptures && checks.getBits()==0) {
    if(isWhite) {
      if(
        position.canWhiteCastleKingside()
        && (occ&96) == 0 // f1,g1 are unoccupied
        && (dangerSquares&96) == 0 // f1,g1 are unattacked
      ) moveList.push_back(Move(4, 6, wk, 1, false, false));
      if(
        position.canWhiteCastleQueenside()
        && (occ&14) == 0 // b1,c1,d1 are unoccupied
        && (dangerSquares&12) == 0 // c1,d1 are unattacked
      ) moveList.push_back(Move(4, 2, wk, 2, false, false));
    } else {
      if(
        position.canBlackCastleKingside()
        && (occ&(96ull<<56)) == 0 // f8,g8 are unoccupied
        && (dangerSquares&(96ull<<56)) == 0 // f8,g8 are unattacked
      ) moveList.push_back(Move(60, 62, bk, 3, false, false));
      if(
        position.canBlackCastleQueenside()
        && (occ&(14ull<<56)) == 0 // b8,c8,d8 are unoccupied
        && (dangerSquares&(12ull<<56)) == 0 // c8,d8 are unattacked
      ) moveList.push_back(Move(60, 58, bk, 4, false, false));
    }
  }

  // if in double check, then can only move king
  if(checks.popcnt() > 1) return moveList;

  // PINNED PIECE MOVES
  Bitboard pinnedPieces = 0;
  // for each of the opponents sliding pieces (bishop, rook, queen), find pieces that are in between our
  // king and the sliding piece (if applicable/if any) - if only one found and it is ours then that piece is pinned,
  // and the pinned piece's legal moves are a subset of the push mask (along with capturing the pinning piece)
  for(int t=wb; t<=wq; ++t) {
    Bitboard enemyPieces = position.getPieces((PieceType)(t + isWhite*6));
    while(enemyPieces.getBits()) {
      int slidingPiece = enemyPieces.popLsb();
      Bitboard squaresBetween = 0;
      if(t!=wb) squaresBetween |= m_rookPushMasks[kingSquare][slidingPiece];
      if(t!=wr) squaresBetween |= m_bishopPushMasks[kingSquare][slidingPiece];
      Bitboard piecesBetween = squaresBetween & (own|enemy);
      if(piecesBetween.popcnt() == 1 && (piecesBetween&enemy) == 0) { // if only one piece in between, and that piece isn't an enemy piece
        pinnedPieces |= piecesBetween;
        // different piece types have different move options e.g. knights can never move when pinned
        PieceType type = position.whichPiece(piecesBetween.getLsb());
        if(type==wp || type==bp) {
          // if the pinned piece is a pawn
          int index = piecesBetween.getLsb();
          Bitboard pushes = pawnPushes(piecesBetween, isWhite, occ) & squaresBetween;
          Bitboard attacks = pawnAttacks(piecesBetween, isWhite) & (squaresBetween|(1ull<<slidingPiece));
          Bitboard captures = attacks & enemy;
          Bitboard moves = (captures & captureMask) | (pushes & pushMask);
          if(onlyCaptures) moves &= enemy;

          Bitboard nonPromotions = isWhite ? (moves & ~eigthRank) : (moves & ~firstRank);
          while(nonPromotions.getBits()) moveList.push_back(Move(index, nonPromotions.popLsb(), type, false, false, false));
          Bitboard promotions = isWhite ? (moves & eigthRank) : (moves & firstRank);
          while(promotions.getBits()) {
            int end = promotions.popLsb(); 
            moveList.push_back(Move(index, end, type, false, isWhite ? wn : bn, false));
            moveList.push_back(Move(index, end, type, false, isWhite ? wb : bb, false));
            moveList.push_back(Move(index, end, type, false, isWhite ? wr : br, false));
            moveList.push_back(Move(index, end, type, false, isWhite ? wq : bq, false));
          }

          // en passant moves
          Bitboard enPassant = position.getEnPassant();
          if(enPassant.getBits()) {
            Bitboard epPushes = attacks & enPassant & (thirdRank|sixthRank) & squaresBetween; // ep pushes (i.e. where the pawn ends up) must be on the third/sixth rank
            Bitboard epCaptures = enPassantCaptures(1ull<<index) & enPassant & fourthFifthRank & squaresBetween; // ep captures (i.e. where the captured pawn is) must be on fourth/fifth rank
            Bitboard epMoves = isWhite ? (epCaptures&captureMask)<<8 : (epCaptures&captureMask)>>8;
            epMoves |= epPushes & pushMask;
            while(epMoves.getBits()) moveList.push_back(Move(index, epMoves.popLsb(), type, false, false, true));
          }
        } else if(t!=wr && (type==wb || type==bb)) {// bishop can't move if pinned by rook
          // if the pinned piece is a bishop
          int index = piecesBetween.getLsb();
          Bitboard moves = bishopMoves(piecesBetween.getLsb(), occ) & (squaresBetween|(1ull<<slidingPiece));
          moves &= ~own;
          if(onlyCaptures) moves &= enemy;
          moves = (moves & captureMask) | (moves & pushMask);
          while(moves.getBits()) moveList.push_back(Move(index, moves.popLsb(), type, false, false, false));
        } else if(t!=wb && (type==wr || type==br)) {// rook can't move if pinned by bishop
          // if the pinned piece is a rook
          int index = piecesBetween.getLsb();
          Bitboard moves = rookMoves(piecesBetween.getLsb(), occ) & (squaresBetween|(1ull<<slidingPiece));
          moves &= ~own;
          if(onlyCaptures) moves &= enemy;
          moves = (moves & captureMask) | (moves & pushMask);
          while(moves.getBits()) moveList.push_back(Move(index, moves.popLsb(), type, false, false, false));
        } else if(type==wq || type==bq) {
          // if the pinned piece is a queen
          int index = piecesBetween.getLsb();
          Bitboard moves = queenMoves(piecesBetween.getLsb(), occ) & (squaresBetween|(1ull<<slidingPiece));
          moves &= ~own;
          if(onlyCaptures) moves &= enemy;
          moves = (moves & captureMask) | (moves & pushMask);
          while(moves.getBits()) moveList.push_back(Move(index, moves.popLsb(), type, false, false, false));
        }
      }
    }
  }

  // normal pawn moves
  PieceType type = isWhite ? wp : bp;
  Bitboard i = position.getPieces(type) & ~pinnedPieces;
  Bitboard enPassant = position.getEnPassant();
  while(i.getBits()) {

    int index = i.popLsb();
    Bitboard pushes = pawnPushes(1ull<<index, isWhite, occ);
    Bitboard attacks = pawnAttacks(1ull<<index, isWhite);
    Bitboard captures = attacks & enemy;
    Bitboard moves = (captures & captureMask) | (pushes & pushMask);
    if(onlyCaptures) moves &= enemy;
    Bitboard nonPromotions = isWhite ? (moves & ~eigthRank) : (moves & ~firstRank);
    while(nonPromotions.getBits()) moveList.push_back(Move(index, nonPromotions.popLsb(), type, false, false, false));
    Bitboard promotions = isWhite ? (moves & eigthRank) : (moves & firstRank);
    while(promotions.getBits()) {
      int end = promotions.popLsb(); 
      moveList.push_back(Move(index, end, type, false, isWhite ? wn : bn, false));
      moveList.push_back(Move(index, end, type, false, isWhite ? wb : bb, false));
      moveList.push_back(Move(index, end, type, false, isWhite ? wr : br, false));
      moveList.push_back(Move(index, end, type, false, isWhite ? wq : bq, false));
    }

    // en passant
    if(enPassant.getBits()) {
      Bitboard epPushes = attacks & enPassant & (thirdRank|sixthRank); // ep pushes (i.e. where the pawn ends up) must be on the third/sixth rank
      Bitboard epCaptures = enPassantCaptures(1ull<<index) & enPassant & fourthFifthRank; // ep captures (i.e. where the captured pawn is) must be on fourth/fifth rank
      Bitboard epMoves = isWhite ? (epCaptures&captureMask)<<8 : (epCaptures&captureMask)>>8;
      epMoves |= epPushes & pushMask;
      bool valid = true;
      if(epMoves.getBits() && ( // en passant moves exist
            (isWhite && (kingSquare>>3) == 4) // white to move and white king on 5th rank
            || (!isWhite && (kingSquare>>3) == 3) // black to move and black king on 4th rank
        )) {
        // deal with en passant double reveal pin issue
        Position pawnsRemoved = position;
        pawnsRemoved.removePieces(type, 1ull<<index);
        pawnsRemoved.removePieces(isWhite ? bp : wp, epCaptures);
        if(getCheckingPieces(pawnsRemoved).getBits()) valid = false;
      }
      if(valid) {
        while(epMoves.getBits()) moveList.push_back(Move(index, epMoves.popLsb(), type, false, false, true));
      }
    }
  }

  // normal moves for each other piece type
  for(int t=wn; t<wk; ++t) {
    PieceType type = (PieceType) (t + (!isWhite)*6);
    Bitboard i = position.getPieces(type) & ~pinnedPieces;
    while(i.getBits()) {
      Bitboard moves = 0;
      int index = i.popLsb();
      switch(t) {
        case wn:
          moves |= m_knightMoves[index];
          break;
        case wb:
          moves |= bishopMoves(index, occ);
          break;
        case wr:
          moves |= rookMoves(index, occ);
          break;
        case wq:
          moves |= queenMoves(index, occ);
          break;
      }
      moves &= ~own;
      if(onlyCaptures) moves &= enemy;
      moves = (moves & captureMask) | (moves & pushMask);
      while(moves.getBits()) moveList.push_back(Move(index, moves.popLsb(), type, false, false, false));
    }
  }

  return moveList;

}
