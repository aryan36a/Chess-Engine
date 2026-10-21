#include "dev_mode.h"

#include <raylib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "board.h"
#include "evaluate.h"
#include "input.h"
#include "search.h"
#include "strategy/threat_analysis.h"
#include "strategy/opponent_model.h"
#include "strategy/goal_manager.h"

#define DEV_LINE_COUNT 24
#define DEV_LINE_LENGTH 128
#define DEV_MAX_CANDIDATES 5
#define DEV_MAX_MOVES 256

typedef struct {
    Move move;
    int score;
    Piece captured;
    bool givesCheck;
} CandidateMove;

static bool devVisible;
static int keySequenceIndex;
static double lastKeyTime;
static Board previousBoard;
static Turn previousTurn;
static uint64_t previousHash;
static bool snapshotReady;
static OpponentProfile opponentProfile;
static char devLines[DEV_LINE_COUNT][DEV_LINE_LENGTH];
static int devLineCount;
static CandidateMove topCandidates[DEV_MAX_CANDIDATES];
static int topCandidateCount;
static FILE *decisionLog;

static void logTimestamp(void) {
    if (!decisionLog) return;
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    if (local) fprintf(decisionLog, "[%04d-%02d-%02d %02d:%02d:%02d] ",
        local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
        local->tm_hour, local->tm_min, local->tm_sec);
}

static void logLine(const char *text) {
    if (!decisionLog) return;
    logTimestamp();
    fprintf(decisionLog, "%s\n", text ? text : "");
    fflush(decisionLog);
}

static const char *pieceName(Piece p) {
    switch (p) {
        case WHITE_PAWN: case BLACK_PAWN: return "pawn";
        case WHITE_ROOK: case BLACK_ROOK: return "rook";
        case WHITE_KNIGHT: case BLACK_KNIGHT: return "knight";
        case WHITE_BISHOP: case BLACK_BISHOP: return "bishop";
        case WHITE_QUEEN: case BLACK_QUEEN: return "queen";
        case WHITE_KING: case BLACK_KING: return "king";
        default: return "none";
    }
}

static int pieceValue(Piece p) {
    switch (p) {
        case WHITE_PAWN: case BLACK_PAWN: return 100;
        case WHITE_KNIGHT: case BLACK_KNIGHT: return 320;
        case WHITE_BISHOP: case BLACK_BISHOP: return 330;
        case WHITE_ROOK: case BLACK_ROOK: return 500;
        case WHITE_QUEEN: case BLACK_QUEEN: return 900;
        case WHITE_KING: case BLACK_KING: return 20000;
        default: return 0;
    }
}

static void squareName(int square, char out[3]) {
    if (square < 0 || square >= 64) {
        out[0] = '-'; out[1] = '-'; out[2] = '\0';
        return;
    }
    out[0] = (char)('a' + square % 8);
    out[1] = (char)('8' - square / 8);
    out[2] = '\0';
}

static void moveName(Move move, char out[8]) {
    char from[3], to[3];
    squareName(move.from, from);
    squareName(move.to, to);
    snprintf(out, 8, "%s%s", from, to);
}

static const char *intentName(OpponentIntentType type) {
    switch (type) {
        case INTENT_CHECKING_ATTACK: return "checking attack";
        case INTENT_QUEEN_ATTACK: return "queen pressure";
        case INTENT_KING_SIDE_ATTACK: return "king-side attack";
        case INTENT_CASTLING: return "castling";
        case INTENT_MATERIAL_HUNT: return "material hunt";
        default: return "unknown intent";
    }
}

static const char *goalName(StrategicGoalType type) {
    switch (type) {
        case GOAL_NEUTRALIZE_KING_THREAT: return "neutralize king threat";
        case GOAL_SAVE_QUEEN: return "save queen";
        case GOAL_EXPLOIT_HANGING_PIECE: return "exploit hanging piece";
        case GOAL_PROTECT_HANGING_PIECE: return "protect hanging piece";
        case GOAL_ATTACK_OPPONENT_QUEEN: return "attack opponent queen";
        case GOAL_PREVENT_CASTLING: return "prevent castling";
        case GOAL_IMPROVE_KING_SAFETY: return "improve king safety";
        default: return "unknown goal";
    }
}

