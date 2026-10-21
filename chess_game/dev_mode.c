#include "dev_mode.h"
#include "dev_window.h"

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
static SearchRootAnalysis rootAnalysis;
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

static void rebuildAnalysis(bool humanWhite, bool writeLog) {
    devLineCount = 0;
    bool engineWhite = !humanWhite;
    ThreatReport threats;
    OpponentIntent intents[8];
    StrategicGoal goals[8];

    ThreatAnalysis_Analyze(&threats);
    int intentCount = OpponentModel_InferIntentions(
        &threats, engineWhite, &opponentProfile, intents, 8);
    int goalCount = GoalManager_Update(
        engineWhite, &threats, intents, intentCount, goals, 8);
    GetSearchRootAnalysis(&rootAnalysis);

    addHudLine("CHOSEN MOVE");
    if (rootAnalysis.candidateCount == 0 ||
        rootAnalysis.selectedMove.from < 0) {
        addHudLine("  no completed root search / no legal move");
    } else {
        char selected[8];
        moveName(rootAnalysis.selectedMove, selected);
        addHudLine("  %s  score %+d (%s POV) depth %d  selected=%s",
            selected, rootAnalysis.selectedScore,
            rootAnalysis.sideToMove == WHITE_TURN ? "White" : "Black",
            rootAnalysis.completedDepth,
            rootAnalysis.selectedScore == rootAnalysis.highestScore ?
                "highest" : "chaos tie-break");
    }
    addHudLine("WHY THIS MOVE");
    if (rootAnalysis.candidateCount > 0) {
        const SearchRootCandidate *selected = NULL;
        for (int i = 0; i < rootAnalysis.candidateCount; ++i) {
            if (rootAnalysis.candidates[i].selected) {
                selected = &rootAnalysis.candidates[i];
                break;
            }
        }
        if (selected) {
            int materialDelta = selected->materialEval -
                rootAnalysis.rootMaterialEval;
            int positionalDelta = selected->positionalEval -
                rootAnalysis.rootPositionalEval;
            if (materialDelta != 0)
                addHudLine("  material eval %+d (measured, White POV)", materialDelta);
            if (positionalDelta != 0)
                addHudLine("  positional eval %+d (measured, White POV)", positionalDelta);
            if (selected->captured != EMPTY)
                addHudLine("  captures %s (board move fact)", pieceName(selected->captured));
            if (selected->givesCheck)
                addHudLine("  gives check (board move fact)");
            if (selected->isPromotion)
                addHudLine("  promotes (board move fact)");
            if (materialDelta == 0 && positionalDelta == 0 &&
                selected->captured == EMPTY && !selected->givesCheck &&
                !selected->isPromotion)
                addHudLine("  no distinct measured factor; root score was decisive");
            if (selected->score != rootAnalysis.highestScore)
                addHudLine("  selected within %d cp; chaos tie-break applied",
                    CHAOS_SCORE_WINDOW);
        }
    } else {
        addHudLine("  search data is incomplete");
    }
    addHudLine("ALTERNATIVES (score from side-to-move POV)");
    int alternatives = rootAnalysis.candidateCount < DEV_MAX_CANDIDATES ?
        rootAnalysis.candidateCount : DEV_MAX_CANDIDATES;
    for (int i = 0; i < alternatives; ++i) {
        char notation[8];
        moveName(rootAnalysis.candidates[i].move, notation);
        addHudLine("  #%d %s %+d %s%s", i + 1, notation,
            rootAnalysis.candidates[i].score,
            rootAnalysis.candidates[i].selected ? "[SELECTED]" :
                rootAnalysis.candidates[i].score > rootAnalysis.selectedScore ?
                "higher score, not selected" : "lower searched score",
            rootAnalysis.candidates[i].pv[0] ? " pv available" : "");
    }
    if (alternatives == 0) addHudLine("  no legal root candidates");
    addHudLine("SEARCH CONTEXT");
    addHudLine("  depth %d | nodes %llu | root candidates %d",
        rootAnalysis.completedDepth, rootAnalysis.nodes,
        rootAnalysis.candidateCount);

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
    DevWindow_SetText("DEVELOPER INTELLIGENCE / PRIVATE HUD", devLines, devLineCount);
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
    for (int i = 0; i < alternatives; ++i) {
        char notation[8];
        moveName(rootAnalysis.candidates[i].move, notation);
        snprintf(line, sizeof(line),
            "ROOT_CANDIDATE #%d: %s score=%d depth=%d bound=%s selected=%d capture=%s check=%d promotion=%d pv=%s",
            i + 1, notation, rootAnalysis.candidates[i].score,
            rootAnalysis.candidates[i].depth,
            rootAnalysis.candidates[i].bound == 0 ? "exact" :
                rootAnalysis.candidates[i].bound == 1 ? "lower" : "upper",
            rootAnalysis.candidates[i].selected,
            pieceName(rootAnalysis.candidates[i].captured),
            rootAnalysis.candidates[i].givesCheck,
            rootAnalysis.candidates[i].isPromotion,
            rootAnalysis.candidates[i].pv[0] ? rootAnalysis.candidates[i].pv : "-");
        logLine(line);
    }
    snprintf(line, sizeof(line),
        "ROOT_EXPLANATION: selected=%s score=%d highest=%d depth=%d nodes=%llu",
        rootAnalysis.selectedMove.from >= 0 ? "move-present" : "none",
        rootAnalysis.selectedScore, rootAnalysis.highestScore,
        rootAnalysis.completedDepth, rootAnalysis.nodes);
    logLine(line);
    if (rootAnalysis.selectedMove.from >= 0) {
        char selected[8];
        moveName(rootAnalysis.selectedMove, selected);
        snprintf(line, sizeof(line),
            "SELECTED_MOVE: %s score=%d perspective=%s",
            selected, rootAnalysis.selectedScore,
            rootAnalysis.sideToMove == WHITE_TURN ? "white" : "black");
        logLine(line);
        for (int i = 0; i < rootAnalysis.candidateCount; ++i) {
            const SearchRootCandidate *candidate = &rootAnalysis.candidates[i];
            if (!candidate->selected) continue;
            snprintf(line, sizeof(line),
                "EXPLANATION_FACTOR: materialDelta=%d positionalDelta=%d source=evaluation_components",
                candidate->materialEval - rootAnalysis.rootMaterialEval,
                candidate->positionalEval - rootAnalysis.rootPositionalEval);
            logLine(line);
            snprintf(line, sizeof(line),
                "EXPLANATION_FACTOR: capture=%d check=%d promotion=%d source=legal_board_state",
                candidate->captured != EMPTY, candidate->givesCheck,
                candidate->isPromotion);
            logLine(line);
            break;
        }
    }
    snprintf(line, sizeof(line), "SEARCH: state=%s nodes=%llu staticEvalWhitePOV=%d",
        IsComputerThinking() ? "running" : "idle",
        GetSearchNodeCount(), evaluate());
    logLine(line);
}

