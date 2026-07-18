#ifndef FEN_H
#define FEN_H

#include<stdbool.h>

bool loadFEN(const char *fen);

char *exportFEN(void);

int squareFromString(const char *str);

void squareToString(int square,char *buffer);

#endif