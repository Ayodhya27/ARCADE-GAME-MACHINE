// #include <esp_now.h>
// #include <WiFi.h>
// #include <TFT_eSPI.h>

// // Create an instance of the display
// TFT_eSPI tft = TFT_eSPI();

// // Define a data structure for the incoming message
// typedef struct struct_message {
//   int command;  // Command from the controller
// } struct_message;

// struct_message myData;

// // Define some constants for commands
// #define COMMAND_UP 1
// #define COMMAND_DOWN 2
// #define COMMAND_SELECT 3
// #define COMMAND_P2_UP 4  // Command for Player 2 Up
// #define COMMAND_P2_DOWN 5 // Command for Player 2 Down

// // Menu options
// String menuItems[] = {"Pong 1 Player", "Pong 2 Player", "Settings"};
// int menuIndex = 0;
// int totalItems = sizeof(menuItems) / sizeof(menuItems[0]);

// // Pong game variables
// int player1Y = 60;
// int player2Y = 60;
// int ballX = 160;
// int ballY = 120;
// int ballSpeedX = 5;
// int ballSpeedY = 5;

// // New color definitions
// #define BACKGROUND_COLOR TFT_NAVY
// #define PADDLE_COLOR TFT_CYAN
// #define BALL_COLOR TFT_YELLOW
// #define SCORE_COLOR TFT_WHITE
// #define LINE_COLOR TFT_DARKGREY

// // New paddle dimensions
// const int paddleHeight = 50;
// const int paddleWidth = 8;
// const int paddleRadius = 4;
// const int paddleSpeed = 10;

// // New ball dimensions
// const int ballSize = 8;

// // Scoring variables
// int player1Score = 0;
// int player2Score = 0;

// // Sound pin
// #define SOUND_PIN 18

// // Notes for the victory tune
// #define NOTE_C5 523
// #define NOTE_E5 659
// #define NOTE_G5 784

// // Game state tracking
// bool inMenu = true;
// bool inPongGame = false;

// // Function declarations
// void displayMenu();
// void resetPongGame();
// void updateGame();
// void drawGame();
// void resetBall();
// void handleMenuInput();
// void handleGameInput();
// void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
// void playBounceSound();
// void playVictoryTune();
// void playTone(int frequency, int duration);

// void setup() {
//   Serial.begin(115200);

//   tft.init();
//   tft.setRotation(1);
//   tft.fillScreen(BACKGROUND_COLOR);

//   // Initialize PWM for sound output on GPIO 18
//   ledcSetup(0, 2000, 8);  // Channel 0, 2kHz base frequency, 8-bit resolution
//   ledcAttachPin(SOUND_PIN, 0);  // Attach pin 18 to channel 0

//   displayMenu();

//   WiFi.mode(WIFI_STA);

//   if (esp_now_init() != ESP_OK) {
//     Serial.println("Error initializing ESP-NOW");
//     return;
//   }

//   esp_now_register_recv_cb(OnDataRecv);
// }

// void loop() {
//   if (inPongGame) {
//     updateGame();
//     drawGame();
//     delay(30); // Control frame rate
//   }
// }

// void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
//   memcpy(&myData, incomingData, sizeof(myData));

//   if (inMenu) {
//     handleMenuInput();
//   } else if (inPongGame) {
//     handleGameInput();
//   }
// }

// void handleMenuInput() {
//   if (myData.command == COMMAND_UP) {
//     menuIndex = (menuIndex - 1 + totalItems) % totalItems;
//   } else if (myData.command == COMMAND_DOWN) {
//     menuIndex = (menuIndex + 1) % totalItems;
//   } else if (myData.command == COMMAND_SELECT) {
//     if (menuIndex == 1) { // "Pong 2 Player" selected
//       inMenu = false;
//       inPongGame = true;
//       tft.fillScreen(BACKGROUND_COLOR);
//       resetPongGame();
//     } else {
//       tft.fillScreen(BACKGROUND_COLOR);
//       tft.setCursor(0, 30);
//       tft.setTextSize(2);
//       tft.setTextColor(TFT_GREEN);
//       tft.print("Selected: ");
//       tft.println(menuItems[menuIndex]);
//       delay(2000);
//     }
//   }
//   displayMenu();
// }

// void handleGameInput() {
//   if (myData.command == COMMAND_UP) {
//     player1Y = max(0, player1Y - paddleSpeed);
//   } else if (myData.command == COMMAND_DOWN) {
//     player1Y = min(tft.height() - paddleHeight, player1Y + paddleSpeed);
//   } else if (myData.command == COMMAND_P2_UP) {
//     player2Y = max(0, player2Y - paddleSpeed);
//   } else if (myData.command == COMMAND_P2_DOWN) {
//     player2Y = min(tft.height() - paddleHeight, player2Y + paddleSpeed);
//   }
// }

