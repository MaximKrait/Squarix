#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PLAYER_SIZE = 20;
const int PLAYER_SPEED = 5;
const int INITIAL_LIVES = 3;
const Uint32 FRAME_DELAY = 16; // For 60 FPS
const int ENEMY_SIZE = 20;
const int ENEMY_SPEED = 2;
const int NUM_ENEMIES = 5;
const int SCORE_INCREMENT_INTERVAL = 1000; // 1 second
const int IMMUNITY_DURATION = 3000; // 3 seconds

// Global variables
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
TTF_Font* gFont = nullptr;

// Structure for enemies
struct Enemy {
    int x, y;
    bool active;
};

// Function to initialize SDL and TTF
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

// Function to close the application
void close() {
    if (gFont != nullptr) {
        TTF_CloseFont(gFont);
        gFont = nullptr;
    }

    if (gRenderer != nullptr) {
        SDL_DestroyRenderer(gRenderer);
        gRenderer = nullptr;
    }

    if (gWindow != nullptr) {
        SDL_DestroyWindow(gWindow);
        gWindow = nullptr;
    }

    TTF_Quit();
    SDL_Quit();
}

// Function to render text
void renderText(const std::string& message, int x, int y, SDL_Color color) {
    if (gFont == nullptr) {
        std::cerr << "Font not loaded!" << std::endl;
        return;
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, message.c_str(), color);
    if (textSurface != nullptr) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (textTexture != nullptr) {
            SDL_Rect renderQuad = { x, y, textSurface->w, textSurface->h };
            SDL_RenderCopy(gRenderer, textTexture, nullptr, &renderQuad);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    } else {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }
}

// Function to display the main menu
void showMenu(int& screenWidth, int& screenHeight, bool& fullscreen) {
    bool menuOpen = true;
    SDL_Event e;
    SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};

    while (menuOpen) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                menuOpen = false;
                return;
            }

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    menuOpen = false;
                    return;
                }
                if (e.key.keysym.sym == SDLK_1) {
                    screenWidth = 800;
                    screenHeight = 600;
                }
                if (e.key.keysym.sym == SDLK_2) {
                    screenWidth = 1024;
                    screenHeight = 768;
                }
                if (e.key.keysym.sym == SDLK_3) {
                    screenWidth = 1280;
                    screenHeight = 720;
                }
                if (e.key.keysym.sym == SDLK_f) {
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(gWindow, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
                }
            }
        }

        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(gRenderer);

        renderText("Select Resolution:", 50, 50, white);
        renderText("Press 1 for 800x600", 50, 100, white);
        renderText("Press 2 for 1024x768", 50, 150, white);
        renderText("Press 3 for 1280x720", 50, 200, white);
        renderText("Press F to toggle fullscreen", 50, 250, white);

        SDL_RenderPresent(gRenderer);

        SDL_Delay(100);
    }
}

// Function to check and update enemy positions
void updateEnemies(std::vector<Enemy>& enemies, int playerX, int playerY, int screenWidth, int screenHeight) {
    for (auto& enemy : enemies) {
        if (enemy.active) {
            // Move towards the player
            if (enemy.x < playerX) enemy.x += ENEMY_SPEED;
            if (enemy.x > playerX) enemy.x -= ENEMY_SPEED;
            if (enemy.y < playerY) enemy.y += ENEMY_SPEED;
            if (enemy.y > playerY) enemy.y -= ENEMY_SPEED;

            // Check boundaries
            if (enemy.x < 0) enemy.x = 0;
            if (enemy.x > screenWidth - ENEMY_SIZE) enemy.x = screenWidth - ENEMY_SIZE;
            if (enemy.y < 0) enemy.y = 0;
            if (enemy.y > screenHeight - ENEMY_SIZE) enemy.y = screenHeight - ENEMY_SIZE;
        }
    }

    // Check for enemy collisions
    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            if (enemies[i].active && enemies[j].active) {
                if (abs(enemies[i].x - enemies[j].x) < ENEMY_SIZE &&
                    abs(enemies[i].y - enemies[j].y) < ENEMY_SIZE) {
                    // Move enemies to random positions if they collide
                    enemies[j].x = rand() % (screenWidth - ENEMY_SIZE);
                    enemies[j].y = rand() % (screenHeight - ENEMY_SIZE);
                }
            }
        }
    }
}

// Function to render enemies
void renderEnemies(const std::vector<Enemy>& enemies) {
    SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF); // Green color for enemies
    for (const auto& enemy : enemies) {
        if (enemy.active) {
            SDL_Rect fillRect = {enemy.x, enemy.y, ENEMY_SIZE, ENEMY_SIZE};
            SDL_RenderFillRect(gRenderer, &fillRect);
        }
    }
}

