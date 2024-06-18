#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>

const int SPEED = 600;
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 544;
const int FRAME_RATE = 60; 

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

Mix_Chunk *test = nullptr;
SDL_Texture* sprite = nullptr;

SDL_Rect spriteBounds = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 38, 34};

SDL_Texture* loadSprite(const char* file, SDL_Renderer* renderer) {

    SDL_Texture* texture = IMG_LoadTexture(renderer, file);
    return texture;
}

void renderSprite(SDL_Texture* sprite, SDL_Renderer* renderer, SDL_Rect spriteBounds) {

    SDL_QueryTexture(sprite, NULL, NULL, &spriteBounds.w, &spriteBounds.h);
    SDL_RenderCopy(renderer, sprite, NULL, &spriteBounds);
}

Mix_Chunk* loadSound(const char *p_filePath)
{
    Mix_Chunk *sound = nullptr;

    sound = Mix_LoadWAV(p_filePath);
    if (sound == nullptr)
    {
        printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    }

    return sound;
}

void quitGame() {

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void handleEvents() {

    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) {
            
            quitGame();
            exit(0);
        }

         if (event.key.keysym.sym == SDLK_SPACE) {
             Mix_PlayChannel(-1, test, 0);
         }
    }
}

void update(float deltaTime) {

    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

    if (currentKeyStates[SDL_SCANCODE_W] && spriteBounds.y > 0) {
        spriteBounds.y -= SPEED * deltaTime;
    }

    else if (currentKeyStates[SDL_SCANCODE_S] && spriteBounds.y < SCREEN_HEIGHT - spriteBounds.h) {
        spriteBounds.y += SPEED * deltaTime;
    }

    else if (currentKeyStates[SDL_SCANCODE_A] && spriteBounds.x > 0) {
        spriteBounds.x -= SPEED * deltaTime;
    }

    else if (currentKeyStates[SDL_SCANCODE_D] && spriteBounds.x < SCREEN_WIDTH - spriteBounds.w) {
        spriteBounds.x += SPEED * deltaTime;
    }
}

void render() {
 
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    renderSprite(sprite, renderer, spriteBounds);

    SDL_RenderPresent(renderer);
}

void capFrameRate(Uint32 frameStartTime) {

    Uint32 frameTime = SDL_GetTicks() - frameStartTime;
    
    if (frameTime < 1000 / FRAME_RATE) {
        SDL_Delay(1000 / FRAME_RATE - frameTime);
    }
}

int main(int argc, char *args[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cout << "SDL crashed. Error: " << SDL_GetError();
    }

    window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!IMG_Init(IMG_INIT_PNG))
    {
        std::cout << "SDL_image crashed. Error: " << SDL_GetError();
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }

    sprite = loadSprite("res/sprites/alien_1.png", renderer);

    test = loadSound("res/sounds/magic.wav");

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    while (true) {

        currentFrameTime = SDL_GetTicks();

        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f; 

        previousFrameTime = currentFrameTime;

        handleEvents();
        update(deltaTime);
        render();
    }

    return 0;
}