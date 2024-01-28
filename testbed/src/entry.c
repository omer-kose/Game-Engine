#include <entry.h>

#include "game.h"

// TODO: Remove this
#include <platform/platform.h>

// Define the function to create a game.
b8 create_game(game* out_game)
{
    // Application configuration.
    out_game->app_config.start_pos_x = 100;
    out_game->app_config.start_pos_y = 100;
    out_game->app_config.start_width = 1280;
    out_game->app_config.start_height = 720;
    out_game->app_config.name = "KoEngine Testbed";

    // Assign function pointers of the game.
    out_game->update = game_update;
    out_game->render = game_render;
    out_game->initialize = game_initialize;
    out_game->on_resize = game_on_resize;

    // Create the game state. (Note that for memory allocation we always use platform-specific allocation functions we have defined)
    out_game->state = platform_allocate(sizeof(game_state), FALSE);

    return TRUE;
}