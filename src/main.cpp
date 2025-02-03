#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include "sdl_starter.h"
#include "sdl_assets_loader.h"

bool isGamePaused;
bool isGameOver;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

Mix_Chunk *laserSound = nullptr;
Mix_Chunk *explosionSound = nullptr;
Mix_Chunk *pauseSound = nullptr;
Mix_Music *music = nullptr;

TTF_Font *fontSquare = nullptr;

SDL_Texture *scoreTexture = nullptr;
SDL_Rect scoreBounds;

SDL_Texture *liveTexture = nullptr;
SDL_Rect liveBounds;

SDL_Texture *pauseTexture = nullptr;
SDL_Rect pauseBounds;

Sprite shipSprite;
Sprite playerSprite;
Sprite alienSprite1;
Sprite alienSprite2;
Sprite alienSprite3;
Sprite structureSprite;

typedef struct
{
    SDL_Rect bounds;
    bool isDestroyed;
} Laser;

std::vector<Laser> playerLasers;
std::vector<Laser> alienLasers;

float lastTimePlayerShoot;
float lastTimeAliensShoot;

typedef struct
{
    Sprite sprite;
    int lives;
    int speed;
    int score;
} Player;

Player player;

typedef struct
{
    float x;
    Sprite sprite;
    int points;
    int velocityX;
    bool shouldMove;
    bool isDestroyed;
} MysteryShip;

MysteryShip mysteryShip;

float lastTimeMysteryShipSpawn;

typedef struct
{
    Sprite sprite;
    int lives;
    bool isDestroyed;
} Structure;

std::vector<Structure> structures;

Sprite loadSprite(const char *file, int positionX, int positionY)
{
    SDL_Rect textureBounds = {positionX, positionY, 0, 0};

    SDL_Texture *texture = IMG_LoadTexture(renderer, file);

    if (texture != nullptr)
    {
        SDL_QueryTexture(texture, NULL, NULL, &textureBounds.w, &textureBounds.h);
    }

    Sprite sprite = {texture, textureBounds};

    return sprite;
}

typedef struct
{
    float x;
    Sprite sprite;
    int points;
    int velocity;
    bool isDestroyed;
} Alien;

std::vector<Alien> aliens;

bool shouldChangeVelocity = false;

std::vector<Alien> createAliens()
{
    alienSprite1 = loadSprite("res/sprites/alien_1.png", 0, 0);
    alienSprite2 = loadSprite("res/sprites/alien_2.png", 0, 0);
    alienSprite3 = loadSprite("res/sprites/alien_3.png", 0, 0);

    std::vector<Alien> aliens;

    // we should use .reserve when creating a vector to avoid requiring allocation, reserve does: Increase the capacity
    //  of the vector (the total number of elements that the vector can hold without requiring reallocation
    // we increase the vector capacity to 5 * 11 = 55 aliens struct.
    aliens.reserve(55);

    int positionX;
    int positionY = 50;
    int alienPoints = 8;

    Sprite actualSprite;

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
            actualSprite.textureBounds.x = positionX;
            actualSprite.textureBounds.y = positionY;

            Alien actualAlien = {(float)positionX, actualSprite, alienPoints, 100, false};

            aliens.push_back(actualAlien);
            positionX += 60;
        }

        alienPoints--;
        positionY += 50;
    }

    return aliens;
}

void aliensMovement(float deltaTime)
{
    for (Alien &alien : aliens)
    {
        alien.x += alien.velocity * deltaTime;
        alien.sprite.textureBounds.x = alien.x;

        float alienPosition = alien.sprite.textureBounds.x + alien.sprite.textureBounds.w;

        if ((!shouldChangeVelocity && alienPosition > SCREEN_WIDTH) || alienPosition < alien.sprite.textureBounds.w)
        {
            shouldChangeVelocity = true;
            break;
        }
    }

    if (shouldChangeVelocity)
    {
        for (Alien &alien : aliens)
        {
            alien.velocity *= -1;
            alien.sprite.textureBounds.y += 10;
        }

        shouldChangeVelocity = false;
    }
}

