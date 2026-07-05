#ifndef TEXTURE_H
#define TEXTURE_H

#include<raylib.h>
#include "board.h"

extern Texture2D pieceTextures[13];

//Prototypes
void LoadPieceTextures(void);
void UnloadPieceTextures(void);

#endif