// void displayMenu() {
//   if (inMenu) {
//     tft.fillScreen(BACKGROUND_COLOR);
//     tft.setTextSize(2);
//     for (int i = 0; i < totalItems; i++) {
//       tft.setTextColor(i == menuIndex ? TFT_YELLOW : TFT_WHITE);
//       tft.setCursor(10, 40 + i * 30);
//       tft.println(menuItems[i]);
//     }
//   }
// }

// void resetPongGame() {
//   player1Score = 0;
//   player2Score = 0;
//   ballX = tft.width() / 2;
//   ballY = tft.height() / 2;
//   player1Y = player2Y = tft.height() / 2 - paddleHeight / 2;
// }

// void updateGame() {
//   ballX += ballSpeedX;
//   ballY += ballSpeedY;

//   if (ballY <= 0 || ballY >= tft.height() - ballSize) {
//     ballSpeedY = -ballSpeedY;
//     playBounceSound();  // Play sound on bounce
//   }

//   if ((ballX <= paddleWidth && ballY + ballSize >= player1Y && ballY <= player1Y + paddleHeight) ||
//       (ballX >= tft.width() - paddleWidth - ballSize && ballY + ballSize >= player2Y && ballY <= player2Y + paddleHeight)) {
//     ballSpeedX = -ballSpeedX;
//     playBounceSound();  // Play sound on paddle hit
//   }

//   if (ballX < 0) {
//     player2Score++;
//     playVictoryTune();  // Play victory tune when Player 2 scores
//     resetBall();
//   }
//   if (ballX > tft.width() - ballSize) {
//     player1Score++;
//     playVictoryTune();  // Play victory tune when Player 1 scores
//     resetBall();
//   }
// }

// void resetBall() {
//   ballX = tft.width() / 2;
//   ballY = tft.height() / 2;
//   ballSpeedX = ballSpeedX > 0 ? 5 : -5;
//   ballSpeedY = ballSpeedY > 0 ? 5 : -5;
// }

// void drawGame() {
//   tft.fillScreen(BACKGROUND_COLOR);

//   // Draw center line
//   for (int i = 0; i < tft.height(); i += 10) {
//     tft.fillRect(tft.width() / 2 - 1, i, 2, 5, LINE_COLOR);
//   }

//   // Draw paddles with rounded corners
//   tft.fillRoundRect(0, player1Y, paddleWidth, paddleHeight, paddleRadius, PADDLE_COLOR);
//   tft.fillRoundRect(tft.width() - paddleWidth, player2Y, paddleWidth, paddleHeight, paddleRadius, PADDLE_COLOR);

//   // Draw the ball with a glowing effect
//   tft.fillCircle(ballX, ballY, ballSize / 2, BALL_COLOR);
//   tft.drawCircle(ballX, ballY, ballSize / 2 + 1, TFT_ORANGE);
//   tft.drawCircle(ballX, ballY, ballSize / 2 + 2, TFT_RED);

//   // Draw scores with a shadow effect
//   tft.setTextSize(3);
//   tft.setTextColor(TFT_DARKGREY);
//   tft.setCursor(tft.width() / 4 - 10, 11);
//   tft.print(player1Score);
//   tft.setCursor(3 * tft.width() / 4 - 10, 11);
//   tft.print(player2Score);

//   tft.setTextColor(SCORE_COLOR);
//   tft.setCursor(tft.width() / 4 - 10, 10);
//   tft.print(player1Score);
//   tft.setCursor(3 * tft.width() / 4 - 10, 10);
//   tft.print(player2Score);

//   // Draw court boundaries
//   tft.drawRect(0, 0, tft.width(), tft.height(), LINE_COLOR);
// }

// // Audio functions
// void playBounceSound() {
//   playTone(1000, 50); // Play a 1kHz tone for 50ms
// }

// void playVictoryTune() {
//   playTone(NOTE_C5, 200);
//   playTone(NOTE_E5, 200);
//   playTone(NOTE_G5, 400);
// }

// void playTone(int frequency, int duration) {
//   ledcWriteTone(0, frequency); // Set PWM frequency on channel 0 (SOUND_PIN)
//   delay(duration);             // Play tone for the specified duration
//   ledcWriteTone(0, 0);         // Turn off the tone (0 Hz)
// }