void quitGame()
{
    SDL_DestroyTexture(shipSprite.texture);
    SDL_DestroyTexture(playerSprite.texture);
    SDL_DestroyTexture(structureSprite.texture);
    SDL_DestroyTexture(alienSprite1.texture);
    SDL_DestroyTexture(alienSprite2.texture);
    SDL_DestroyTexture(alienSprite3.texture);
    SDL_DestroyTexture(scoreTexture);
    SDL_DestroyTexture(liveTexture);
    SDL_DestroyTexture(pauseTexture);

    // Close SDL_image
    IMG_Quit();

    Mix_FreeChunk(laserSound);
    Mix_FreeChunk(explosionSound);
    Mix_FreeChunk(pauseSound);
    Mix_FreeMusic(music);

    // Close SDL_mixer
    Mix_CloseAudio();
    Mix_Quit();

    // Close SDL_ttf
    TTF_Quit();

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

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_f)
        {
            isGamePaused = !isGamePaused;
            Mix_PlayChannel(-1, pauseSound, 0);
        }
    }
}

void checkCollisionBetweenStructureAndLaser(Laser &laser)
{
    for (Structure &structure : structures)
    {
        if (!structure.isDestroyed && SDL_HasIntersection(&structure.sprite.textureBounds, &laser.bounds))
        {
            laser.isDestroyed = true;

            structure.lives--;

            if (structure.lives == 0)
            {
                structure.isDestroyed = true;
            }

            Mix_PlayChannel(-1, explosionSound, 0);

            break;
        }
    }
}

void removeDestroyedElements()
{
    for (auto iterator = aliens.begin(); iterator != aliens.end();)
    {
        if (iterator->isDestroyed)
        {
            aliens.erase(iterator);
        }
        else
        {
            iterator++;
        }
    }

    for (auto iterator = playerLasers.begin(); iterator != playerLasers.end();)
    {
        if (iterator->isDestroyed)
        {
            playerLasers.erase(iterator);
        }
        else
        {
            iterator++;
        }
    }

    for (auto iterator = alienLasers.begin(); iterator != alienLasers.end();)
    {
        if (iterator->isDestroyed)
        {
            alienLasers.erase(iterator);
        }
        else
        {
            iterator++;
        }
    }
}

