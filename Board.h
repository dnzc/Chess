#pragma once

#include <string>
#include <cstdint>

class Board {

  public:

    Board();

    void displayBoard();
    static void display8x8(uint64_t bitboard);
    static void display8x8(std::string symbols[64]);

    void makeMove(int start, int end);

    uint64_t lookupKnightMoves(int square);
    uint64_t lookupRookMoves(int square, uint64_t occupancyBoard);
    uint64_t lookupBishopMoves(int square, uint64_t occupancyBoard);

  private:

    void initKnightMoveTable();

    void findRookBishopMagics(bool isRook);
    void initRookBishopBlockerMasks(bool isRook);
    void initRookBishopMoveTable(bool isRook);

    uint64_t getBlockerBoard(uint64_t mask, int index);
    uint64_t getRookBishopMoveBoard(bool isRook, uint64_t blockerBoard, int square);

    // pawns, knights, bishops, rooks, queens, king; white, black: used as indices for m_pieces and m_symbols, for readability
    enum BitboardIndex {
      wp, wn, wb, wr, wq, wk,
      bp, bn, bb, br, bq, bk,
    };

    // BITBOARD INDEX SYSTEM:
    // 56 57 58 59 60 61 62 63
    // 48 49 50 51 52 53 54 55
    // 40 41 42 43 44 45 46 47
    // 32 33 34 35 36 37 38 39
    // 24 25 26 27 28 29 30 31
    // 16 17 18 19 20 21 22 23
    //  8  9 10 11 12 13 14 15
    //  0  1  2  3  4  5  6  7

    // bitboards and symbols for each (piece type, colour) pair
    uint64_t m_pieces[12];
    std::string m_symbols[12];

    // static lookup tables: the key, k, is a board square
    // possible knight moves from square k
    uint64_t m_knightMoves[64];

    // possible positions of a piece that would block a rook on square k ("blockers")
    // note: a piece on the board edge can't be a blocker because it could be captured, not obstructing movement
    uint64_t m_rookMasks[64];
    // rook moves on square k, magic-indexed ("hashed") by an arrangement of blocker pieces
    // note: at most 4096 possible blocker configurations (i.e. table indices) because a rook has at most 12 potential blockers
    uint64_t m_rookMoves[64][4096];

    // same as above, for bishops
    uint64_t m_bishopMasks[64];
    uint64_t m_bishopMoves[64][512];

    // rook and bishop magics: these were found by running the findRookBishopMagics function in Board.cpp
    uint64_t m_rookMagics[64] = {
      36029347062104064,
      594475289326649352,
      2341880671045427200,
      36033229430456322,
      72066390198452480,
      153124590649081984,
      1161929253650956352,
      36029346778980608,
      633318968131648,
      70368947605760,
      2286984319995906,
      285873577918472,
      141046726197376,
      562984380334608,
      140746615226497,
      281476059496704,
      72705208553472,
      18983757561856,
      2314850775412510720,
      35192962171136,
      352960412909569,
      35734170107912,
      282608856465472,
      4503737070551312,
      563091704152064,
      581092031594504,
      2251941682872328,
      5501854683136,
      1125904235888656,
      11020902859776,
      292470629875744,
      283675073716352,
      35322214219904,
      2251937523171456,
      4574037125636096,
      35343302660096,
      17600792758400,
      87969553711368,
      36591748050321536,
      288265561599639684,
      4504149392754688,
      2331034444111872,
      9570252354488320,
      2251817035530248,
      18032003580495872,
      70385924080128,
      282024867006466,
      288230513625268744,
      598168686297632,
      17593268177168,
      288239176540749954,
      9079767089291296,
      562988616516624,
      36029363954712832,
      627069420548,
      4785075677896720,
      140737758953537,
      70368886788129,
      281612416204817,
      2199292223554,
      17626546110465,
      2233458753537,
      8864829407364,
      275423428866,
    };

    uint64_t m_bishopMagics[64] = {
      9007345417850912,
      9024868754395136,
      4507998211022976,
      36112360036892736,
      422487880107008,
      18049694684938240,
      577586927356870656,
      576482776962834432,
      2305843318484897824,
      558480098308,
      9016005028225024,
      290314023600192,
      576601765776986112,
      567207264320,
      4535619813760,
      140806476792960,
      18014553130434592,
      144396671643811968,
      2251869611100168,
      2306968978914025472,
      4582764734054400,
      70368878411776,
      4522291870310400,
      2207622645760,
      70369029652736,
      2251937420427520,
      18700292064256,
      18018796560253088,
      153932173156352,
      1125968644276352,
      9070971715712,
      71469329682448,
      2256206450393152,
      44547401318432,
      17635136798784,
      2305878227945588752,
      2308097025230508048,
      2306968978377017344,
      144185591733422080,
      565157568712832,
      4611721340506873860,
      1126041774981280,
      2305847683216052224,
      18014536087371808,
      9380346986624,
      2252144493199368,
      1155175503645114880,
      26388816461968,
      17867332911105,
      9012181483945984,
      142004457488,
      36028797325295616,
      140772930256928,
      4855444426260480,
      148621055448056832,
      149533850091520,
      70385932472352,
      8592056368,
      144115222636922881,
      70368760987912,
      17302032,
      138579829248,
      2203989319688,
      18015498591666240,
    };

};
