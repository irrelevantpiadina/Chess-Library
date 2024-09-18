#include "chessBoard.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "SDL_mouse.h"
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "chessBase.hpp"
#include "chessPiece.hpp"

int intSqrt(int n) { return static_cast<int>(std::floor(std::sqrt(n))); }

namespace chess {

Board::Board(int length, int numSquares, bool flipped, std::pair<int, int> offset,
             BoardColors colors, bool createPieceMap, SDL_Renderer *ren)
    : _ren(ren),
      length(length),
      numSquares(numSquares),
      offset(offset),
      flipped(flipped),
      colors(colors) {
    for (int x = 1; x <= intSqrt(numSquares); x++) {
        for (int y = 1; y <= intSqrt(numSquares); y++) {
            pieceMap[pairToChessPos({x, y})] = nullptr;
            squaresMap[pairToChessPos({x, y})] = {};
        }
    }

    updateSquaresPosition();
    updateSquaresColor();

    if (createPieceMap) {
        createDefaultPieceMap();
    }
}

void Board::createDefaultPieceMap() {
    using enum PieceColor;

    for (int x = 1; x <= intSqrt(numSquares); x++) {
        pieceMap[pairToChessPos({x, 2})] =
            std::make_unique<Pawn>(pairToChessPos({x, 2}), 1, 'p', white);
        pieceMap[pairToChessPos({x, 7})] =
            std::make_unique<Pawn>(pairToChessPos({x, 7}), 1, 'p', black);
    }
    pieceMap["a1"] = std::make_unique<Rook>("a1", 5, 'r', white);
    pieceMap["h1"] = std::make_unique<Rook>("h1", 5, 'r', white);
    pieceMap["a8"] = std::make_unique<Rook>("a8", 5, 'r', black);
    pieceMap["h8"] = std::make_unique<Rook>("h8", 5, 'r', black);
    pieceMap["b1"] = std::make_unique<Knight>("b1", 3, 'n', white);
    pieceMap["g1"] = std::make_unique<Knight>("g1", 3, 'n', white);
    pieceMap["b8"] = std::make_unique<Knight>("b8", 3, 'n', black);
    pieceMap["g8"] = std::make_unique<Knight>("g8", 3, 'n', black);
    pieceMap["c1"] = std::make_unique<Bishop>("c1", 3, 'b', white);
    pieceMap["f1"] = std::make_unique<Bishop>("f1", 3, 'b', white);
    pieceMap["c8"] = std::make_unique<Bishop>("c8", 3, 'b', black);
    pieceMap["f8"] = std::make_unique<Bishop>("f8", 3, 'b', black);
    pieceMap["d1"] = std::make_unique<Queen>("d1", 9, 'q', white);
    pieceMap["d8"] = std::make_unique<Queen>("d8", 9, 'q', black);
    pieceMap["e1"] = std::make_unique<King>("e1", 0, 'k', white);
    pieceMap["e8"] = std::make_unique<King>("e8", 0, 'k', black);
}

void Board::clear() {
    for (auto &[pos, piece] : pieceMap) {
        piece = nullptr;
    }
}

void Board::updateSquaresColor() {
    for (auto &[pos, square] : squaresMap) {
        std::pair<int, int> pair = chessPosToPair(pos);
        auto &[x, y] = pair;
        square.color = (x + y) % 2 == 0 ? colors.dark : colors.light;
    }
}

void Board::updateSquaresPosition() {
    for (auto &[pos, square] : squaresMap) {
        std::pair<int, int> pair = chessPosToPair(pos);
        auto &[x, y] = pair;
        auto &[xOffset, yOffset] = offset;
        int lengthOfSquare = length / intSqrt(numSquares);
        square.rect = {
            (std::abs(x - (!flipped * (intSqrt(numSquares) + 1))) - 1) * lengthOfSquare + xOffset,
            (std::abs(y - (flipped * (intSqrt(numSquares) + 1))) - 1) * lengthOfSquare + yOffset,
            lengthOfSquare, lengthOfSquare};
        square.position = pos;
    }
}

void Board::draw() {
    for (auto &[pos, square] : squaresMap) {
        SDL_SetRenderDrawColor(_ren, square.color.r, square.color.g, square.color.b,
                               square.color.a);
        SDL_RenderFillRect(_ren, &square.rect);
        SDL_SetRenderDrawColor(_ren, colors.outline.r, colors.outline.g, colors.outline.b,
                               colors.outline.a);
        SDL_RenderDrawRect(_ren, &square.rect);
    }
}

void Board::drawSquareOutline(BoardSquare square, SDL_Color color) {
    SDL_SetRenderDrawColor(_ren, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(_ren, &square.rect);
}

void Board::renderPieces() {
    for (auto &[pos, piece] : pieceMap) {
        if (piece != nullptr &&
            spriteMap.find({piece->notation, piece->color}) != spriteMap.end()) {
            PieceSprite sprite = spriteMap.at({piece->notation, piece->color});
            auto &[xOffset, yOffset] = offset;
            std::pair<int, int> pair = chessPosToPair(pos);
            auto &[x, y] = pair;
            int lengthOfSquare = length / intSqrt(numSquares);
            if (piece->_dstOverride) {
                piece->dst = {
                    (std::abs(x - (!flipped * (intSqrt(numSquares) + 1))) - 1) * lengthOfSquare +
                        xOffset,
                    (std::abs(y - (flipped * (intSqrt(numSquares) + 1))) - 1) * lengthOfSquare +
                        yOffset,
                    lengthOfSquare, lengthOfSquare};
            }
            SDL_RenderCopy(_ren, sprite.texture, &sprite.source, &piece->dst);
        }
    }
}

void Board::flip() {
    flipped = !flipped;
    updateSquaresPosition();
}

void Board::keepCentered(int areaWidth, int areaHeight) {
    std::pair<int, int> prevOffset = offset;
    int prevLength = length;

    offset.first = areaWidth / 2 - length / 2;
    length = areaHeight;

    if (prevLength != length || prevOffset != offset) {
        updateSquaresPosition();
    }
}

bool Board::makeMove(Move move) {
    bool ret = false;

    if (move.startPiece == pieceMap.at(move.start).get() &&
        (move.endPiece == pieceMap.at(move.end).get() || move.type == MoveType::enPassant) &&
        move.startPiece != nullptr && move.startPiece != move.endPiece &&
        pieceMap.find(move.start) != pieceMap.end() && pieceMap.find(move.end) != pieceMap.end()) {
        if (move.type == MoveType::enPassant) {
            int dir = move.startPiece->color == PieceColor::white ? -1 : +1;
            std::string capturedPawnPos = {move.end[0], static_cast<char>(move.end[1] + dir)};
            if (pieceMap.at(capturedPawnPos) != nullptr) {
                pieceMap.at(capturedPawnPos).reset();
            }
        } else if (move.type == MoveType::shortCastle || move.type == MoveType::longCastle) {
            std::string pos1 = {
                static_cast<char>(move.start[0] + (move.type == MoveType::shortCastle ? +1 : -1)),
                move.start[1]};
            std::string pos2 = {
                static_cast<char>(move.start[0] + (move.type == MoveType::shortCastle ? +3 : -4)),
                move.start[1]};

            pieceMap.at(pos1).swap(pieceMap.at(pos2));
            pieceMap.at(pos1)->position = pos1;
        }
        if (move.endPiece != nullptr) {
            pieceMap.at(move.endPiece->position).reset();
        }

        pieceMap.at(move.start).swap(pieceMap.at(move.end));
        pieceMap.at(move.end)->position = move.end;

        move.endPiece = nullptr;
        ret = true;
    }

    return ret;
}

bool Board::makeMove(Move move, std::vector<std::unique_ptr<Piece>> &capturedPieces) {
    bool ret = false;

    if (move.startPiece == pieceMap.at(move.start).get() &&
        (move.endPiece == pieceMap.at(move.end).get() || move.type == MoveType::enPassant) &&
        move.startPiece != nullptr && move.startPiece != move.endPiece &&
        pieceMap.find(move.start) != pieceMap.end() && pieceMap.find(move.end) != pieceMap.end()) {
        if (move.type == MoveType::enPassant) {
            int dir = move.startPiece->color == PieceColor::white ? -1 : +1;
            std::string capturedPawnPos = {move.end[0], static_cast<char>(move.end[1] + dir)};
            if (pieceMap.at(capturedPawnPos) != nullptr) {
                capturedPieces.push_back(nullptr);
                pieceMap.at(capturedPawnPos).swap(capturedPieces.back());
            }
        } else if (move.type == MoveType::shortCastle || move.type == MoveType::longCastle) {
            std::string pos1 = {
                static_cast<char>(move.start[0] + (move.type == MoveType::shortCastle ? +1 : -1)),
                move.start[1]};
            std::string pos2 = {
                static_cast<char>(move.start[0] + (move.type == MoveType::shortCastle ? +3 : -4)),
                move.start[1]};

            pieceMap.at(pos1).swap(pieceMap.at(pos2));
            pieceMap.at(pos1)->position = pos1;
        } else if (move.endPiece != nullptr) {
            capturedPieces.push_back(nullptr);
            pieceMap.at(move.endPiece->position).swap(capturedPieces.back());
        }

        pieceMap.at(move.start).swap(pieceMap.at(move.end));
        pieceMap.at(move.end)->position = move.end;

        ret = true;
    }

    return ret;
}

OptionalRef<BoardSquare> Board::getSquareUnderCursor() {
    OptionalRef<BoardSquare> ret = std::nullopt;

    SDL_Point mousePos{};
    SDL_GetMouseState(&mousePos.x, &mousePos.y);

    auto it = std::find_if(squaresMap.begin(), squaresMap.end(), [=](auto &a) -> bool {
        return SDL_PointInRect(&mousePos, &a.second.rect);
    });

    if (it != squaresMap.end()) {
        ret = it->second;
    } else {
        ret = std::nullopt;
    }

    return ret;
}

void Board::highlightSquareUnderCursor(int increment) {
    OptionalRef<BoardSquare> s = getSquareUnderCursor();
    updateSquaresColor();
    if (s.has_value()) {
        s->get().color.r += s->get().color.r + increment > 255 ? 255 - s->get().color.r : increment;
        s->get().color.g += s->get().color.g + increment > 255 ? 255 - s->get().color.g : increment;
        s->get().color.b += s->get().color.b + increment > 255 ? 255 - s->get().color.b : increment;
    }
}

}  // namespace chess