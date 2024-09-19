#include "chessBase.hpp"

#include <cmath>
#include <memory>
#include <string>
#include <utility>

#include "SDL_image.h"
#include "SDL_pixels.h"
#include "SDL_render.h"

namespace chess {

void SDLTextureDeleter::operator()(SDL_Texture *t) {
    if (t != nullptr) {
        SDL_DestroyTexture(t);
    }
}

std::pair<int, int> chessPosToPair(std::string s) {
    return s.length() >= 2 ? std::pair(s[0] - 96, s[1] - 48) : std::pair(-1, -1);
}

std::string pairToChessPos(std::pair<int, int> p) {
    return {static_cast<char>(p.first + 96), static_cast<char>(p.second + 48)};
}

void createSpriteSheet(std::string path, int width, int height, int horizontalFrames,
                       int verticalFrames, SDL_Renderer *ren) {
    sheet.width = width;
    sheet.height = height;
    sheet.horizontalFrames = horizontalFrames;
    sheet.verticalFrames = verticalFrames;
    sheet.texture.reset(IMG_LoadTexture(ren, path.c_str()));
}

void setPieceSprite(char type, PieceColor color, int hFrame, int vFrame) {
    spriteMap[{type, color}] = {
        sheet.texture.get(),
        {hFrame * (sheet.width / sheet.horizontalFrames),
         vFrame * (sheet.height / sheet.verticalFrames), sheet.width / sheet.horizontalFrames,
         sheet.height / sheet.verticalFrames}};
}

std::pair<int, int> absDistance(std::string start, std::string end) {
    return {std::abs(chessPosToPair(start).first - chessPosToPair(end).first),
            std::abs(chessPosToPair(start).second - chessPosToPair(end).second)};
}

std::pair<int, int> relativeDistance(std::string start, std::string end) {
    return {chessPosToPair(start).first - chessPosToPair(end).first,
            chessPosToPair(start).second - chessPosToPair(end).second};
}

void renderDrawQueue(SDL_Renderer *ren, SDL_Color color) {
    for (auto &[idx, fn] : drawQueue) {
        SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
        fn(ren);
    }
}

}  // namespace chess