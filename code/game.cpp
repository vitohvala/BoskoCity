#include "game.h"

UPDATE_FUNC(game_update)
{
    if(!m->is_init) {
        set_console(&m->log);
        m->is_init = true;
    }
    //Fatal("GameCode test");
}