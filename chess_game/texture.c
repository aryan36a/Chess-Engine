#include<raylib.h>
#include "texture.h"

Texture2D pieceTextures[13];

//Texture Loading
void LoadPieceTextures(void){

    pieceTextures[WHITE_PAWN]   = LoadTexture("../assets/pieces/white-pawn.png");
    pieceTextures[WHITE_KNIGHT] = LoadTexture("../assets/pieces/white-knight.png");
    pieceTextures[WHITE_BISHOP] = LoadTexture("../assets/pieces/white-bishop.png");
    pieceTextures[WHITE_ROOK]   = LoadTexture("../assets/pieces/white-rook.png");
    pieceTextures[WHITE_QUEEN]  = LoadTexture("../assets/pieces/white-queen.png");
    pieceTextures[WHITE_KING]   = LoadTexture("../assets/pieces/white-king.png");

    pieceTextures[BLACK_PAWN]   = LoadTexture("../assets/pieces/black-pawn.png");
    pieceTextures[BLACK_KNIGHT] = LoadTexture("../assets/pieces/black-knight.png");
    pieceTextures[BLACK_BISHOP] = LoadTexture("../assets/pieces/black-bishop.png");
    pieceTextures[BLACK_ROOK]   = LoadTexture("../assets/pieces/black-rook.png");
    pieceTextures[BLACK_QUEEN]  = LoadTexture("../assets/pieces/black-queen.png");
    pieceTextures[BLACK_KING]   = LoadTexture("../assets/pieces/black-king.png");
}

//Texture Unloading
void UnloadPieceTextures(void){

    for(int i=WHITE_PAWN;i<=BLACK_KING;i++){
        UnloadTexture(pieceTextures[i]);
    }
}