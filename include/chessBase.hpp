#pragma once

/* TODO:
        castlong,
        show selected piece on board,
        promotion,
        add arrows
        maybe do some refactoring,
*/
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "SDL_pixels.h"
#include "SDL_render.h"

namespace chess {

template <class Ty>
using OptionalRef = std::optional<std::reference_wrapper<Ty>>;

class Player;
class Board;
class Piece;

enum class WinSearchResult {
    nothing,
    whiteWinCheckmate,
    blackWinCheckmate,
    stalemateDraw,
    repetitionDraw,
    fiftyMoveDraw,
    materialDraw
};

enum class PieceColor { white, black };

enum class MoveType { normal, enPassant, shortCastle, longCastle };

enum class RunResult { invalid, still, turnedPassed, awaitPromotion };

struct BoardColors {
    SDL_Color light;
    SDL_Color dark;
    SDL_Color outline;
};

struct BoardSquare {
    std::string position;
    SDL_Rect rect;
    SDL_Color color;
};

struct Move {
    Piece *startPiece;
    Piece *endPiece;
    std::string start;
    std::string end;
    std::optional<MoveType> type = std::nullopt;
    bool PiecePromoted = false;

    auto operator<=>(const Move &) const = default;
};

struct SDLTextureDeleter {
    void operator()(SDL_Texture *t);
};

struct PieceSprite {
    SDL_Texture *texture;
    SDL_Rect source;
};

struct PieceSpriteSheet {
    int width;
    int height;
    int horizontalFrames;
    int verticalFrames;
    std::unique_ptr<SDL_Texture, SDLTextureDeleter> texture;
};

inline std::map<int, std::function<void(SDL_Renderer *ren)>> drawQueue{};
inline std::map<std::pair<char, PieceColor>, PieceSprite> spriteMap{};
inline PieceSpriteSheet sheet{};

inline constexpr SDL_Color defaultLightBrown = {237, 214, 176, 255};
inline constexpr SDL_Color defaultDarkBrown = {184, 135, 98, 255};
inline constexpr SDL_Color defaultLightBlue = {100, 100, 255, 255};
inline constexpr SDL_Color defaultDarkBlue = {10, 10, 100, 255};

void renderDrawQueue(SDL_Renderer *ren, SDL_Color color);
void createSpriteSheet(std::string path, int width, int height, int horizontalFrames,
                       int verticalFrames, SDL_Renderer *ren);
void setPieceSprite(char type, PieceColor color, int hFrame, int vFrame);
std::string pairToChessPos(std::pair<int, int> p);
std::pair<int, int> chessPosToPair(std::string s);
std::pair<int, int> absDistance(std::string start, std::string end);
std::pair<int, int> normalDistance(std::string start, std::string end);

}  // namespace chess