static void addHudLine(const char *fmt, ...) {
    if (devLineCount >= DEV_LINE_COUNT) return;
    va_list args;
    va_start(args, fmt);
    vsnprintf(devLines[devLineCount], DEV_LINE_LENGTH, fmt, args);
    va_end(args);
    ++devLineCount;
}

static int centerBonus(int square) {
    int row = square / 8, col = square % 8;
    int distance = abs(3 - row) + abs(3 - col);
    return 6 - distance;
}

static void rankOpponentCandidates(bool humanWhite) {
    topCandidateCount = 0;
    if ((currentTurn == WHITE_TURN) != humanWhite) return;

    Move savedMoves[256];
    memcpy(savedMoves, moves, sizeof(savedMoves));
    int savedMoveCount = moveCount;
    int savedSelectedSquare = selectedSquare;
    Piece savedSelectedPiece = selectedPiece;
    Board position = board;
    Turn savedTurn = currentTurn;
    Move candidates[DEV_MAX_MOVES];
    int count = generateAllMoves(humanWhite, candidates);
    if (count > DEV_MAX_MOVES) count = DEV_MAX_MOVES;

    CandidateMove ranked[DEV_MAX_MOVES];
    int rankedCount = 0;
    int before = evaluate();

    for (int i = 0; i < count; ++i) {
        board = position;
        currentTurn = savedTurn;
        Move move = candidates[i];
        Piece captured = EMPTY;
        makeTemporaryMove(move, &captured);
        int whiteScore = evaluate();
        bool givesCheck = isKingInCheck(!humanWhite);
        undoTemporaryMove(move, captured);

        int score = humanWhite ? whiteScore : -whiteScore;
        score -= humanWhite ? before : -before;
        if (givesCheck) score += 250;
        score += centerBonus(move.to) * 4;
        if (captured != EMPTY) score += pieceValue(captured) / 8;

        ranked[rankedCount++] = (CandidateMove){move, score, captured, givesCheck};
    }

    for (int i = 1; i < rankedCount; ++i) {
        CandidateMove value = ranked[i];
        int j = i;
        while (j > 0 && ranked[j - 1].score < value.score) {
            ranked[j] = ranked[j - 1];
            --j;
        }
        ranked[j] = value;
    }

    topCandidateCount = rankedCount < DEV_MAX_CANDIDATES ? rankedCount : DEV_MAX_CANDIDATES;
    for (int i = 0; i < topCandidateCount; ++i) topCandidates[i] = ranked[i];

    board = position;
    currentTurn = savedTurn;
    memcpy(moves, savedMoves, sizeof(savedMoves));
    moveCount = savedMoveCount;
    selectedSquare = savedSelectedSquare;
    selectedPiece = savedSelectedPiece;
}

