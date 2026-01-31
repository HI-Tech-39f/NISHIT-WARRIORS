#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <fstream>

using namespace std;
using namespace std::chrono;

// ==========================================
// 1. GLOBAL SETTINGS & VARIABLES
// ==========================================

const int WIDTH = 90;
const int HEIGHT = 26;
const int TARGET_FPS = 120; // Doubled from 60 to 120 for 2x speed
const int FRAME_TIME = 1000 / TARGET_FPS; // milliseconds per frame
const string HIGH_SCORE_FILE = "highscore.dat";

// Color Codes
const int BLUE = 9;
const int GREEN = 10;
const int CYAN = 11;
const int RED = 12;
const int MAGENTA = 13;
const int YELLOW = 14;
const int WHITE = 15;
const int DARKBROWN = 6;

struct Object {
    int x, y;
    int hp;
    int type; // 0 = Small Enemy, 1 = Monster
};

struct Bullet {
    float x, y; 
    bool isPlayer;
    int dy; 
};

struct PowerUp {
    int x, y;
    bool active;
};

char board[HEIGHT][WIDTH];
int playerX, playerY;
int playerHP;
int score;
int highScore = 0;
int enemiesKilledForBoost;
int triShotAmmo;
bool hasBomb; 
int bossHP;
int maxBossHP;
bool bossActive;
bool gameRunning;
bool isPaused; 

vector<Object> enemies;
vector<Bullet> bullets;
PowerUp boostPack;

// Frame timing
high_resolution_clock::time_point lastFrameTime;
int frameCounter = 0;

// Difficulty scaling
float difficultyMultiplier = 1.0f;
int lastScoreMilestone = 0;

// ==========================================
// 2. HELPER FUNCTIONS
// ==========================================

