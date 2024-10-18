#include "ESP32S3VGA.h"
#include <GfxWrapper.h>
#include <esp_now.h>
#include <WiFi.h>

// ESP-NOW command definitions
#define COMMAND_UP 1
#define COMMAND_DOWN 2
#define COMMAND_SELECT 3
#define COMMAND_PLAYER2_UP 4
#define COMMAND_PLAYER2_DOWN 5

typedef struct struct_message {
    int command;
} struct_message;

struct_message myData;

// VGA setup
const PinConfig pins = {-1,-1,-1,-1,1, -1,-1,-1,-1,-1,2, -1,-1,-1,-1,3, 10,11};
VGA vga;
Mode mode = Mode::MODE_320x240x60;
GfxWrapper<VGA> gfx(vga, mode.hRes, mode.vRes);

// Screen dimensions
const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

// Game objects and properties
int leftPaddleY, rightPaddleY;
int ballX, ballY;
int ballVelocityX, ballVelocityY;
const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 50;
const int PADDLE_SPEED = 5;
const int BALL_SIZE = 6;
const int BALL_SPEED = 3;

// Colors
const uint16_t COLOR_BACKGROUND = vga.rgb(0, 0, 64);
const uint16_t COLOR_PADDLE = vga.rgb(255, 255, 255);
const uint16_t COLOR_BALL = vga.rgb(255, 255, 0);
const uint16_t COLOR_TEXT = vga.rgb(0, 255, 0);

// Scoring
int leftScore = 0;
int rightScore = 0;

// Sound
const int SOUND_PIN = 18;
const int SOUND_CHANNEL = 0;

// Ode to Joy melody
const int melodySize = 16;
const int melody[melodySize] = {330, 330, 349, 392, 392, 349, 330, 294, 262, 262, 294, 330, 330, 294, 294, 0};
const int noteDurations[melodySize] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 4};

// Background effect
const int NUM_STARS = 50;
int starX[NUM_STARS];
int starY[NUM_STARS];
uint16_t starColors[NUM_STARS];

// Game mode
bool singlePlayerMode = true;

// Button states
bool leftPaddleUp = false;
bool leftPaddleDown = false;
bool rightPaddleUp = false;
bool rightPaddleDown = false;
bool selectPressed = false;

// Function prototypes
void initializeGame();
void updatePaddles();
void updateBall();
void drawGame();
void playSound(int frequency, int duration);
void playOdeToJoy();
void displayWinMessage(int winner);
void initializeStars();
void updateStars();
void drawStars();
void showMenu();
void playSinglePlayerGame();
void playTwoPlayerGame();

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));
    // Process the received command
    switch(myData.command) {
        case COMMAND_UP:
            leftPaddleUp = true;
            leftPaddleDown = false;
            break;
        case COMMAND_DOWN:
            leftPaddleDown = true;
            leftPaddleUp = false;
            break;
        case COMMAND_SELECT:
            selectPressed = true;
            break;
        case COMMAND_PLAYER2_UP:
            rightPaddleUp = true;
            rightPaddleDown = false;
            break;
        case COMMAND_PLAYER2_DOWN:
            rightPaddleDown = true;
            rightPaddleUp = false;
            break;
    }
}

void setup() {
    vga.bufferCount = 2;
    if(!vga.init(pins, mode, 8)) while(1) delay(1);
    vga.start();

    // Setup sound
    ledcSetup(SOUND_CHANNEL, 5000, 8);
    ledcAttachPin(SOUND_PIN, SOUND_CHANNEL);

    initializeStars();
    
    // Set up ESP-NOW
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
    showMenu();
}

