#include "chessPiece.hpp"

#include <memory>
#include <optional>
#include <utility>

#include "chessBase.hpp"
#include "chessBoard.hpp"

namespace chess {

Piece::Piece(std::string position, int value, char notation, PieceColor color)
    : _dstOverride(true), position(position), value(value), notation(notation),
      color(color), moveCount(0) {}

std::vector<std::string> Piece::getVision(const Board &board,
                                          std::optional<Move> lastMove) {
  std::vector<std::string> ret;
  for (auto &[pos, square] : board.squaresMap) {
    std::optional<MoveType> type = canMove(board.pieceMap, pos, lastMove);
    if (type == MoveType::normal || type == MoveType::enPassant) {
      ret.push_back(pos);
    }
  }

  return ret;
}

std::optional<MoveType>
Pawn::canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
              std::string where, std::optional<Move> lastMove) {
  std::optional<MoveType> ret = std::nullopt;

  if (pieceMap.contains(where)) {
    std::pair<int, int> absDist = absDistance(position, where);

    auto isAhead = [this](std::string pos1, std::string pos2) {
      std::pair<int, int> dist = relativeDistance(pos2, pos1);
      return color == PieceColor::white ? dist.second < 0 : dist.second > 0;
    };

    if (isAhead(where, position)) {
      if (absDist == std::pair(0, 1) && pieceMap.at(where) == nullptr) {
        ret = MoveType::normal;
      } else if (absDist == std::pair(1, 1) && pieceMap.at(where) != nullptr &&
                 pieceMap.at(where)->color != color) {
        ret = MoveType::normal;
      } else if (absDist == std::pair(0, 2) && moveCount == 0 &&
                 pieceMap.at(where) == nullptr) {
        for (int i = 1; i <= 2; i++) {
          std::pair<int, int> dir =
              color == PieceColor::white ? std::pair(0, 1) : std::pair(0, -1);
          std::pair<int, int> p = chessPosToPair(position);
          std::string p2 = pairToChessPos({
              p.first + dir.first * i,
              p.second + dir.second * i,
          });

          if (p2 == where) {
            ret = MoveType::normal;
          } else if (pieceMap.at(p2) != nullptr) {
            ret = std::nullopt;
            break;
          }
        }
      } else if (absDist == std::pair(1, 1) &&
                 lastMove->startPiece->notation == 'p' &&
                 lastMove->startPiece->color != color &&
                 lastMove->end[0] == where[0] &&
                 absDistance(position, lastMove->end) == std::pair(1, 0) &&
                 !isAhead(lastMove->end, where) &&
                 absDistance(lastMove->start, lastMove->end).second == 2) {
        ret = MoveType::enPassant;
      }
    }
  }

  return ret;
}

std::optional<MoveType>
Rook::canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
              std::string where, std::optional<Move> lastMove) {
  std::optional<MoveType> ret = std::nullopt;

  if (pieceMap.contains(where)) {
    std::pair<int, int> absDist = absDistance(position, where);

    if (pieceMap.at(where) != nullptr && pieceMap.at(where)->color == color) {
      ret = std::nullopt;
    } else if (absDist.first == 0 || absDist.second == 0) {
      std::pair<int, int> dist = relativeDistance(position, where);
      std::pair<int, int> dir = {
          dist.first / (dist.first == 0 ? 1 : absDist.first) * -1,
          dist.second / (dist.second == 0 ? 1 : absDist.second) * -1};

      for (int i = 1; i <= std::max(absDist.first, absDist.second); i++) {
        std::pair<int, int> p = chessPosToPair(position);
        std::string np = pairToChessPos({
            p.first + dir.first * i,
            p.second + dir.second * i,
        });

        if (np == where) {
          ret = MoveType::normal;
        } else if (pieceMap.at(np) != nullptr) {
          ret = std::nullopt;
          break;
        }
      }
    }
  }

  return ret;
}

std::optional<MoveType>
Knight::canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
                std::string where, std::optional<Move> lastMove) {
  std::optional<MoveType> ret = std::nullopt;

  if (pieceMap.contains(where)) {
    std::pair<int, int> absDist = absDistance(position, where);

    if (pieceMap.at(where) != nullptr && pieceMap.at(where)->color == color) {
      ret = std::nullopt;
    } else if (absDist == std::pair(2, 1) || absDist == std::pair(1, 2)) {
      ret = MoveType::normal;
    }
  }

  return ret;
}

