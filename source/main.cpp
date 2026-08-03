#include "Game.h"
#include <eng.h>


int main()
{
    Game* game = new Game();
    Eng::CreatorEngine engine;
    engine.SetApplication(game);

    if(engine.Init())
    {
        engine.Run();
    }

    engine.Destroy();

    return 0;
}