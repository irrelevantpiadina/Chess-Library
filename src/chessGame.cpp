#include "chessGame.hpp"

#include <math.h>

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_mouse.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "chessBase.hpp"
#include "chessBoard.hpp"
#include "chessPiece.hpp"

extern int intSqrt(int n);

namespace chess {

static constexpr int rectIdx = 0;

Player::Player(PieceColor color) : color(color), materialCaptured(0), selectedPiece(nullptr) {}

Game::Game(Board &board, Player &player1, Player &player2, SDL_Event &event)
    : _event(event),
      running(false),
      board(board),
      player1(player1),
      player2(player2),
      currentPlayer(player1.color == PieceColor::white ? &player1 : &player2),
      movesUntilDraw(50),
      turnCount(0),
      moveCount(0) {}

std::optional<RunResult> Game::start() {
    std::optional<RunResult> ret =
        player1.color == player2.color ? std::make_optional(RunResult::invalid) : std::nullopt;

    running = !ret.has_value();

    return ret;
}

std::optional<Move> Player::handleEvents(Board &board, SDL_Event event) {
    std::optional<Move> ret = std::nullopt;
    static std::optional<SDL_Rect> rect = std::nullopt;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        std::optional<BoardSquare> startSquare = board.getSquareUnderCursor();

        if (startSquare.has_value()) {
            rect = startSquare->rect;
            if (selectedPiece == board.pieceMap.at(startSquare->position).get()) {
                selectedPiece = nullptr;
            } else if (board.pieceMap.at(startSquare->position).get() != nullptr) {
                if (selectedPiece != nullptr) {
                    ret = {selectedPiece, board.pieceMap.at(startSquare->position).get(),
                           selectedPiece->position, startSquare->position};
                } else {
                    selectedPiece = board.pieceMap.at(startSquare->position).get();
                    selectedPiece->_dstOverride = false;
                }
            }
        }
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        std::optional<BoardSquare> endSquare = board.getSquareUnderCursor();
        rect = std::nullopt;

        if (selectedPiece != nullptr) {
            selectedPiece->_dstOverride = true;
        }

        if (!endSquare.has_value()) {
            selectedPiece = nullptr;
        } else if (selectedPiece != nullptr &&
                   selectedPiece != board.pieceMap.at(endSquare->position).get()) {
            ret = {selectedPiece, board.pieceMap.at(endSquare->position).get(),
                   selectedPiece->position, endSquare->position};
            selectedPiece = nullptr;
        }
    }

    if (selectedPiece != nullptr) {
        int mouseX{};
        int mouseY{};

        SDL_GetMouseState(&mouseX, &mouseY);

        int lengthOfSquare = board.length / intSqrt(board.numSquares);
        selectedPiece->dst = {mouseX - lengthOfSquare / 2, mouseY - lengthOfSquare / 2,
                              lengthOfSquare, lengthOfSquare};

        if (rect.has_value()) {
            drawQueue[rectIdx] = [=](SDL_Renderer *ren) -> void {
                SDL_RenderDrawRect(ren, &*rect);
            };
        }
    } else {
        drawQueue.erase(rectIdx);
    }

    return ret;
}

void Game::logMove(Move move) {
    moveCount++;
    turnCount += currentPlayer->color == PieceColor::white ? 1 : 0;
    moveLog.push_back(move);

    if (move.type != MoveType::normal && move.type != MoveType::enPassant) {
        moveLogText.push_back(move.type == MoveType::longCastle ? "O-O-O" : "O-O");
    } else {
        moveLogText.push_back(
            std::to_string(moveCount) + ". " +
            (move.startPiece->notation == 'p' ? '\0' : move.startPiece->notation) +
            (move.endPiece != nullptr ? "x" : "") + move.end);
    }

    std::string str{};
    for (auto &[pos, piece] : board.pieceMap) {
        str += piece == nullptr ? "" : piece->notation + pos;
    }
    _positions.push_back(str);
}