static void rebuildAnalysis(bool humanWhite, bool writeLog) {
    devLineCount = 0;
    topCandidateCount = 0;
    bool engineWhite = !humanWhite;
    ThreatReport threats;
    OpponentIntent intents[8];
    StrategicGoal goals[8];

    ThreatAnalysis_Analyze(&threats);
    int intentCount = OpponentModel_InferIntentions(
        &threats, engineWhite, &opponentProfile, intents, 8);
    int goalCount = GoalManager_Update(
        engineWhite, &threats, intents, intentCount, goals, 8);
    rankOpponentCandidates(humanWhite);

    addHudLine("Opponent model: %u observed moves | capture %.0f%% | check %.0f%% | castle %.0f%%",
        opponentProfile.observations, opponentProfile.captureRate * 100.0f,
        opponentProfile.checkRate * 100.0f, opponentProfile.castlingRate * 100.0f);
    const SideThreatSummary *engine = engineWhite ? &threats.white : &threats.black;
    const SideThreatSummary *human = humanWhite ? &threats.white : &threats.black;
    addHudLine("Threats: engine attacked=%d hanging=%d check=%s | human hanging=%d",
        engine->attackedPieces, engine->hangingPieces, engine->inCheck ? "YES" : "no",
        human->hangingPieces);
    addHudLine("Inferred opponent intentions (heuristic confidence, not probability):");
    if (intentCount == 0) addHudLine("  none strongly indicated by this position");
    for (int i = 0; i < intentCount && i < 3; ++i) {
        addHudLine("  %s  %.0f%%", intentName(intents[i].type), intents[i].confidence * 100.0f);
    }
    addHudLine("Current strategic goals:");
    if (goalCount == 0) addHudLine("  no urgent strategic goal");
    for (int i = 0; i < goalCount && i < 3; ++i) {
        addHudLine("  %s  priority %.2f", goalName(goals[i].type), goals[i].priority);
    }
    addHudLine("Opponent candidate moves (static heuristic ranking):");
    if ((currentTurn == WHITE_TURN) != humanWhite) {
        addHudLine("  waiting for engine move; opponent is not to move");
    } else if (topCandidateCount == 0) {
        addHudLine("  no legal moves");
    }
    for (int i = 0; i < topCandidateCount; ++i) {
        char notation[8];
        moveName(topCandidates[i].move, notation);
        const char *reason = topCandidates[i].givesCheck ? "check" :
            topCandidates[i].captured != EMPTY ? "capture" : "positional";
        addHudLine("  #%d %s  score %+d  (%s)", i + 1, notation,
                   topCandidates[i].score, reason);
    }
    addHudLine("Search: %s | nodes %llu | last score %d",
        IsComputerThinking() ? "running" : "idle",
        GetSearchNodeCount(), GetSearchScore());

    if (!writeLog) return;
    logLine("=== POSITION DECISION TRACE ===");
    char line[DEV_LINE_LENGTH];
    snprintf(line, sizeof(line), "Side to move: %s; engine side: %s",
        currentTurn == WHITE_TURN ? "White" : "Black",
        engineWhite ? "White" : "Black");
    logLine(line);
    snprintf(line, sizeof(line), "Opponent observations=%u captureRate=%.3f checkRate=%.3f castleRate=%.3f",
        opponentProfile.observations, opponentProfile.captureRate,
        opponentProfile.checkRate, opponentProfile.castlingRate);
    logLine(line);
    snprintf(line, sizeof(line), "Threats: engine attacked=%d hanging=%d inCheck=%d; opponent hanging=%d",
        engine->attackedPieces, engine->hangingPieces, engine->inCheck,
        human->hangingPieces);
    logLine(line);
    for (int i = 0; i < intentCount; ++i) {
        snprintf(line, sizeof(line), "INFERENCE: %s confidence=%.3f (heuristic)",
            intentName(intents[i].type), intents[i].confidence);
        logLine(line);
    }
    for (int i = 0; i < goalCount; ++i) {
        snprintf(line, sizeof(line), "GOAL: %s priority=%.3f confidence=%.3f",
            goalName(goals[i].type), goals[i].priority, goals[i].confidence);
        logLine(line);
    }
    for (int i = 0; i < topCandidateCount; ++i) {
        char notation[8];
        moveName(topCandidates[i].move, notation);
        snprintf(line, sizeof(line), "CANDIDATE #%d: %s score=%d capture=%s check=%d",
            i + 1, notation, topCandidates[i].score,
            pieceName(topCandidates[i].captured), topCandidates[i].givesCheck);
        logLine(line);
    }
    snprintf(line, sizeof(line), "SEARCH: state=%s nodes=%llu lastScore=%d",
        IsComputerThinking() ? "running" : "idle",
        GetSearchNodeCount(), GetSearchScore());
    logLine(line);
}

void DevMode_Init(void) {
    OpponentModel_Init(&opponentProfile);
    decisionLog = fopen("engine_dev.log", "a");
    if (decisionLog) {
        logLine("");
        logLine("=== NEW DEVELOPER DIAGNOSTICS SESSION ===");
    }
    previousBoard = board;
    previousTurn = currentTurn;
    previousHash = generateHash();
    snapshotReady = true;
}

static bool digitPressed(int digit) {
    switch (digit) {
        case 1: return IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1);
        case 2: return IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2);
        case 3: return IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3);
        case 5: return IsKeyPressed(KEY_FIVE) || IsKeyPressed(KEY_KP_5);
        case 8: return IsKeyPressed(KEY_EIGHT) || IsKeyPressed(KEY_KP_8);
        default: return false;
    }
}