void DevMode_Init(void) {
    OpponentModel_Init(&opponentProfile);
    DevWindow_Init();
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
        case 0: return IsKeyPressed(KEY_ZERO) || IsKeyPressed(KEY_KP_0);
        case 3: return IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3);
        case 4: return IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4);
        case 5: return IsKeyPressed(KEY_FIVE) || IsKeyPressed(KEY_KP_5);
        case 6: return IsKeyPressed(KEY_SIX) || IsKeyPressed(KEY_KP_6);
        case 7: return IsKeyPressed(KEY_SEVEN) || IsKeyPressed(KEY_KP_7);
        case 8: return IsKeyPressed(KEY_EIGHT) || IsKeyPressed(KEY_KP_8);
        case 9: return IsKeyPressed(KEY_NINE) || IsKeyPressed(KEY_KP_9);
        default: return false;
    }
}

void DevMode_HandleHotkey(void) {
    static const int sequence[] = {1, 1, 2, 3, 5, 8};
    int pressed = 0;
    const int digits[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 10; ++i) {
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
        devVisible = !DevWindow_IsVisible();
        DevWindow_Show(devVisible);
        keySequenceIndex = 0;
        logLine(devVisible ? "DEV HUD ENABLED by 112358 hotkey" :
                              "DEV HUD DISABLED by 112358 hotkey");
        if (devVisible) rebuildAnalysis(!IsBoardFlipped(), true);
    }
}

void DevMode_Update(bool humanPlaysWhite) {
    DevWindow_Poll();
    /* Human promotion is a two-step UI action; wait until the piece is chosen. */
    if (promotionMenu.active) return;
    uint64_t hash = generateHash();
    if (!snapshotReady) {
        previousBoard = board;
        previousTurn = currentTurn;
        previousHash = hash;
        snapshotReady = true;
        return;
    }
    if (hash == previousHash) {
        if (devVisible && devLineCount > 0) {
            snprintf(devLines[devLineCount - 1], DEV_LINE_LENGTH,
                "Search: %s | nodes %llu | static eval (White POV) %+d",
                IsComputerThinking() ? "running" : "idle",
                GetSearchNodeCount(), evaluate());
        }
        return;
    }

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
        } else {
            snprintf(line, sizeof(line),
                "ENGINE DECISION: move=%s postMoveStaticEvalWhitePOV=%d nodes=%llu; search=negamax/alpha-beta; tie-break may prefer chaos within %d cp",
                notation, evaluate(), GetSearchNodeCount(), CHAOS_SCORE_WINDOW);
            logLine(line);
        }
    }

    previousBoard = currentBoard;
    previousTurn = currentTurnNow;
    previousHash = hash;
    rebuildAnalysis(humanPlaysWhite, true);
    board = currentBoard;
}

void DevMode_DrawOverlay(void) { /* HUD is rendered in its own native window. */ }

void DevMode_Shutdown(void) {
    DevWindow_Shutdown();
    if (decisionLog) {
        logLine("=== DEVELOPER DIAGNOSTICS SESSION ENDED ===");
        fclose(decisionLog);
        decisionLog = NULL;
    }
}