void Game::undoLastMove() {
    if (!moveLog.empty()) {
        Move lastMove = moveLog.back();

        if (lastMove.PiecePromoted) {
            board.pieceMap.at(lastMove.end) =
                std::make_unique<Pawn>(lastMove.end, 1, 'p', lastMove.startPiece->color);
        }

        board.pieceMap.at(lastMove.end).swap(board.pieceMap.at(lastMove.start));
        board.pieceMap.at(lastMove.start)->position = lastMove.start;
        currentPlayer = lastMove.startPiece->color == player1.color ? &player1 : &player2;
        lastMove.startPiece->moveCount--;

        if (lastMove.type == MoveType::enPassant) {
            int dir = lastMove.startPiece->color == PieceColor::white ? -1 : +1;
            std::string capturedPawnPos = {lastMove.end[0],
                                           static_cast<char>(lastMove.end[1] + dir)};

            currentPlayer->capturedPieces.back().swap(board.pieceMap.at(capturedPawnPos));
            currentPlayer->capturedPieces.pop_back();
            currentPlayer->materialCaptured -= lastMove.endPiece->value;
        } else if (lastMove.type == MoveType::shortCastle ||
                   lastMove.type == MoveType::longCastle) {
            std::string pos1 = {
                static_cast<char>(lastMove.start[0] +
                                  (lastMove.type == MoveType::shortCastle ? +1 : -1)),
                lastMove.start[1]};
            std::string pos2 = {
                static_cast<char>(lastMove.start[0] +
                                  (lastMove.type == MoveType::shortCastle ? 3 : -4)),
                lastMove.start[1]};

            board.pieceMap.at(pos1).swap(board.pieceMap.at(pos2));
            board.pieceMap.at(pos2)->position = pos2;
        } else if (lastMove.endPiece != nullptr) {
            currentPlayer->capturedPieces.back().swap(board.pieceMap.at(lastMove.end));
            currentPlayer->capturedPieces.pop_back();
            currentPlayer->materialCaptured -= lastMove.endPiece->value;
        }

        moveCount--;
        turnCount -= lastMove.startPiece->color == PieceColor::black ? 1 : 0;
        moveLog.pop_back();
        moveLogText.pop_back();
        _positions.pop_back();
    }
}

bool Game::isKingInCheck(PieceColor color) {
    bool ret = false;
    std::string kingPos{};

    auto it = std::find_if(board.pieceMap.begin(), board.pieceMap.end(), [=](auto &a) -> bool {
        return a.second != nullptr && a.second->notation == 'k' && a.second->color == color;
    });

    kingPos = it != board.pieceMap.end() ? it->second->position : "";

    it = std::find_if(board.pieceMap.begin(), board.pieceMap.end(), [=, this](auto &a) -> bool {
        return a.second != nullptr &&
               a.second->canMove(board.pieceMap, kingPos) == MoveType::normal;
    });

    ret = it != board.pieceMap.end();

    return ret;
}

bool Game::isMoveLegal(Move &move, const Player &player) {
    bool ret = false;
    std::optional<MoveType> type = move.startPiece->canMove(
        board.pieceMap, move.end,
        moveLog.empty() ? std::nullopt : std::make_optional(moveLog.back()));

    if (move.startPiece != nullptr && player.color == move.startPiece->color && type.has_value()) {
        move.type = type;
        board.makeMove(move, currentPlayer->capturedPieces);

        ret = !isKingInCheck(currentPlayer->color);

        board.pieceMap.at(move.end).swap(board.pieceMap.at(move.start));
        board.pieceMap.at(move.start)->position = move.start;

        if (move.type == MoveType::enPassant) {
            int dir = move.startPiece->color == PieceColor::white ? -1 : +1;
            std::string capturedPawnPos = {move.end[0], static_cast<char>(move.end[1] + dir)};

            currentPlayer->capturedPieces.back().swap(board.pieceMap.at(capturedPawnPos));
            currentPlayer->capturedPieces.pop_back();
        } else if (move.type == MoveType::shortCastle || move.type == MoveType::longCastle) {
            std::string pos1 = {
                static_cast<char>(move.start[0] + (move.type == MoveType::shortCastle ? +1 : -1)),
                move.start[1]};
            std::string pos2 = {
                static_cast<char>(move.start[0] + (move.type == MoveType::shortCastle ? 3 : -4)),
                move.start[1]};

            board.pieceMap.at(pos1).swap(board.pieceMap.at(pos2));
            board.pieceMap.at(pos2)->position = pos2;
        } else if (move.endPiece != nullptr) {
            currentPlayer->capturedPieces.back().swap(board.pieceMap.at(move.end));
            currentPlayer->capturedPieces.pop_back();
        }
    }

    return ret;
}