std::optional<MoveType>
Bishop::canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
                std::string where, std::optional<Move> lastMove) {
  std::optional<MoveType> ret = std::nullopt;

  if (pieceMap.contains(where)) {
    std::pair<int, int> absDist = absDistance(position, where);

    if (pieceMap.at(where) != nullptr && pieceMap.at(where)->color == color) {
      ret = std::nullopt;
    } else if (absDist.first == absDist.second) {
      std::pair<int, int> dist = relativeDistance(position, where);
      std::pair<int, int> dir = {dist.first / absDist.first * -1,
                                 dist.second / absDist.second * -1};

      for (int i = 1; i <= absDist.first; i++) {
        std::pair<int, int> p = chessPosToPair(position);
        std::string np = pairToChessPos({
            p.first + dir.first * i,
            p.second + dir.second * i,
        });

        if (np == where) {
          ret = MoveType::normal;
        } else if (pieceMap.at(np) != nullptr) {
          ret = std::nullopt;
          break;
        }
      }
    }
  }

  return ret;
}

std::optional<MoveType>
Queen::canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
               std::string where, std::optional<Move> lastMove) {
  std::optional<MoveType> ret = std::nullopt;

  if (pieceMap.contains(where)) {
    std::pair<int, int> absDist = absDistance(position, where);
    std::pair<int, int> dist = relativeDistance(position, where);
    std::pair<int, int> dir = {0, 0};

    if (pieceMap.at(where) != nullptr && pieceMap.at(where)->color == color) {
      ret = std::nullopt;
    } else if (absDist.first == absDist.second) {
      dir = {dist.first / absDist.first * -1,
             dist.second / absDist.second * -1};
    } else if (absDist.first == 0 || absDist.second == 0) {
      dir = {dist.first / (dist.first == 0 ? 1 : absDist.first) * -1,
             dist.second / (dist.second == 0 ? 1 : absDist.second) * -1};
    }

    if (dir != std::pair(0, 0)) {
      for (int i = 1; i <= std::max(absDist.first, absDist.second); i++) {
        std::pair<int, int> p = chessPosToPair(position);
        std::string np = pairToChessPos({
            p.first + dir.first * i,
            p.second + dir.second * i,
        });

        if (np == where) {
          ret = MoveType::normal;
        } else if (pieceMap.at(np) != nullptr) {
          ret = std::nullopt;
          break;
        }
      }
    }
  }

  return ret;
}

std::optional<MoveType>
King::canMove(const std::map<std::string, std::unique_ptr<Piece>> &pieceMap,
              std::string where, std::optional<Move> lastMove) {
  std::optional<MoveType> ret = std::nullopt;

  if (pieceMap.contains(where)) {
    std::pair<int, int> absDist = absDistance(position, where);
    std::pair<int, int> dist = relativeDistance(position, where);

    if (pieceMap.at(where) != nullptr && pieceMap.at(where)->color == color) {
      ret = std::nullopt;
    } else if (absDist.first <= 1 && absDist.second <= 1) {
      ret = MoveType::normal;
    } else if (absDist == std::pair(2, 0) && moveCount == 0) {
      if (dist.first > 0) {
        std::string pos1 = {static_cast<char>(position[0] - 1), position[1]};
        std::string pos2 = {static_cast<char>(position[0] - 2), position[1]};
        std::string pos3 = {static_cast<char>(position[0] - 3), position[1]};
        std::string pos4 = {static_cast<char>(position[0] - 4), position[1]};

        Piece *piece1 = pieceMap.at(pos1).get();
        Piece *piece2 = pieceMap.at(pos1).get();
        Piece *piece3 = pieceMap.at(pos3).get();
        Piece *piece4 = pieceMap.at(pos4).get();

        for (auto &[pos, piece] : pieceMap) {
          if (piece == nullptr) {
            continue;
          }

          if (piece4 != nullptr && piece4->notation == 'r' &&
              piece4->color == color && piece1 == nullptr &&
              piece2 == nullptr && piece3 == nullptr && piece->color != color &&
              !piece->canMove(pieceMap, pos1).has_value() &&
              !piece->canMove(pieceMap, pos2).has_value() &&
              !piece->canMove(pieceMap, pos3).has_value()) {
            ret = MoveType::longCastle;
            break;
          }
        }
      } else if (dist.first < 0) {
        std::string pos1 = {static_cast<char>(position[0] + 1), position[1]};
        std::string pos2 = {static_cast<char>(position[0] + 2), position[1]};
        std::string pos3 = {static_cast<char>(position[0] + 3), position[1]};

        Piece *piece1 = pieceMap.at(pos1).get();
        Piece *piece2 = pieceMap.at(pos1).get();
        Piece *piece3 = pieceMap.at(pos3).get();

        for (auto &[pos, piece] : pieceMap) {
          if (piece == nullptr) {
            continue;
          }

          if (piece3 != nullptr && piece3->notation == 'r' &&
              piece3->color == color && piece1 == nullptr &&
              piece2 == nullptr && piece->color != color &&
              !piece->canMove(pieceMap, pos1).has_value() &&
              !piece->canMove(pieceMap, pos2).has_value()) {
            ret = MoveType::shortCastle;
            break;
          }
        }
      }
    }
  }

  return ret;
}

} // namespace chess