void Color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void Gotoxy(int x, int y) {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void HideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// ==========================================
// 3. HIGH SCORE MANAGEMENT
// ==========================================

int LoadHighScore() {
    ifstream file(HIGH_SCORE_FILE);
    int score = 0;
    if (file.is_open()) {
        file >> score;
        file.close();
    }
    return score;
}

void SaveHighScore(int score) {
    ofstream file(HIGH_SCORE_FILE);
    if (file.is_open()) {
        file << score;
        file.close();
    }
}

void UpdateHighScore() {
    if (score > highScore) {
        highScore = score;
        SaveHighScore(highScore);
    }
}

// ==========================================
// 4. SOUNDS & MUSIC
// ==========================================

void PlayFireSound() { Beep(800, 5); }
void PlayHitSound() { Beep(200, 10); }
void PlayExplosionSound() { Beep(100, 30); }
void PlayPowerUpSound() { Beep(1200, 20); Beep(1500, 20); }
void PlayMenuSound() { Beep(400, 50); }
void PlayHighScoreSound() { 
    Beep(800, 100); 
    Beep(1000, 100); 
    Beep(1200, 100); 
    Beep(1500, 200); 
}

// ==========================================
// 5. GRAPHICS & MENU
// ==========================================

// Forward declarations
void ShowMissionBriefing();
void ShowHowToPlay();
void ShowHighScores();

void DrawLogo(bool blinkState) {
    Gotoxy(0, 1);
    if (blinkState) Color(CYAN); else Color(BLUE);
    cout << "\t    _   _ ___ _____ _   _ _____  _    _  ___  ______ " << endl;
    cout << "\t   | \\ | |_ _/  ___| | | |_   _|| |  | |/ _ \\ | ___ \\" << endl;
    if (blinkState) Color(WHITE); else Color(RED);
    cout << "\t   |  \\| || |\\ `--. | |_| | | |  | |  | / /_\\ \\| |_/ /" << endl;
    cout << "\t   | . ` || | `--. \\|  _  | | |  | |/\\| |  _  ||    / " << endl;
    if (blinkState) Color(CYAN); else Color(BLUE);
    cout << "\t   | |\\  || |/\\__/ /| | | | | |  \\  /\\  / | | || |\\ \\ " << endl;
    cout << "\t   \\_| \\_/\\_/\\____/ \\_| |_/ \\_/   \\/  \\/\\_| |_/\\_| \\_|" << endl;
    Color(WHITE);
    Gotoxy(25, 10); cout << "--- THE ULTIMATE WARRIOR ARCADE ---";
}

void ShowMenu() {
    system("cls");
    bool blink = true;
    int blinkCounter = 0;
    
    while (!_kbhit()) {
        Gotoxy(0, 0);
        Color(CYAN);
        for (int i = 0; i < 92; i++) cout << (char)205; cout << endl;
        
        DrawLogo(blink);
        
        // Display high score
        Gotoxy(32, 11); 
        Color(YELLOW); 
        cout << "HIGH SCORE: " << highScore;
        
        if (++blinkCounter % 3 == 0) blink = !blink;
        PlayMenuSound();
        
        Gotoxy(0, 12);
        Color(CYAN);
        for (int i = 0; i < 92; i++) cout << (char)205; cout << endl;
        
        Gotoxy(0, 13);
        Color(CYAN);
        cout << (char)186; 
        for (int i = 0; i < 90; i++) cout << " ";
        cout << (char)186 << endl;
        
        Gotoxy(28, 14); Color(YELLOW); cout << (char)187 << " 1. START MISSION " << (char)187;
        Gotoxy(0, 15); Color(CYAN); cout << (char)186;
        for (int i = 0; i < 90; i++) cout << " ";
        cout << (char)186 << endl;
        
        Gotoxy(28, 16); Color(WHITE);  cout << (char)187 << " 2. MISSION BRIEFING " << (char)187;
        Gotoxy(0, 17); Color(CYAN); cout << (char)186;
        for (int i = 0; i < 90; i++) cout << " ";
        cout << (char)186 << endl;
        
        Gotoxy(28, 18); Color(CYAN);   cout << (char)187 << " 3. HOW TO PLAY " << (char)187;
        Gotoxy(0, 19); Color(CYAN); cout << (char)186;
        for (int i = 0; i < 90; i++) cout << " ";
        cout << (char)186 << endl;
        
        Gotoxy(28, 20); Color(RED);    cout << (char)187 << " 4. RETREAT (Exit) " << (char)187;
        Gotoxy(0, 21); Color(CYAN); cout << (char)186;
        for (int i = 0; i < 90; i++) cout << " ";
        cout << (char)186 << endl;
        
        Gotoxy(0, 22);
        Color(CYAN);
        for (int i = 0; i < 92; i++) cout << (char)205; cout << endl;
        
        Sleep(200); 
    }
    char ch = _getch();
    if (ch == '1') return;
    if (ch == '2') {
        ShowMissionBriefing();
    }
    if (ch == '3') {
        ShowHowToPlay();
    }
    if (ch == '4') exit(0);
}

void ShowMissionBriefing() {
    system("cls");
    Color(MAGENTA);
    cout << "\n";
    for (int i = 0; i < 80; i++) cout << "=";
    cout << "\n";
    Color(CYAN);
    cout << "\t\t\t   *** MISSION BRIEFING ***\n";
    Color(MAGENTA);
    cout << "\n";
    for (int i = 0; i < 80; i++) cout << "=";
    cout << "\n\n";
    
    Color(YELLOW);
    cout << "\t STATUS: CLASSIFIED TOP SECRET\n\n";
    
    Color(WHITE);
    cout << "\t OBJECTIVE:\n";
    Color(GREEN);
    cout << "\t   Defend Earth against an alien invasion. Small enemy jets are attacking\n";
    cout << "\t   in waves, and a massive boss monster awaits at higher threat levels.\n";
    cout << "\t   When the boss appears, it brings fighter escorts to overwhelm defenses!\n\n";
    
    Color(WHITE);
    cout << "\t ENEMY INTEL:\n";
    Color(RED);
    cout << "\t   - Red Jets: Fast-moving scouts (5 points each)\n";
    cout << "\t   - Purple Monster: Heavy Boss Unit (100 points)\n";
    cout << "\t   - Boss Escort: 4-6 fighter jets accompany each boss wave\n";
    cout << "\t   - During boss battles, additional fighters continue spawning\n\n";
    
    Color(WHITE);
    cout << "\t REWARD SYSTEM:\n";
    Color(YELLOW);
    cout << "\t   - Eliminate 10 jets to earn a Health Boost Power-Up\n";
    cout << "\t   - Each boss defeated grants 100 points\n\n";
    
    Color(WHITE);
    cout << "\t SPECIAL WEAPONS:\n";
    Color(MAGENTA);
    cout << "\t   - Tri-Shot Ammo: 60 rounds available when boss appears\n";
    cout << "\t   - Bomb Charge: Activate when boss is active (20 damage)\n\n";
    
    Color(MAGENTA);
    for (int i = 0; i < 80; i++) cout << "=";
    cout << "\n";
    Color(WHITE);
    cout << "\n\t\t\t   Press any key to return to menu...";
    Color(WHITE);
    _getch();
    ShowMenu();
}

void ShowHowToPlay() {
    system("cls");
    Color(MAGENTA);
    cout << "\n";
    for (int i = 0; i < 80; i++) cout << "=";
    cout << "\n";
    Color(CYAN);
    cout << "\t\t\t      *** HOW TO PLAY ***\n";
    Color(MAGENTA);
    cout << "\n";
    for (int i = 0; i < 80; i++) cout << "=";
    cout << "\n\n";
    
    Color(YELLOW);
    cout << "\t MOVEMENT:\n";
    Color(WHITE);
    cout << "\t   UP ARROW    -> Move upward\n";
    cout << "\t   DOWN ARROW  -> Move downward\n";
    cout << "\t   LEFT ARROW  -> Move left\n";
    cout << "\t   RIGHT ARROW -> Move right\n\n";
    
    Color(YELLOW);
    cout << "\t COMBAT:\n";
    Color(WHITE);
    cout << "\t   SPACEBAR -> Fire continuous beam at enemies\n";
    cout << "\t   B KEY    -> Activate bomb (when available & boss is active)\n";
    cout << "\t              - Bomb deals 20 damage to the boss\n\n";
    
    Color(YELLOW);
    cout << "\t GAME CONTROL:\n";
    Color(WHITE);
    cout << "\t   P KEY   -> Pause/Resume game\n";
    cout << "\t   ESC KEY -> Quit game\n\n";
    
    Color(YELLOW);
    cout << "\t GAME MECHANICS:\n";
    Color(GREEN);
    cout << "\t   + Collect health boost power-ups to recover health (max: 100%)\n";
    cout << "\t   + Avoid enemy bullets or take damage\n";
    cout << "\t   + Your ship has a default health of 100%\n";
    cout << "\t   + Game ends when your health reaches 0%\n";
    cout << "\t   + Boss appears every 30 points with 4-6 fighter escorts\n";
    cout << "\t   + During boss battles, more fighters spawn continuously\n";
    cout << "\t   + Game speed increases every 20 points (up to 2.5x max)\n";
    cout << "\t   + High score is automatically saved to file\n\n";
    
    Color(YELLOW);
    cout << "\t TRI-SHOT WEAPON:\n";
    Color(MAGENTA);
    cout << "\t   When active, spacebar fires 3 bullets instead of 1\n";
    cout << "\t   Available: 60 ammo rounds per boss encounter\n\n";
    
    Color(MAGENTA);
    for (int i = 0; i < 80; i++) cout << "=";
    cout << "\n";
    Color(WHITE);
    cout << "\n\t\t\t   Press any key to return to menu...";
    Color(WHITE);
    _getch();
    ShowMenu();
}

void Setup() {
    playerX = 5; playerY = HEIGHT / 2;
    playerHP = 100; score = 0;
    enemiesKilledForBoost = 0; triShotAmmo = 0;
    hasBomb = false; bossActive = false;
    boostPack.active = false; isPaused = false;
    enemies.clear(); bullets.clear();
    gameRunning = true;
    frameCounter = 0;
    lastFrameTime = high_resolution_clock::now();
    difficultyMultiplier = 1.0f;
    lastScoreMilestone = 0;
}

// ==========================================
// 6. INPUT HANDLING
// ==========================================

void Input() {
    // Check for pause key FIRST, only when P is pressed
    static bool pKeyPressed = false;
    if (GetAsyncKeyState('P') & 0x8000) {
        if (!pKeyPressed) {
            isPaused = !isPaused;
            pKeyPressed = true;
        }
    } else {
        pKeyPressed = false;
    }
    
    if (isPaused) return;

    // Check for bomb key
    static bool bKeyPressed = false;
    if (GetAsyncKeyState('B') & 0x8000) {
        if (!bKeyPressed && bossActive && hasBomb) {
            bossHP -= 20; 
            hasBomb = false;
            PlayExplosionSound();
            bKeyPressed = true;
        }
    } else {
        bKeyPressed = false;
    }

    // Smooth movement - responsive controls (2x speed)
    int moveSpeed = 2; // Doubled from 1
    if (GetAsyncKeyState(VK_UP) & 0x8000) playerY = max(2, playerY - moveSpeed);
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) playerY = min(HEIGHT - 3, playerY + moveSpeed);
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) playerX = max(1, playerX - moveSpeed);
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) playerX = min(WIDTH / 2, playerX + moveSpeed);

    // Firing with cooldown for better performance
    static int fireCooldown = 0;
    if (fireCooldown > 0) fireCooldown--;
    
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        if (fireCooldown == 0) {
            bullets.push_back({ (float)playerX + 7, (float)playerY, true, 0 });
            if (triShotAmmo > 0) {
                bullets.push_back({ (float)playerX + 7, (float)playerY, true, -1 });
                bullets.push_back({ (float)playerX + 7, (float)playerY, true, 1 });
                triShotAmmo--;
            }
            PlayFireSound();
            fireCooldown = 3; // Reduced from 5 for faster firing
        }
    }
    
    if (GetAsyncKeyState(VK_ESCAPE)) gameRunning = false;
}