std::vector<Move> Game::getLegalMoves(Piece &piece, Player &player) {
    std::vector<Move> ret{};

    for (auto &[pos, square] : board.squaresMap) {
        Move move = {&piece, board.pieceMap.at(pos).get(), piece.position, pos};
        if (isMoveLegal(move, player)) {
            ret.push_back(move);
        }
    }

    return ret;
}

WinSearchResult Game::lookForWin() {
    WinSearchResult ret = WinSearchResult::nothing;
    int numWhiteLegalMoves{};
    int numBlackLegalMoves{};

    numWhiteLegalMoves = std::accumulate(
        board.pieceMap.begin(), board.pieceMap.end(), 0, [this](int val, auto &a) -> int {
            Player &p = player1.color == PieceColor::white ? player1 : player2;
            return a.second != nullptr && a.second->color == PieceColor::white
                       ? val + getLegalMoves(*a.second, p).size()
                       : val;
        });

    numBlackLegalMoves = std::accumulate(
        board.pieceMap.begin(), board.pieceMap.end(), 0, [this](int val, auto &a) -> int {
            Player &p = player1.color == PieceColor::black ? player1 : player2;
            return a.second != nullptr && a.second->color == PieceColor::black
                       ? val + getLegalMoves(*a.second, p).size()
                       : val;
        });

    if (numWhiteLegalMoves == 0) {
        if (isKingInCheck(PieceColor::white)) {
            ret = WinSearchResult::blackWinCheckmate;
            moveLogText.back() += "#";
        } else {
            ret = WinSearchResult::stalemateDraw;
        }
    } else if (numBlackLegalMoves == 0) {
        if (isKingInCheck(PieceColor::black)) {
            ret = WinSearchResult::whiteWinCheckmate;
            moveLogText.back() += "#";
        } else {
            ret = WinSearchResult::stalemateDraw;
        }
    }

    if (ret == WinSearchResult::nothing) {
        ret = movesUntilDraw == 0 ? WinSearchResult::fiftyMoveDraw : WinSearchResult::nothing;
    }

    if (ret == WinSearchResult::nothing) {
        auto getMaterial = [this](Player &p) -> int {
            return std::accumulate(
                board.pieceMap.begin(), board.pieceMap.end(), 0, [&](int val, auto &a) -> int {
                    return a.second != nullptr && a.second->color == p.color ? val + a.second->value
                                                                             : val;
                });
        };
        auto hasPawn = [this](Player &p) -> bool {
            return std::find_if(board.pieceMap.begin(), board.pieceMap.end(), [&](auto &a) -> bool {
                       return a.second != nullptr && a.second->notation == 'p' &&
                              a.second->color == p.color;
                   }) == board.pieceMap.end();
        };

        ret = getMaterial(player1) < 5 && hasPawn(player1) && getMaterial(player2) < 5 &&
                      hasPawn(player2)
                  ? WinSearchResult::materialDraw
                  : WinSearchResult::nothing;
    }

    if (ret == WinSearchResult::nothing && _positions.size() > 2) {
        std::map<std::string, int> map{};
        for (std::string &pos : _positions) {
            map[pos] = map.contains(pos) ? map[pos] + 1 : 1;

            if (map[pos] == 3) {
                ret = WinSearchResult::repetitionDraw;
                break;
            }
        }
    }

    return ret;
}