// Function to move the player to a safe place
void respawnPlayer(int& playerX, int& playerY, int screenWidth, int screenHeight) {
    playerX = rand() % (screenWidth - PLAYER_SIZE);
    playerY = rand() % (screenHeight - PLAYER_SIZE);
}

// Main function
int main(int argc, char* args[]) {
    srand(static_cast<unsigned>(time(0))); // Initialize random number generator

    if (!init()) {
        std::cerr << "Failed to initialize!" << std::endl;
        return -1;
    }

    int screenWidth = WINDOW_WIDTH;
    int screenHeight = WINDOW_HEIGHT;
    bool fullscreen = false;

    gWindow = SDL_CreateWindow("Pixel Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        close();
        return -1;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        close();
        return -1;
    }

    gFont = TTF_OpenFont("arial.ttf", 28); // Make sure you have Arial.ttf font in the current directory
    if (gFont == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        close();
        return -1;
    }

    showMenu(screenWidth, screenHeight, fullscreen);

    bool quit = false;
    int playerX = screenWidth / 2;
    int playerY = screenHeight / 2;
    int score = 0;
    int lives = INITIAL_LIVES;
    Uint32 lastScoreUpdate = SDL_GetTicks();
    bool immune = false;
    Uint32 immuneStartTime = 0;

    std::vector<Enemy> enemies(NUM_ENEMIES);
    for (auto& enemy : enemies) {
        enemy.x;
        enemy.x = rand() % (screenWidth - ENEMY_SIZE);
        enemy.y = rand() % (screenHeight - ENEMY_SIZE);
        enemy.active = true;
    }

    Uint32 lastTick = SDL_GetTicks();
    Uint32 lastCollision = 0;

    while (!quit) {
        Uint32 currentTick = SDL_GetTicks();
        Uint32 deltaTime = currentTick - lastTick;

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
        if (!immune) {
            if (currentKeyStates[SDL_SCANCODE_W]) {
                playerY -= PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_S]) {
                playerY += PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_A]) {
                playerX -= PLAYER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_D]) {
                playerX += PLAYER_SPEED;
            }
        }

        // Check player boundaries
        if (playerX < 0) playerX = 0;
        if (playerX > screenWidth - PLAYER_SIZE) playerX = screenWidth - PLAYER_SIZE;
        if (playerY < 0) playerY = 0;
        if (playerY > screenHeight - PLAYER_SIZE) playerY = screenHeight - PLAYER_SIZE;

        if (!immune && (currentTick - lastCollision) >= 100) {
            for (auto& enemy : enemies) {
                if (enemy.active &&
                    playerX < enemy.x + ENEMY_SIZE && playerX + PLAYER_SIZE > enemy.x &&
                    playerY < enemy.y + ENEMY_SIZE && playerY + PLAYER_SIZE > enemy.y) {

                    lives--;
                    if (lives <= 0) {
                        std::cout << "Game Over!" << std::endl;
                        quit = true;
                    } else {
                        respawnPlayer(playerX, playerY, screenWidth, screenHeight);
                        immune = true;
                        immuneStartTime = currentTick;
                        lastCollision = currentTick;
                    }
                }
            }
        }

        // Update enemies
        if (!immune) {
            updateEnemies(enemies, playerX, playerY, screenWidth, screenHeight);
        }

        // Update score
        if (currentTick - lastScoreUpdate >= SCORE_INCREMENT_INTERVAL) {
            score++;
            lastScoreUpdate = currentTick;
        }

        // Render
        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(gRenderer);

        if (immune) {
            Uint32 immuneTime = currentTick - immuneStartTime;
            if ((immuneTime / 500) % 2 == 0) {
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            } else {
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
            }
        } else {
            SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF); // Blue color for player
        }
        SDL_Rect playerRect = { playerX, playerY, PLAYER_SIZE, PLAYER_SIZE };
        SDL_RenderFillRect(gRenderer, &playerRect);

        renderEnemies(enemies);

        SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};
        renderText("Score: " + std::to_string(score), 10, 10, white);
        renderText("Lives: " + std::to_string(lives), 10, 50, white);

        SDL_RenderPresent(gRenderer);

        // Handle immunity
        if (immune && (currentTick - immuneStartTime >= IMMUNITY_DURATION)) {
            immune = false;
        }

        SDL_Delay(FRAME_DELAY);
        lastTick = currentTick;
    }

    close();
    return 0;
}