void DevMode_HandleHotkey(void) {
    static const int sequence[] = {1, 1, 2, 3, 5, 8};
    int pressed = 0;
    const int digits[] = {1, 2, 3, 5, 8};
    for (int i = 0; i < 5; ++i) {
        if (digitPressed(digits[i])) { pressed = digits[i]; break; }
    }
    if (!pressed) {
        if (keySequenceIndex > 0 && GetTime() - lastKeyTime > 2.5) keySequenceIndex = 0;
        return;
    }
    lastKeyTime = GetTime();
    if (pressed == sequence[keySequenceIndex]) {
        ++keySequenceIndex;
    } else {
        keySequenceIndex = pressed == 1 ? 1 : 0;
    }
    if (keySequenceIndex == (int)(sizeof(sequence) / sizeof(sequence[0]))) {
        devVisible = !devVisible;
        keySequenceIndex = 0;
        logLine(devVisible ? "DEV HUD ENABLED by 112358 hotkey" :
                              "DEV HUD DISABLED by 112358 hotkey");
        if (devVisible) rebuildAnalysis(!IsBoardFlipped(), true);
    }
}

void DevMode_Update(bool humanPlaysWhite) {
    uint64_t hash = generateHash();
    if (!snapshotReady) {
        previousBoard = board;
        previousTurn = currentTurn;
        previousHash = hash;
        snapshotReady = true;
        return;
    }
    if (hash == previousHash) return;

    Move last = {-1, -1};
    GetLastMove(&last);
    Board currentBoard = board;
    Turn currentTurnNow = currentTurn;
    Piece moving = EMPTY, captured = EMPTY;
    if (last.from >= 0 && last.from < 64 && last.to >= 0 && last.to < 64) {
        board = previousBoard;
        moving = GetPieceAtSquare(last.from);
        captured = GetPieceAtSquare(last.to);
        if ((moving == WHITE_PAWN || moving == BLACK_PAWN) &&
            last.to == board.enPassantSquare && last.from % 8 != last.to % 8) {
            int captureSquare = moving == WHITE_PAWN ? last.to + 8 : last.to - 8;
            captured = GetPieceAtSquare(captureSquare);
        }
        board = currentBoard;

        bool gaveCheck = isKingInCheck(currentTurnNow == WHITE_TURN);
        bool castled = (moving == WHITE_KING || moving == BLACK_KING) &&
                       (last.from % 8 - last.to % 8 >= 2 ||
                        last.to % 8 - last.from % 8 >= 2);
        char notation[8], line[DEV_LINE_LENGTH];
        moveName(last, notation);
        snprintf(line, sizeof(line), "MOVE: %s %s %s; captured=%s check=%d castle=%d",
            previousTurn == WHITE_TURN ? "White" : "Black",
            pieceName(moving), notation, pieceName(captured), gaveCheck, castled);
        logLine(line);

        if (previousTurn == (humanPlaysWhite ? WHITE_TURN : BLACK_TURN)) {
            OpponentModel_RecordMove(&opponentProfile, moving, captured, gaveCheck, castled);
            snprintf(line, sizeof(line), "OPPONENT MODEL UPDATE: observations=%u",
                opponentProfile.observations);
            logLine(line);
        }
    }

    previousBoard = currentBoard;
    previousTurn = currentTurnNow;
    previousHash = hash;
    rebuildAnalysis(humanPlaysWhite, true);
    board = currentBoard;
}

void DevMode_DrawOverlay(void) {
    if (!devVisible) return;
    const int x = 16, y = 16, width = 790, height = 610;
    DrawRectangle(x, y, width, height, (Color){13, 16, 22, 238});
    DrawRectangle(x, y, width, 5, (Color){245, 190, 65, 255});
    DrawText("DEVELOPER INTELLIGENCE  /  PRIVATE HUD", x + 18, y + 16, 18,
             (Color){245, 190, 65, 255});
    DrawText("Toggle: type 112358 | log: engine_dev.log | Esc still closes game",
             x + 18, y + 42, 12, (Color){155, 165, 180, 255});
    int lineY = y + 68;
    for (int i = 0; i < devLineCount; ++i) {
        DrawText(devLines[i], x + 18, lineY, 14, (Color){226, 231, 239, 255});
        lineY += 21;
        if (lineY > y + height - 12) break;
    }
}
