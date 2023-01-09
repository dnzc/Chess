#pragma once

#include "Bitboard.h"
#include "Position.h"
#include <string>

struct Util {
    static void display(Position position);
    static void display(Bitboard bitboard);
    static void display(std::string symbols[64]);
};
