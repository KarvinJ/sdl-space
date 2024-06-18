#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 544;
const int FRAME_RATE = 60;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

Mix_Chunk *laserSound = nullptr;

typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int lives;
    int speed;
} Player;

Player player;

typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int points;
    int velocityX;
    bool shouldMove;
    bool isDestroyed;
} MysteryShip;

MysteryShip mysteryShip;

typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int lives;
    bool isDestroyed;
} Structure;

std::vector<Structure> structures;

SDL_Texture *loadSprite(const char *file)
{
    SDL_Texture *texture = IMG_LoadTexture(renderer, file);
    return texture;
}

typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int points;
    int velocity;
    bool isDestroyed;
} Alien;

std::vector<Alien> aliens;

std::vector<Alien> createAliens()
{
    SDL_Texture *alienSprite1 = loadSprite("res/sprites/alien_1.png");
    SDL_Texture *alienSprite2 = loadSprite("res/sprites/alien_2.png");
    SDL_Texture *alienSprite3 = loadSprite("res/sprites/alien_3.png");

    std::vector<Alien> aliens;

    int positionX;
    int positionY = 80;
    int alienPoints = 8;

    SDL_Texture *actualSprite;

    for (int row = 0; row < 5; row++)
    {
        positionX = 150;

        switch (row)
        {

        case 0:
            actualSprite = alienSprite3;
            break;

        case 1:
        case 2:
            actualSprite = alienSprite2;
            break;

        default:
            actualSprite = alienSprite1;
        }

        for (int columns = 0; columns < 11; columns++)
        {
            SDL_Rect alienBounds = {positionX, positionY, 38, 34};

            Alien actualAlien = {alienBounds, actualSprite, alienPoints, 50, false};

            aliens.push_back(actualAlien);
            positionX += 60;
        }

        alienPoints--;
        positionY += 50;
    }

    return aliens;
}

Mix_Chunk *loadSound(const char *p_filePath)
{
    Mix_Chunk *sound = nullptr;

    sound = Mix_LoadWAV(p_filePath);
    if (sound == nullptr)
    {
        printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    }

    return sound;
}

void quitGame()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {

        if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
        {
            quitGame();
            exit(0);
        }

        if (event.key.keysym.sym == SDLK_SPACE)
        {
            Mix_PlayChannel(-1, laserSound, 0);
        }
    }
}

void update(float deltaTime)
{
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);

    if (currentKeyStates[SDL_SCANCODE_A] && player.bounds.x > 0)
    {
        player.bounds.x -= player.speed * deltaTime;
    }

    else if (currentKeyStates[SDL_SCANCODE_D] && player.bounds.x < SCREEN_WIDTH - player.bounds.w)
    {
        player.bounds.x += player.speed * deltaTime;
    }
}

void renderSprite(SDL_Texture *sprite, SDL_Rect spriteBounds)
{
    SDL_QueryTexture(sprite, NULL, NULL, &spriteBounds.w, &spriteBounds.h);
    SDL_RenderCopy(renderer, sprite, NULL, &spriteBounds);
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 29, 29, 27, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    renderSprite(mysteryShip.sprite, mysteryShip.bounds);

    for (Alien alien : aliens)
    {
        renderSprite(alien.sprite, alien.bounds);
    }

    for (Structure structure : structures)
    {
        renderSprite(structure.sprite, structure.bounds);
    }

    renderSprite(player.sprite, player.bounds);

    SDL_RenderPresent(renderer);
}

void capFrameRate(Uint32 frameStartTime)
{
    Uint32 frameTime = SDL_GetTicks() - frameStartTime;

    if (frameTime < 1000 / FRAME_RATE)
    {
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
    if (window == nullptr)
    {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!IMG_Init(IMG_INIT_PNG))
    {
        std::cout << "SDL_image crashed. Error: " << SDL_GetError();
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }

    SDL_Texture *shipSprite = loadSprite("res/sprites/mystery.png");

    SDL_Rect shipBounds = {SCREEN_WIDTH / 2, 30, 58, 25};

    mysteryShip = {shipBounds, shipSprite, 50, -100, false, false};

    aliens = createAliens();

    SDL_Texture *playerSprite = loadSprite("res/sprites/spaceship.png");

    SDL_Rect playerBounds = {SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50, 38, 34};

    player = {playerBounds, playerSprite, 2, 600};

    SDL_Rect structureBounds = {120, SCREEN_HEIGHT - 100, 56, 33};
    SDL_Rect structureBounds2 = {350, SCREEN_HEIGHT - 100, 56, 33};
    SDL_Rect structureBounds3 = {200*3, SCREEN_HEIGHT - 100, 56, 33};
    SDL_Rect structureBounds4 = {200*4, SCREEN_HEIGHT - 100, 56, 33};

    SDL_Texture *structureSprite = loadSprite("res/sprites/structure.png");

    structures.push_back({structureBounds, structureSprite, 5, false});
    structures.push_back({structureBounds2, structureSprite, 5, false});
    structures.push_back({structureBounds3, structureSprite, 5, false});
    structures.push_back({structureBounds4, structureSprite, 5, false});

    laserSound = loadSound("res/sounds/laser.ogg");

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    while (true)
    {
        currentFrameTime = SDL_GetTicks();

        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;

        previousFrameTime = currentFrameTime;

        handleEvents();
        update(deltaTime);
        render();
    }

    return 0;
}