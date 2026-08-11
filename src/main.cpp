// main.cpp
#include "game.h"
#include "state.h"

int main(int argc, char* argv[]) {
    Game game;
    if (!game.Init("SDL3 Game Template")) {
        return 1;
    }

    // Pass the game's context to the first state
    game.PushState(new TitleState(game.GetContext()));

    game.Run();
    return 0;
}