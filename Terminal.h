#pragma once

#include "Bitboard.h"
#include "Position.h"
#include <string>

struct Terminal {
    static void display(Position& position);
    static void display8x8(Bitboard bitboard);
    static void display8x8(std::string symbols[64]);
};
