#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "board.h"
#include "fen.h"
#include "search.h"

#define UCI_LINE_SIZE 4096
#define UCI_SEARCH_DEPTH 5
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

static int squareFromText(const char *s) {
    if (!s || s[0] < 'a' || s[0] > 'h' || s[1] < '1' || s[1] > '8') return -1;
    int file = s[0] - 'a';
    int rank = '8' - s[1];
    return rank * 8 + file;
}

static void squareToText(int square, char out[3]) {
    out[0] = (char)('a' + square % 8);
    out[1] = (char)('8' - square / 8);
    out[2] = '\0';
}

static int parseMoveToken(const char *token, Move *out) {
    if (!token || strlen(token) < 4) return 0;
    int from = squareFromText(token);
    int to = squareFromText(token + 2);
    if (from < 0 || to < 0) return 0;

    Move legal[256];
    int count = generateAllMoves(currentTurn == WHITE_TURN, legal);
    for (int i = 0; i < count; ++i) {
        if (legal[i].from == from && legal[i].to == to) {
            *out = legal[i];
            return 1;
        }
    }
    return 0;
}

static int applyMoveToken(const char *token) {
    Move move;
    if (!parseMoveToken(token, &move)) return 0;
    Piece moving = GetPieceAtSquare(move.from);
    makeMove(move);
    if ((moving == WHITE_PAWN && move.to < 8) ||
        (moving == BLACK_PAWN && move.to >= 56)) {
        /* The current Move representation does not encode promotion choice. */
        finalizePromotionIfNeeded(move);
    }
    toggleTurn();
    if (board.historyCount < MAX_POSITION_HISTORY)
        board.positionHistory[board.historyCount++] = generateHash();
    return 1;
}

static int setPosition(char *args) {
    char *save = NULL;
    char *token = strtok_s(args, " \t\r\n", &save);
    if (!token) return 0;

    if (strcmp(token, "startpos") == 0) {
        initBoard();
        token = strtok_s(NULL, " \t\r\n", &save);
    } else if (strcmp(token, "fen") == 0) {
        char fen[512] = {0};
        size_t used = 0;
        int fields = 0;
        while ((token = strtok_s(NULL, " \t\r\n", &save)) != NULL && strcmp(token, "moves") != 0) {
            size_t n = strlen(token);
            if (used + n + 2 >= sizeof(fen)) return 0;
            if (used) fen[used++] = ' ';
            memcpy(fen + used, token, n);
            used += n;
            fen[used] = '\0';
            fields++;
            if (fields == 6) {
                token = strtok_s(NULL, " \t\r\n", &save);
                break;
            }
        }
        if (fields != 6 || !loadFEN(fen)) return 0;
        if (token && strcmp(token, "moves") != 0) return 0;
        if (!token) return 1;
        token = strtok_s(NULL, " \t\r\n", &save);
    } else {
        return 0;
    }

    if (token && strcmp(token, "moves") == 0)
        token = strtok_s(NULL, " \t\r\n", &save);
    while (token) {
        if (!applyMoveToken(token)) return 0;
        token = strtok_s(NULL, " \t\r\n", &save);
    }
    return 1;
}

static void printBestMove(int depth) {
    Move best = findBestMove(depth);
    if (best.from < 0 || best.from >= 64 || best.to < 0 || best.to >= 64) {
        puts("bestmove 0000");
        fflush(stdout);
        return;
    }
    char from[3], to[3];
    squareToText(best.from, from);
    squareToText(best.to, to);
    int score = GetSearchScore();
    if(currentTurn == BLACK_TURN) score = -score;
    printf("info depth %d nodes %llu score cp %d pv %s%s\n",
           depth, GetSearchNodeCount(), score, from, to);
    printf("bestmove %s%s", from, to);
    Piece moving = GetPieceAtSquare(best.from);
    if ((moving == WHITE_PAWN && best.to < 8) || (moving == BLACK_PAWN && best.to >= 56))
        printf("q");
    putchar('\n');
    fflush(stdout);
}

int runUciMode(void) {
    char line[UCI_LINE_SIZE];
    initZobrist();
    initBoard();

    while (fgets(line, sizeof(line), stdin)) {
        size_t n = strlen(line);
        while (n && (line[n - 1] == '\n' || line[n - 1] == '\r')) line[--n] = '\0';
        if (strcmp(line, "uci") == 0) {
            puts("id name Aryan Chess Engine");
            puts("id author Aryan Sangave");
            puts("uciok");
            fflush(stdout);
        } else if (strcmp(line, "isready") == 0) {
            puts("readyok");
            fflush(stdout);
        } else if (strcmp(line, "ucinewgame") == 0) {
            initBoard();
        } else if (strncmp(line, "position ", 9) == 0) {
            char args[UCI_LINE_SIZE];
            strncpy(args, line + 9, sizeof(args) - 1);
            args[sizeof(args) - 1] = '\0';
            if (!setPosition(args))
                fprintf(stderr, "info string invalid position command\n");
        } else if (strncmp(line, "go ", 3) == 0 || strcmp(line, "go") == 0) {
            /* This engine is intentionally benchmarked at a fixed depth. */
            printBestMove(UCI_SEARCH_DEPTH);
        } else if (strcmp(line, "stop") == 0) {
            /* Search is synchronous; go completes before the next command is read. */
        } else if (strcmp(line, "quit") == 0) {
            break;
        } else if (strcmp(line, "debug on") == 0 || strcmp(line, "debug off") == 0) {
            /* Accepted for protocol compatibility. */
        }
    }
    return 0;
}