// ==========================================
// 7. GAME LOGIC
// ==========================================

void Logic() {
    if (isPaused) return;

    frameCounter++;

    // Progressive difficulty scaling - increases every 20 points
    int currentMilestone = (score / 20) * 20;
    if (currentMilestone > lastScoreMilestone) {
        lastScoreMilestone = currentMilestone;
        difficultyMultiplier += 0.12f; // Smoother increase: 12% every 20 points
        if (difficultyMultiplier > 2.5f) difficultyMultiplier = 2.5f; // Cap at 2.5x speed
    }

    // Boss spawn logic
    if (score > 0 && score % 30 == 0 && !bossActive) {
        bossActive = true; maxBossHP = 100; bossHP = maxBossHP;
        enemies.push_back({ WIDTH - 20, HEIGHT / 2, bossHP, 1 });
        triShotAmmo = 60; hasBomb = true; 
        
        // Spawn fighter jets along with the boss
        int numFighters = 4 + (rand() % 3); // 4 to 6 fighters
        for (int f = 0; f < numFighters; f++) {
            int spawnY = 3 + (rand() % (HEIGHT - 6));
            int spawnX = WIDTH - 10 - (rand() % 15);
            enemies.push_back({ spawnX, spawnY, 1, 0 });
        }
    }

    // Enemy spawning - smoother controlled rate (2x speed)
    if (enemies.size() < (bossActive ? 12 : 8)) {
        int spawnRate = max(10, (int)(18 - (difficultyMultiplier - 1.0f) * 4)); // Halved for 2x speed
        if (frameCounter % spawnRate == 0) {
            enemies.push_back({ WIDTH - 6, (rand() % (HEIGHT - 6)) + 3, 1, 0 });
        }
    }

    // Bullet Movement - optimized speeds (2x speed)
    for (int i = 0; i < (int)bullets.size(); i++) {
        if (bullets[i].isPlayer) {
            bullets[i].x += 6.0f; // Doubled from 3.0
            bullets[i].y += (float)bullets[i].dy * 0.5f; // Doubled from 0.25
        } else { 
            bullets[i].x -= 2.4f * difficultyMultiplier; // Doubled from 1.2
            bullets[i].y += (float)bullets[i].dy * 0.8f; // Doubled from 0.4
        }

        if (bullets[i].x >= WIDTH || bullets[i].x <= 0 || bullets[i].y <= 1 || bullets[i].y >= HEIGHT - 1) {
            bullets.erase(bullets.begin() + i); i--;
        }
    }

    // Enemy/Boss Logic (2x speed)
    for (int i = 0; i < (int)enemies.size(); i++) {
        if (enemies[i].type == 1) { // Boss
            if (enemies[i].x > WIDTH - 25) {
                enemies[i].x -= max(2, (int)(1.6f * difficultyMultiplier)); // Doubled from 0.8
            }
            
            // Boss shoots with smoother rate (2x speed)
            int bossFireRate = max(13, (int)(23 - (difficultyMultiplier - 1.0f) * 3)); // Halved for faster shooting
            if (frameCounter % bossFireRate == 0) {
                bullets.push_back({ (float)enemies[i].x - 1, (float)enemies[i].y, false, 0 });
                bullets.push_back({ (float)enemies[i].x - 1, (float)enemies[i].y, false, -1 });
                bullets.push_back({ (float)enemies[i].x - 1, (float)enemies[i].y, false, 1 });
            }
            
            // Boss tracks player smoothly (2x speed)
            int bossTrackSpeed = max(2, (int)(2 - (difficultyMultiplier - 1.0f) * 0.15f)); // Halved for faster tracking
            if (frameCounter % bossTrackSpeed == 0) {
                if (playerY < enemies[i].y) enemies[i].y--;
                else if (playerY > enemies[i].y) enemies[i].y++;
            }
        } else { // Small enemies (2x speed)
            enemies[i].x -= max(2, (int)(2.6f * difficultyMultiplier)); // Doubled from 1.3
        }

        if (enemies[i].x <= 1) {
            if(enemies[i].type == 0) { 
                playerHP -= 5; 
                PlayHitSound(); 
                enemies.erase(enemies.begin() + i); 
                i--; 
            }
            continue;
        }

        // Collision with player
        if (abs(enemies[i].x - playerX) < 6 && abs(enemies[i].y - playerY) < 2) {
            playerHP -= 10; 
            PlayExplosionSound();
            if(enemies[i].type == 0) { 
                enemies.erase(enemies.begin() + i); 
                i--; 
            }
        }
    }

    // Hit Detection
    for (int i = 0; i < (int)bullets.size(); i++) {
        bool bulletRemoved = false;
        if (bullets[i].isPlayer) {
            for (int j = 0; j < (int)enemies.size(); j++) {
                if (bullets[i].x >= (float)enemies[j].x && 
                    bullets[i].x <= (float)(enemies[j].x + 10) && 
                    fabs(bullets[i].y - (float)enemies[j].y) < 2.0f) {
                    
                    if(enemies[j].type == 1) bossHP -= 2; 
                    else enemies[j].hp--;
                    
                    bulletRemoved = true;
                    
                    if ((enemies[j].type == 1 && bossHP <= 0) || 
                        (enemies[j].type == 0 && enemies[j].hp <= 0)) {
                        if (enemies[j].type == 1) { 
                            score += 100; 
                            bossActive = false; 
                            PlayExplosionSound(); 
                        }
                        else { 
                            score += 5; 
                            enemiesKilledForBoost++; 
                        }
                        
                        if (enemiesKilledForBoost >= 10) { 
                            boostPack.active = true; 
                            boostPack.x = enemies[j].x; 
                            boostPack.y = enemies[j].y; 
                            enemiesKilledForBoost = 0; 
                        }
                        enemies.erase(enemies.begin() + j);
                    }
                    break; 
                }
            }
        } else {
            if (fabs(bullets[i].x - (float)playerX) < 4.0f && (int)bullets[i].y == playerY) {
                playerHP -= 5; 
                PlayHitSound(); 
                bulletRemoved = true;
            }
        }
        if (bulletRemoved) { 
            bullets.erase(bullets.begin() + i); 
            i--; 
        }
    }

    // Power-up logic (2x speed)
    if (boostPack.active) {
        boostPack.x -= max(4, (int)(3.6f * difficultyMultiplier)); // Doubled from 2 and 1.8
        if (abs(boostPack.x - playerX) < 5 && boostPack.y == playerY) { 
            playerHP += 30; 
            if(playerHP > 100) playerHP = 100; 
            PlayPowerUpSound(); 
            boostPack.active = false; 
        }
        if (boostPack.x <= 1) boostPack.active = false;
    }

    if (playerHP <= 0) gameRunning = false;
}