void update(float deltaTime)
{
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);

    if (currentKeyStates[SDL_SCANCODE_A] && player.sprite.textureBounds.x > 0)
    {
        player.sprite.textureBounds.x -= player.speed * deltaTime;
    }

    else if (currentKeyStates[SDL_SCANCODE_D] && player.sprite.textureBounds.x < SCREEN_WIDTH - player.sprite.textureBounds.w)
    {
        player.sprite.textureBounds.x += player.speed * deltaTime;
    }

    if (!mysteryShip.shouldMove)
    {
        lastTimeMysteryShipSpawn += deltaTime;

        if (lastTimeMysteryShipSpawn >= 10)
        {
            lastTimeMysteryShipSpawn = 0;

            mysteryShip.shouldMove = true;
        }
    }

    if (mysteryShip.shouldMove)
    {
        if (mysteryShip.sprite.textureBounds.x > SCREEN_WIDTH + mysteryShip.sprite.textureBounds.w || mysteryShip.sprite.textureBounds.x < -80)
        {
            mysteryShip.velocityX *= -1;
            mysteryShip.shouldMove = false;
        }

        mysteryShip.x += mysteryShip.velocityX * deltaTime;
        mysteryShip.sprite.textureBounds.x = mysteryShip.x;
    }

    if (currentKeyStates[SDL_SCANCODE_SPACE])
    {
        lastTimePlayerShoot += deltaTime;

        if (lastTimePlayerShoot >= 0.35)
        {
            SDL_Rect laserBounds = {player.sprite.textureBounds.x + 20, player.sprite.textureBounds.y - player.sprite.textureBounds.h, 4, 16};

            playerLasers.push_back({laserBounds, false});

            lastTimePlayerShoot = 0;

            Mix_PlayChannel(-1, laserSound, 0);
        }
    }

    for (Laser &laser : playerLasers)
    {
        laser.bounds.y -= 400 * deltaTime;

        if (laser.bounds.y < 0)
            laser.isDestroyed = true;

        if (!mysteryShip.isDestroyed && SDL_HasIntersection(&mysteryShip.sprite.textureBounds, &laser.bounds))
        {
            laser.isDestroyed = true;

            player.score += mysteryShip.points;

            std::string scoreString = "score: " + std::to_string(player.score);

            updateTextureText(scoreTexture, scoreString.c_str(), fontSquare, renderer);

            mysteryShip.isDestroyed = true;

            Mix_PlayChannel(-1, explosionSound, 0);

            break;
        }

        for (Alien &alien : aliens)
        {
            if (!alien.isDestroyed && SDL_HasIntersection(&alien.sprite.textureBounds, &laser.bounds))
            {
                alien.isDestroyed = true;
                laser.isDestroyed = true;

                player.score += alien.points;

                std::string scoreString = "score: " + std::to_string(player.score);

                updateTextureText(scoreTexture, scoreString.c_str(), fontSquare, renderer);

                Mix_PlayChannel(-1, explosionSound, 0);

                break;
            }
        }

        checkCollisionBetweenStructureAndLaser(laser);
    }

    lastTimeAliensShoot += deltaTime;

    if (aliens.size() > 0 && lastTimeAliensShoot >= 0.6)
    {
        int randomAlienIndex = rand() % aliens.size();

        Alien alienShooter = aliens[randomAlienIndex];

        SDL_Rect laserBounds = {alienShooter.sprite.textureBounds.x + 20, alienShooter.sprite.textureBounds.y + alienShooter.sprite.textureBounds.h, 4, 16};

        alienLasers.push_back({laserBounds, false});

        lastTimeAliensShoot = 0;

        Mix_PlayChannel(-1, laserSound, 0);
    }

    for (Laser &laser : alienLasers)
    {
        laser.bounds.y += 400 * deltaTime;

        if (laser.bounds.y > SCREEN_HEIGHT)
            laser.isDestroyed = true;

        if (player.lives > 0 && SDL_HasIntersection(&player.sprite.textureBounds, &laser.bounds))
        {
            laser.isDestroyed = true;

            player.lives--;

            std::string liveString = "lives: " + std::to_string(player.lives);

            updateTextureText(liveTexture, liveString.c_str(), fontSquare, renderer);

            Mix_PlayChannel(-1, explosionSound, 0);

            break;
        }

        checkCollisionBetweenStructureAndLaser(laser);
    }

    aliensMovement(deltaTime);

    removeDestroyedElements();
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 29, 29, 27, 255);
    SDL_RenderClear(renderer);

    SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreBounds.w, &scoreBounds.h);
    scoreBounds.x = 200;
    scoreBounds.y = scoreBounds.h / 2;
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreBounds);

    SDL_QueryTexture(liveTexture, NULL, NULL, &liveBounds.w, &liveBounds.h);
    liveBounds.x = 600;
    liveBounds.y = liveBounds.h / 2;
    SDL_RenderCopy(renderer, liveTexture, NULL, &liveBounds);

    if (!mysteryShip.isDestroyed)
    {
        renderSprite(renderer, mysteryShip.sprite);
    }

    for (Alien &alien : aliens)
    {
        if (!alien.isDestroyed)
        {
            renderSprite(renderer, alien.sprite);
        }
    }

    SDL_SetRenderDrawColor(renderer, 243, 216, 63, 255);

    SDL_RenderDrawLine(renderer, 0, 1, SCREEN_WIDTH, 1);
    SDL_RenderDrawLine(renderer, 0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, SCREEN_HEIGHT - 1);
    SDL_RenderDrawLine(renderer, 0, 0, 0, SCREEN_HEIGHT);
    SDL_RenderDrawLine(renderer, SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT);

    for (Laser &laser : alienLasers)
    {
        if (!laser.isDestroyed)
        {
            SDL_RenderFillRect(renderer, &laser.bounds);
        }
    }

    for (Laser &laser : playerLasers)
    {
        if (!laser.isDestroyed)
        {
            SDL_RenderFillRect(renderer, &laser.bounds);
        }
    }

    for (Structure &structure : structures)
    {
        if (!structure.isDestroyed)
        {
            renderSprite(renderer, structure.sprite);
        }
    }

    renderSprite(renderer, player.sprite);

    if (isGamePaused)
    {
        SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseBounds);
    }

    SDL_RenderPresent(renderer);
}

