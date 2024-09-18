#include <iostream>
#include <string>

#include "SDL.h"
#include "SDL_events.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include "chessBase.hpp"
#include "chessBoard.hpp"
#include "chessGame.hpp"

using namespace chess;

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Event event{};
    SDL_Window *window = SDL_CreateWindow("Chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1280, 720, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    Board board = {
        720,  64,      true, {0, 0}, {defaultLightBrown, defaultDarkBrown, {0, 0, 0, 100}},
        true, renderer};
    Player player1 = {PieceColor::white};
    Player player2 = {PieceColor::black};
    Game game = {board, player1, player2, event};

    createSpriteSheet("../example/pieces.png", 2560, 854, 6, 2, renderer);
    setPieceSprite('p', PieceColor::white, 5, 0);
    setPieceSprite('p', PieceColor::black, 5, 1);
    setPieceSprite('r', PieceColor::white, 4, 0);
    setPieceSprite('r', PieceColor::black, 4, 1);
    setPieceSprite('n', PieceColor::white, 3, 0);
    setPieceSprite('n', PieceColor::black, 3, 1);
    setPieceSprite('b', PieceColor::white, 2, 0);
    setPieceSprite('b', PieceColor::black, 2, 1);
    setPieceSprite('q', PieceColor::white, 1, 0);
    setPieceSprite('q', PieceColor::black, 1, 1);
    setPieceSprite('k', PieceColor::white, 0, 0);
    setPieceSprite('k', PieceColor::black, 0, 1);

    int w{};
    int h{};

    while (true) {
        SDL_GetWindowSize(window, &w, &h);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);

        board.keepCentered(w, h);
        board.draw();
        board.highlightSquareUnderCursor(50);
        board.renderPieces();

        renderDrawQueue(renderer, {0, 0, 0, 255});

        while (SDL_PollEvent(&event) != 0) {
            RunResult r1 = game.run();
            if (r1 == RunResult::invalid) {
                std::cout << "game is invalid somehow\n";
            }
            switch (event.type) {
                case SDL_QUIT:
                    goto end;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_f:
                            board.flip();
                            break;
                        case SDLK_v:
                            game.reset();
                            break;
                        case SDLK_u:
                            if (r1 != RunResult::awaitPromotion) {
                                game.undoLastMove();
                            }
                            break;
                        case SDLK_s:
                            game.start();
                            std::cout << "game started\n";
                            break;
                        case SDLK_p:
                            game.running = false;
                            std::cout << "game paused\n";
                            break;
                        case SDLK_e:
                            game.running = true;
                            std::cout << "game resumed\n";
                            break;
                    }
                    break;
            }

            WinSearchResult r2 = game.lookForWin();

            if (r2 == WinSearchResult::repetitionDraw) {
                game.reset();
                std::cout << "repetition draw\n";
            } else if (r2 == WinSearchResult::blackWinCheckmate) {
                game.reset();
                std::cout << "black won by checkmate\n";
            } else if (r2 == WinSearchResult::whiteWinCheckmate) {
                game.reset();
                std::cout << "white won by checkmate\n";
            } else if (r2 == WinSearchResult::materialDraw) {
                game.reset();
                std::cout << "insufficient material\n";
            }
        }

        SDL_RenderPresent(renderer);
    }

end:
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}