void showMenu() {
    const char* menuItems[] = {"Single Player", "Two Players", "Quit"};
    int selectedItem = 0;
    bool menuActive = true;

    while (menuActive) {
        gfx.fillScreen(COLOR_BACKGROUND);
        drawStars();

        gfx.setTextColor(COLOR_TEXT);
        gfx.setTextSize(2);
        gfx.setCursor(SCREEN_WIDTH / 2 - 70, 30);
        gfx.print("PONG GAME");

        for (int i = 0; i < 3; i++) {
            if (i == selectedItem) {
                gfx.setTextColor(COLOR_BALL);
            } else {
                gfx.setTextColor(COLOR_TEXT);
            }
            gfx.setCursor(SCREEN_WIDTH / 2 - 60, 80 + i * 40);
            gfx.print(menuItems[i]);
        }

        vga.show();

        // Check for received commands
        if (leftPaddleUp) {
            selectedItem = (selectedItem - 1 + 3) % 3;
            leftPaddleUp = false;
            delay(200);  // Debounce
        } else if (leftPaddleDown) {
            selectedItem = (selectedItem + 1) % 3;
            leftPaddleDown = false;
            delay(200);  // Debounce
        } else if (selectPressed) {
            selectPressed = false;
            delay(200);  // Debounce
            switch (selectedItem) {
                case 0:
                    singlePlayerMode = true;
                    playSinglePlayerGame();
                    menuActive = false;
                    break;
                case 1:
                    singlePlayerMode = false;
                    playTwoPlayerGame();
                    menuActive = false;
                    break;
                case 2:
                    // Quit game (you can add any cleanup code here)
                    menuActive = false;
                    break;
            }
        }

        updateStars();
        delay(16);  // Small delay to prevent CPU hogging
    }
}

void playSinglePlayerGame() {
    initializeGame();
    while (true) {
        updatePaddles();
        updateBall();
        drawGame();
        if (leftScore >= 10 || rightScore >= 10) {
            int winner = leftScore >= 10 ? 1 : 2;
            displayWinMessage(winner);
            playOdeToJoy();
            delay(5000);
            break;
        }
        if (selectPressed) {
            selectPressed = false;
            break;  // Exit to menu
        }
        delay(16);
    }
}

void playTwoPlayerGame() {
    initializeGame();
    while (true) {
        updatePaddles();
        updateBall();
        drawGame();
        if (leftScore >= 10 || rightScore >= 10) {
            int winner = leftScore >= 10 ? 1 : 2;
            displayWinMessage(winner);
            playOdeToJoy();
            delay(5000);
            break;
        }
        if (selectPressed) {
            selectPressed = false;
            break;  // Exit to menu
        }
        delay(16);
    }
}

void initializeGame() {
    leftPaddleY = rightPaddleY = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
    ballX = SCREEN_WIDTH / 2;
    ballY = SCREEN_HEIGHT / 2;
    ballVelocityX = BALL_SPEED;
    ballVelocityY = BALL_SPEED;
    leftScore = rightScore = 0;
}

void initializeStars() {
    for (int i = 0; i < NUM_STARS; i++) {
        starX[i] = random(SCREEN_WIDTH);
        starY[i] = random(SCREEN_HEIGHT);
        starColors[i] = vga.rgb(random(128, 256), random(128, 256), random(128, 256));
    }
}

void updatePaddles() {
    // Left paddle movement
    if (leftPaddleUp && leftPaddleY > 0) {
        leftPaddleY -= PADDLE_SPEED;
    }
    if (leftPaddleDown && leftPaddleY < SCREEN_HEIGHT - PADDLE_HEIGHT) {
        leftPaddleY += PADDLE_SPEED;
    }

    // Right paddle movement
    if (singlePlayerMode) {
        // Simple AI for single player mode
        if (ballY < rightPaddleY + PADDLE_HEIGHT / 2) {
            rightPaddleY -= PADDLE_SPEED / 2;
        } else {
            rightPaddleY += PADDLE_SPEED / 2;
        }
        rightPaddleY = constrain(rightPaddleY, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);
    } else {
        // Two-player mode
        if (rightPaddleUp && rightPaddleY > 0) {
            rightPaddleY -= PADDLE_SPEED;
        }
        if (rightPaddleDown && rightPaddleY < SCREEN_HEIGHT - PADDLE_HEIGHT) {
            rightPaddleY += PADDLE_SPEED;
        }
    }
}