// ==========================================
// 8. DRAWING
// ==========================================

void Draw() {
    if (isPaused) {
        Gotoxy(WIDTH / 2 - 5, HEIGHT / 2);
        Color(RED);
        cout << "*** PAUSED ***";
        Color(WHITE);
        return;
    }
    
    Gotoxy(0, 0);
    
    // Clear board and add stars
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (rand() % 200 == 0) board[y][x] = '.'; 
            else board[y][x] = ' ';                
        }
    }

    // Draw player
    string pBody = "}==^==>";
    for (int i = 0; i < (int)pBody.length(); i++) 
        if (playerX + i < WIDTH) board[playerY][playerX + i] = pBody[i];
    if (playerY > 0) board[playerY-1][playerX+2] = '\\'; 
    if (playerY < HEIGHT - 1) board[playerY+1][playerX+2] = '/';

    // Draw enemies
    for (int k = 0; k < (int)enemies.size(); k++) {
        int ex = enemies[k].x; 
        int ey = enemies[k].y;
        
        if (enemies[k].type == 1) { // Boss
            string bossBody = "[[[--MONSTER--]]]";
            for(int i=0; i<(int)bossBody.length(); i++) 
                if(ex+i < WIDTH && ex+i >= 0) board[ey][ex+i] = bossBody[i];
            if(ey > 0 && ex+5 < WIDTH) board[ey-1][ex+5] = '/'; 
            if(ey < HEIGHT - 1 && ex+5 < WIDTH) board[ey+1][ex+5] = '\\'; 
        } else { // Small enemy
            string eBody = "<==^=={";
            for(int i=0; i<(int)eBody.length(); i++) 
                if(ex+i < WIDTH && ex+i >= 0) board[ey][ex+i] = eBody[i];
            if(ey > 0 && ex+3 < WIDTH) board[ey-1][ex+3] = '/'; 
            if(ey < HEIGHT - 1 && ex+3 < WIDTH) board[ey+1][ex+3] = '\\'; 
        }
    }

    // Draw bullets
    for (int k = 0; k < (int)bullets.size(); k++) {
        int bx = (int)bullets[k].x; 
        int by = (int)bullets[k].y;
        if (bx >= 0 && bx < WIDTH && by >= 0 && by < HEIGHT)
            board[by][bx] = bullets[k].isPlayer ? '*' : 'o';
    }
    
    // Draw power-up
    if (boostPack.active && boostPack.x > 0 && boostPack.x < WIDTH) 
        board[boostPack.y][boostPack.x] = 3;

    // Render board
    Color(CYAN);
    for (int i = 0; i < WIDTH + 2; i++) cout << (char)178; 
    cout << endl;
    
    for (int y = 0; y < HEIGHT; y++) {
        Color(CYAN); 
        cout << (char)178; 
        
        for (int x = 0; x < WIDTH; x++) {
            char c = board[y][x];
            
            // Color selection
            if (c == '}' || c == '^' || c == '>' || c == '\\' || c == '/') 
                Color(GREEN); 
            else if (c == '<' || c == '{') 
                Color(RED); 
            else if (c == '[' || c == '-' || c == 'M' || c == 'O' || c == 'N' || c == 'S') 
                Color(MAGENTA);
            else if ((unsigned char)c == 220) 
                Color(DARKBROWN);
            else if (c == '*') 
                Color(YELLOW);
            else if (c == 3) 
                Color(RED);
            else 
                Color(WHITE);
            
            cout << c;
        }
        cout << endl; 
    }
    
    Color(CYAN);
    for (int i = 0; i < WIDTH + 2; i++) cout << (char)178; 
    cout << endl;
    
    // Score Display
    Color(YELLOW); 
    cout << " SCORE: " << score << " ";
    
    // High Score Display
    Color(MAGENTA);
    cout << "| HIGH: " << highScore << " ";
    
    // Difficulty Display
    Color(CYAN);
    cout << "| SPEED: x" << fixed;
    cout.precision(1);
    cout << difficultyMultiplier << " ";
    
    // Health Bar with Percentage
    Color(WHITE);  
    cout << "| HP: ";
    if (playerHP > 50) Color(GREEN); 
    else if (playerHP > 30) Color(YELLOW); 
    else Color(RED);
    
    int healthBarLength = (playerHP / 10);
    cout << "[";
    for (int i = 0; i < healthBarLength; i++) cout << (char)219;
    for (int i = healthBarLength; i < 10; i++) cout << " ";
    cout << "] " << playerHP << "%  ";
    
    // Bomb Status
    Color(CYAN);   
    cout << "| BOMB: " << (hasBomb ? "READY" : "----") << " ";
    
    // Boss Status
    Color(MAGENTA); 
    cout << "| BOSS: ";
    if (bossActive) {
        int bossBarLength = (bossHP / 10);
        cout << "[";
        for (int i = 0; i < bossBarLength; i++) cout << (char)219;
        for (int i = bossBarLength; i < 10; i++) cout << " ";
        cout << "]";
    } else {
        cout << "----";
    }
    cout << "   \r";
}

