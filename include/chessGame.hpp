#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "SDL_events.h"
#include "chessBase.hpp"
#include "chessBoard.hpp"
#include "chessPiece.hpp"

namespace chess {

class Player {
   public:
    PieceColor color;
    int materialCaptured;
    Piece *selectedPiece;
    std::vector<std::unique_ptr<Piece>> capturedPieces;

   public:
    Player(PieceColor color_);
    virtual std::optional<Move> handleEvents(Board &board, SDL_Event event);
    virtual ~Player() {};
};

class Game {
   protected:
    SDL_Event &_event;
    std::vector<std::string> _positions;

   public:
    bool running;
    std::vector<std::string> moveLogText;
    std::vector<Move> moveLog;
    Board &board;
    Player &player1;
    Player &player2;
    Player *currentPlayer;
    int movesUntilDraw;
    int turnCount;
    int moveCount;

   public:
    Game(Board &board, Player &player1, Player &player2, SDL_Event &event);
    virtual ~Game() {}
    std::optional<RunResult> start();
    /**
     * if a promotion is possible, `promotionFn` will be called to handle user input for the
     * promotion, if no function or lambda is provided, keyboard input will be used,
     * the default is: `Q` = queen, `R` = rook, `N` = knight, `B` = bishop,
     * and, the promoted pawn gets deleted and replaced with a new piece,
     * trying to access the pawn through a pointer after a promotion will probably crash your game
     */
    virtual RunResult run(
        std::optional<std::function<RunResult(Piece *piece)>> promotionFn = std::nullopt);
    void logMove(Move move);
    bool isKingInCheck(PieceColor color);
    bool isMoveLegal(Move &move, const Player &player);
    void undoLastMove();
    Piece *lookForPromotion();
    RunResult defaultPromotionHandler(Piece *piece);
    void reset(std::optional<std::function<void()>> boardResetFn = std::nullopt);
    std::vector<Move> getLegalMoves(Piece &piece, Player &player);
    WinSearchResult lookForWin();
};

}  // namespace chess