void updateBall() {
    ballX += ballVelocityX;
    ballY += ballVelocityY;

    // Collision with top and bottom walls
    if (ballY <= 0 || ballY >= SCREEN_HEIGHT - BALL_SIZE) {
        ballVelocityY = -ballVelocityY;
        playSound(440, 50);  // Play a bounce sound
    }

    // Collision with paddles
    if ((ballX <= PADDLE_WIDTH && ballY + BALL_SIZE >= leftPaddleY && ballY <= leftPaddleY + PADDLE_HEIGHT) ||
        (ballX >= SCREEN_WIDTH - PADDLE_WIDTH - BALL_SIZE && ballY + BALL_SIZE >= rightPaddleY && ballY <= rightPaddleY + PADDLE_HEIGHT)) {
        ballVelocityX = -ballVelocityX;
        playSound(660, 50);  // Play a different bounce sound for paddles
    }

    // Score points and reset ball if it goes out of bounds
    if (ballX < 0) {
        rightScore++;
        playSound(880, 200);  // Play a scoring sound
        ballX = SCREEN_WIDTH / 2;
        ballY = SCREEN_HEIGHT / 2;
    } else if (ballX > SCREEN_WIDTH) {
        leftScore++;
        playSound(880, 200);  // Play a scoring sound
        ballX = SCREEN_WIDTH / 2;
        ballY = SCREEN_HEIGHT / 2;
    }
}

void updateStars() {
    for (int i = 0; i < NUM_STARS; i++) {
        starX[i] -= 1;  // Move stars to the left
        if (starX[i] < 0) {
            starX[i] = SCREEN_WIDTH;
            starY[i] = random(SCREEN_HEIGHT);
        }
    }
}

void drawGame() {
    gfx.fillScreen(COLOR_BACKGROUND);
    drawStars();

    // Draw center line
    for (int y = 0; y < SCREEN_HEIGHT; y += 10) {
        gfx.drawFastVLine(SCREEN_WIDTH / 2, y, 5, COLOR_PADDLE);
    }

    // Draw paddles
    gfx.fillRect(0, leftPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT, COLOR_PADDLE);
    gfx.fillRect(SCREEN_WIDTH - PADDLE_WIDTH, rightPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT, COLOR_PADDLE);

    // Draw ball with a glowing effect
    gfx.fillCircle(ballX, ballY, BALL_SIZE, COLOR_BALL);
    gfx.drawCircle(ballX, ballY, BALL_SIZE + 1, vga.rgb(255, 128, 0));
    gfx.drawCircle(ballX, ballY, BALL_SIZE + 2, vga.rgb(255, 64, 0));

    // Draw scores
    gfx.setTextColor(COLOR_TEXT);
    gfx.setTextSize(2);
    gfx.setCursor(SCREEN_WIDTH / 4, 10);
    gfx.print(leftScore);
    gfx.setCursor(3 * SCREEN_WIDTH / 4, 10);
    gfx.print(rightScore);

    vga.show();
}

void drawStars() {
    for (int i = 0; i < NUM_STARS; i++) {
        gfx.drawPixel(starX[i], starY[i], starColors[i]);
    }
}

void playSound(int frequency, int duration) {
    ledcWriteTone(SOUND_CHANNEL, frequency);
    delay(duration);
    ledcWrite(SOUND_CHANNEL, 0);
}

void playOdeToJoy() {
    int noteDuration;
    for (int i = 0; i < melodySize; i++) {
        noteDuration = 1000 / noteDurations[i];
        playSound(melody[i], noteDuration);
        delay(noteDuration * 1.3);  // Add a small pause between notes
    }
}

void displayWinMessage(int winner) {
    gfx.fillScreen(COLOR_BACKGROUND);
    drawStars();
    gfx.setTextColor(COLOR_TEXT);
    gfx.setTextSize(3);
    gfx.setCursor(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20);
    gfx.print("Player ");
    gfx.print(winner);
    gfx.print(" Wins!");
    vga.show();
}