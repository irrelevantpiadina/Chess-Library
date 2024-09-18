#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>

#include "SDL_rect.h"
#include "chessBase.hpp"

namespace chess {

class Piece {
   private:
    bool _dstOverride;

   private:
    friend class Player;
    friend class Board;

   public:
    std::string position;
    const int value;
    const char notation;
    const PieceColor color;
    SDL_Rect dst;
    int moveCount;

   public:
    Piece(std::string position, int value, char notation, PieceColor color);
    virtual std::optional<MoveType> canMove(
        const std::map<std::string, std::unique_ptr<Piece>> &pieceMap, std::string where,
        std::optional<Move> lastMove = std::nullopt) = 0;
    /**
     * get the positions the piece can "see"
     */
    std::vector<std::string> getVision(const Board &board,
                                       std::optional<Move> lastMove = std::nullopt);
    virtual ~Piece() {};
};

class Pawn : public Piece {
   public:
    using Piece::Piece;
    std::optional<MoveType> canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
                                    std::string where,
                                    std::optional<Move> lastMove = std::nullopt) override;
    ~Pawn() override {};
};

class Rook : public Piece {
   public:
    using Piece::Piece;
    std::optional<MoveType> canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
                                    std::string where,
                                    std::optional<Move> lastMove = std::nullopt) override;
    ~Rook() override {};
};

class Knight : public Piece {
   public:
    using Piece::Piece;
    std::optional<MoveType> canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
                                    std::string where,
                                    std::optional<Move> lastMove = std::nullopt) override;
    ~Knight() override {};
};

class Bishop : public Piece {
   public:
    using Piece::Piece;
    std::optional<MoveType> canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
                                    std::string where,
                                    std::optional<Move> lastMove = std::nullopt) override;
    ~Bishop() override {};
};

class Queen : public Piece {
   public:
    using Piece::Piece;
    std::optional<MoveType> canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
                                    std::string where,
                                    std::optional<Move> lastMove = std::nullopt) override;
    ~Queen() override {};
};

class King : public Piece {
   public:
    using Piece::Piece;
    std::optional<MoveType> canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
                                    std::string where,
                                    std::optional<Move> lastMove = std::nullopt) override;
    ~King() override {};
};

}  // namespace chess