void setupStructures()
{
    SDL_Rect structureBounds = {120, SCREEN_HEIGHT - 120, 56, 33};
    SDL_Rect structureBounds2 = {350, SCREEN_HEIGHT - 120, 56, 33};
    SDL_Rect structureBounds3 = {200 * 3, SCREEN_HEIGHT - 120, 56, 33};
    SDL_Rect structureBounds4 = {200 * 4, SCREEN_HEIGHT - 120, 56, 33};

    structureSprite = loadSprite("res/sprites/structure.png", 120, SCREEN_HEIGHT - 120);

    structures.push_back({{structureSprite.texture, structureBounds}, 5, false});
    structures.push_back({{structureSprite.texture, structureBounds2}, 5, false});
    structures.push_back({{structureSprite.texture, structureBounds3}, 5, false});
    structures.push_back({{structureSprite.texture, structureBounds4}, 5, false});
}

void resetGame()
{
    structures.clear();
    setupStructures();

    aliens.clear();
    aliens = createAliens();
}

int main(int argc, char *args[])
{
    window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (startSDL(window, renderer) > 0)
    {
        return 1;
    }

    fontSquare = TTF_OpenFont("res/fonts/square_sans_serif_7.ttf", 30);

    updateTextureText(scoreTexture, "Score: 0", fontSquare, renderer);

    updateTextureText(liveTexture, "Lives: 2", fontSquare, renderer);

    updateTextureText(pauseTexture, "Game Paused", fontSquare, renderer);

    SDL_QueryTexture(pauseTexture, NULL, NULL, &pauseBounds.w, &pauseBounds.h);
    pauseBounds.x = 350;
    pauseBounds.y = pauseBounds.h / 2 + 25;

    laserSound = loadSound("res/sounds/laser.wav");
    pauseSound = loadSound("res/sounds/magic.wav");
    explosionSound = loadSound("res/sounds/explosion.wav");

    Mix_VolumeChunk(explosionSound, MIX_MAX_VOLUME / 2);

    music = loadMusic("res/music/music.wav");

    // Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    // Mix_PlayMusic(music, -1);

    shipSprite = loadSprite("res/sprites/mystery.png", SCREEN_WIDTH, 40);

    mysteryShip = {SCREEN_WIDTH, shipSprite, 50, -200, false, false};

    aliens = createAliens();

    playerSprite = loadSprite("res/sprites/spaceship.png", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 40);

    player = {playerSprite, 2, 600, 0};

    setupStructures();

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    // Activating random seed
    srand(time(NULL));

    while (true)
    {
        currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;
        previousFrameTime = currentFrameTime;

        handleEvents();

        // this is failling when the player dies.
        if (aliens.size() == 0 || player.lives == 0)
        {
            resetGame();
        }

        if (!isGamePaused)
        {
            update(deltaTime);
        }

        render();
    }

    quitGame();
}