// ==========================================
// 9. FRAME RATE CONTROL
// ==========================================

void FrameRateControl() {
    auto currentTime = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(currentTime - lastFrameTime).count();
    
    if (elapsed < FRAME_TIME) {
        Sleep(FRAME_TIME - elapsed);
    }
    
    lastFrameTime = high_resolution_clock::now();
}

// ==========================================
// 10. MAIN GAME LOOP
// ==========================================

int main() {
    srand((unsigned int)time(0));
    HideCursor();
    
    // Load high score
    highScore = LoadHighScore();
    
    ShowMenu();
    Setup();
    
    while (gameRunning) {
        Input();
        Logic();
        Draw();
        FrameRateControl(); // Smooth 60 FPS
    }
    
    // Update high score if needed
    bool isNewHighScore = false;
    if (score > highScore) {
        isNewHighScore = true;
        UpdateHighScore();
    }
    
    system("cls");
    
    if (isNewHighScore) {
        PlayHighScoreSound();
        Color(YELLOW);
        cout << "\n\n\n\t\t ############################" << endl;
        cout << "\t\t #   NEW HIGH SCORE!!!      #" << endl;
        cout << "\t\t ############################" << endl;
        Color(WHITE);
        cout << "\n\t\t    Your Score: " << score << endl;
        cout << "\t\t    Previous Best: " << (highScore - score + LoadHighScore()) << endl;
    } else {
        PlayExplosionSound();
        Color(RED);
        cout << "\n\n\n\t\t ############################" << endl;
        cout << "\t\t #      MISSION FAILED      #" << endl;
        cout << "\t\t ############################" << endl;
        Color(WHITE);
        cout << "\n\t\t    Your Score: " << score << endl;
        cout << "\t\t    High Score: " << highScore << endl;
    }
    
    Color(CYAN);
    cout << "\n\n\t\t Press any key to exit...";
    Color(WHITE);
    _getch();
    return 0;
}
