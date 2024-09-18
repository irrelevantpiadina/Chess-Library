#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "SDL_pixels.h"
#include "SDL_render.h"
#include "chessBase.hpp"

namespace chess {

class Board {
   private:
    SDL_Renderer *_ren;

   public:
    std::map<std::string, std::unique_ptr<Piece>> pieceMap;
    std::map<std::string, BoardSquare> squaresMap;
    int length;
    int numSquares;
    std::pair<int, int> offset;
    bool flipped;
    BoardColors colors;

   public:
    Board(int length, int numSquares, bool flipped, std::pair<int, int> offset, BoardColors colors,
          bool createPieceMap, SDL_Renderer *ren);
    /**
     * creates a map with the default pieces in their default locations, like in regular chess,
     * it can be called in the constructor
     */
    void createDefaultPieceMap();
    void clear();
    void updateSquaresPosition();
    void updateSquaresColor();
    void draw();
    void drawSquareOutline(BoardSquare square, SDL_Color color);
    void renderPieces();
    void flip();
    void keepCentered(int areaWidth, int areaHeight);
    /**
     * attempts to make a move, whether it's legal or not,
     * if `move.endPiece` isn't nullptr the piece it points to will be captured and deleted,
     * returns `true` if the move was made, otherwise `false`
     */
    bool makeMove(Move move);
    /**
     * attempts to make a move, whether it's legal or not,
     * if `move.endPiece` isn't nullptr the piece it points to will be captured
     * and ownership will be transferred to a pointer in `capturedPieces`,
     * returns `true` if the move was made, otherwise `false`
     */
    bool makeMove(Move move, std::vector<std::unique_ptr<Piece>> &capturedPieces);
    OptionalRef<BoardSquare> getSquareUnderCursor();
    void highlightSquareUnderCursor(int increment);
};

}  // namespace chess