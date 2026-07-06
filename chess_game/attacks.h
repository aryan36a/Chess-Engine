#ifndef ATTACK_H
#define ATTACK_H

#include<stdbool.h>

//Individual Attack Detection
bool pawnAttacks(int square,bool byWhite);

bool knightAttacks(int square,bool byWhite);

bool bishopAttacks(int square,bool byWhite);

bool rookAttacks(int square,bool byWhite);

bool kingAttacks(int square,bool byWhite);

#endif