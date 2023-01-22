#pragma once

#include "Move.h"
#include "Bitboard.h"
#include <string>
#include <cstdint>
#include <vector>

// GROUP A SKILL - complex OOP
class Position {

  public:

    Position();
    Position(std::string FEN);
    //Position(const Position& position);

    void makeMove(Move move);
    void undoMove(Move move, PieceType capturedPiece, Bitboard prevPassant, int prev50);

    Bitboard getWhiteOccupancy();
    Bitboard getBlackOccupancy();
    Bitboard getPieces(PieceType pt);
    PieceType whichPiece(int square);
    bool isWhiteToMove();
    bool canWhiteCastleKingside();
    bool canWhiteCastleQueenside();
    bool canBlackCastleKingside();
    bool canBlackCastleQueenside();
    int getPlysSince50();
    Bitboard getEnPassant();

    void removePieces(PieceType pt, Bitboard bb);

  private:
    // array of which piece is on each square, so that "what piece is on this square?"
    // can be answered quickly (slow with bitboards)
    PieceType m_board[64];

    bool m_whiteToMove;
    // castling rights
    bool m_whiteCastleKingside;
    bool m_whiteCastleQueenside;
    bool m_blackCastleKingside;
    bool m_blackCastleQueenside;

    int m_plysSince50; // number of plys since 50 move rule was reset

    // bitboards for each (piece type, colour) pair
    Bitboard m_pieces[12];

    // squares that could be affected by en passant next move
    Bitboard m_enPassant;

};
