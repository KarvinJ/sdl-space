#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 544;

int startSDL(SDL_Window *window, SDL_Renderer *renderer);