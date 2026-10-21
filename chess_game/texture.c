#include<raylib.h>
#include<stdio.h>
#include "texture.h"

Texture2D pieceTextures[13];

static Texture2D LoadPieceTexture(const char *filename){
    char path[512];
    const char *applicationDirectory = GetApplicationDirectory();
    int written = snprintf(path, sizeof(path), "%s../assets/pieces/%s",
                           applicationDirectory, filename);
    if(written < 0 || (size_t)written >= sizeof(path)){
        TraceLog(LOG_ERROR, "Piece texture path is too long: %s", filename);
        return (Texture2D){0};
    }
    return LoadTexture(path);
}

//Texture Loading
void LoadPieceTextures(void){

    pieceTextures[WHITE_PAWN]   = LoadPieceTexture("white-pawn.png");
    pieceTextures[WHITE_KNIGHT] = LoadPieceTexture("white-knight.png");
    pieceTextures[WHITE_BISHOP] = LoadPieceTexture("white-bishop.png");
    pieceTextures[WHITE_ROOK]   = LoadPieceTexture("white-rook.png");
    pieceTextures[WHITE_QUEEN]  = LoadPieceTexture("white-queen.png");
    pieceTextures[WHITE_KING]   = LoadPieceTexture("white-king.png");

    pieceTextures[BLACK_PAWN]   = LoadPieceTexture("black-pawn.png");
    pieceTextures[BLACK_KNIGHT] = LoadPieceTexture("black-knight.png");
    pieceTextures[BLACK_BISHOP] = LoadPieceTexture("black-bishop.png");
    pieceTextures[BLACK_ROOK]   = LoadPieceTexture("black-rook.png");
    pieceTextures[BLACK_QUEEN]  = LoadPieceTexture("black-queen.png");
    pieceTextures[BLACK_KING]   = LoadPieceTexture("black-king.png");
}

//Texture Unloading
void UnloadPieceTextures(void){

    for(int i=WHITE_PAWN;i<=BLACK_KING;i++){
        UnloadTexture(pieceTextures[i]);
    }
}