Piece *Game::lookForPromotion() {
    Piece *ret = nullptr;

    auto it = std::find_if(board.pieceMap.begin(), board.pieceMap.end(), [](auto &a) -> bool {
        return a.second != nullptr && a.second->notation == 'p' &&
               a.second->position[1] == (a.second->color == PieceColor::white ? '8' : '1');
    });

    ret = it != board.pieceMap.end() ? it->second.get() : nullptr;

    return ret;
}

RunResult Game::defaultPromotionHandler(Piece *piece) {
    RunResult ret = RunResult::still;

    Pawn pawn = *reinterpret_cast<Pawn *>(piece);
    switch (_event.type) {
        case SDL_KEYUP:
            switch (_event.key.keysym.sym) {
                case SDLK_q:
                    board.pieceMap.at(pawn.position) =
                        std::make_unique<Queen>(pawn.position, 9, 'q', pawn.color);
                    moveLog.back().startPiece = board.pieceMap.at(pawn.position).get();
                    moveLog.back().PiecePromoted = true;
                    moveLogText.back() += "=q";
                    break;
                case SDLK_r:
                    board.pieceMap.at(pawn.position) =
                        std::make_unique<Rook>(pawn.position, 5, 'r', pawn.color);
                    moveLog.back().startPiece = board.pieceMap.at(pawn.position).get();
                    moveLog.back().PiecePromoted = true;
                    moveLogText.back() += "=r";
                    break;
                case SDLK_n:
                    board.pieceMap.at(pawn.position) =
                        std::make_unique<Knight>(pawn.position, 3, 'n', pawn.color);
                    moveLog.back().startPiece = board.pieceMap.at(pawn.position).get();
                    moveLog.back().PiecePromoted = true;
                    moveLogText.back() += "=n";
                    break;
                case SDLK_b:
                    board.pieceMap.at(pawn.position) =
                        std::make_unique<Bishop>(pawn.position, 3, 'b', pawn.color);
                    moveLog.back().startPiece = board.pieceMap.at(pawn.position).get();
                    moveLog.back().PiecePromoted = true;
                    moveLogText.back() += "=b";
                    break;
            }
            break;
        default:
            ret = RunResult::awaitPromotion;
            break;
    }

    return ret;
}

RunResult Game::run(std::optional<std::function<RunResult(Piece *piece)>> promotionFn) {
    RunResult ret = RunResult::still;

    if (running) {
        std::optional<Move> move = currentPlayer->handleEvents(board, _event);

        Piece *piece = lookForPromotion();
        if (piece != nullptr) {
            if (promotionFn.has_value()) {
                ret = (*promotionFn)(piece);
            } else {
                ret = defaultPromotionHandler(piece);
            }
        }

        if (ret == RunResult::still && move.has_value() && isMoveLegal(*move, *currentPlayer) &&
            board.makeMove(*move, currentPlayer->capturedPieces)) {
            logMove(*move);

            currentPlayer->materialCaptured += move->endPiece != nullptr
                                                   ? currentPlayer->capturedPieces.back()->value
                                               : move->type == MoveType::enPassant ? 1
                                                                                   : 0;

            move->startPiece->moveCount++;
            currentPlayer = (currentPlayer == &player1) ? &player2 : &player1;
            movesUntilDraw = move->startPiece->notation == 'p' || move->endPiece != nullptr
                                 ? 50
                                 : movesUntilDraw - 1;

            ret = RunResult::turnedPassed;
        }
    }

    return ret;
}

void Game::reset(std::optional<std::function<void()>> boardResetFn) {
    board.clear();
    if (boardResetFn.has_value()) {
        (*boardResetFn)();
    } else {
        board.createDefaultPieceMap();
    }

    moveLog.clear();
    moveLogText.clear();
    player1.capturedPieces.clear();
    player2.capturedPieces.clear();
    _positions.clear();

    running = false;
    movesUntilDraw = 50;
    moveCount = 0;
    turnCount = 0;
    player1.materialCaptured = 0;
    player2.materialCaptured = 0;
    player1.selectedPiece = nullptr;
    player2.selectedPiece = nullptr;
    currentPlayer = player1.color == PieceColor::white ? &player1 : &player2;
}

}  // namespace chess