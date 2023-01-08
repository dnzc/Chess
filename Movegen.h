#include "Bitboard.h"
#include "Position.h"
#include "Move.h"
#include <vector>

class Movegen {

  public:
    Movegen();
    std::vector<Move> genMoves(Position& position);

  private:

    void initKnightMoveTable();
    void initKingMoveTable();
    void initRookBishopMoveTable(bool isRook);
    void initPushMasks();

    void findRookBishopMagics(bool isRook);
    void initRookBishopBlockerMasks(bool isRook);

    // slow, used for generation at start only
    Bitboard getBlockerBoard(Bitboard mask, int index);
    Bitboard getRookBishopMoveBoard(bool isRook, Bitboard blockerBoard, int square);

    // pseudo-legal pawn move generation
    Bitboard pawnPushes(Bitboard pawns, bool isWhite, Bitboard occupancy);
    Bitboard pawnAttacks(Bitboard pawns, bool isWhite);
    Bitboard enPassantCaptures(Bitboard pawns);

    // pseudo-legal move generation functions for sliding pieces
    Bitboard bishopMoves(int square, Bitboard occupancy);
    Bitboard rookMoves(int square, Bitboard occupancy);
    Bitboard queenMoves(int square, Bitboard occupancy);

    // danger squares are squares attacked by an enemy piece,
    // ignoring your own king to avoid issues when in check from sliding piece
    Bitboard getDangerSquares(Position& position);

    Bitboard getCheckingPieces(Position& position);

    // useful for removing pieces on the A or H file when calculating pawn attacks
    Bitboard notAFile = ~0x0101010101010101; 
    Bitboard notHFile = ~0x8080808080808080;

    // useful for calculating double pawn moves
    Bitboard thirdSixthRank = 0x0000ff0000ff0000;
    // useful for dealing with en passant
    Bitboard fourthFifthRank = 0x000000ffff000000;

    // STATIC LOOKUP TABLES: the key, k, is a board square
    
    // sliding piece moves between square k and another square: useful when calculating valid places to block sliding piece checks
    Bitboard m_pushMasks[64][64];

    // possible knight moves from square k
    Bitboard m_knightMoves[64];
    // possible king moves from square k
    Bitboard m_kingMoves[64];

    // possible positions of a piece that would block a rook on square k ("blockers")
    // note: a piece on the board edge can't be a blocker because it could be captured, not obstructing movement
    Bitboard m_rookMasks[64];
    // rook moves on square k, "magic-indexed" (hashed) by an arrangement of blocker pieces
    // note: at most 4096 possible blocker configurations (i.e. table indices) because a rook has at most 12 potential blockers
    Bitboard m_rookMoves[64][4096];

    // same as above, for bishops
    Bitboard m_bishopMasks[64];
    Bitboard m_bishopMoves[64][512];

    // SLIDING PIECE MAGICS
    // these were found by running the findRookBishopMagics function in Board.cpp
    Bitboard m_rookMagics[64] = {
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

    Bitboard m_bishopMagics[64] = {
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
