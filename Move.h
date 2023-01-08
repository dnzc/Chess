#pragma once

struct Move {
    Move(int start, int end, int piece, bool enPassant, bool castle) : start(start), end(end), piece(piece), enPassant(enPassant), castle(castle) {};
    int start;
    int end;
    int piece;
    bool enPassant;
    bool castle;
};
