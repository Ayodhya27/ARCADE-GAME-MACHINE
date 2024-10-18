// #include "ESP32S3VGA.h"
// #include <GfxWrapper.h>
// #include <Fonts/FreeSans9pt7b.h>
// #include <Fonts/FreeSansBold12pt7b.h>
// #include "menu.h"

// // VGA setup
// const PinConfig pins = {-1,-1,-1,-1,1, -1,-1,-1,-1,-1,2, -1,-1,-1,-1,3, 10,11};
// VGA vga;
// Mode mode = Mode::MODE_320x240x60;
// GfxWrapper<VGA> gfx(vga, mode.hRes, mode.vRes);

// // Button pins
// const int BUTTON_PIN_1 = 0;  // GPIO 0 for Player 1
// const int BUTTON_PIN_2 = 14; // GPIO 14 for Player 2

// // Speaker pin
// const int SPEAKER_PIN = 18;

// // Screen dimensions
// const int SCREEN_WIDTH = 320;
// const int SCREEN_HEIGHT = 240;

// // Game states
// enum GameState {
//     MENU,
//     WAITING_TO_START,
//     GET_READY,
//     WAITING_FOR_PRESS,
//     SHOW_ROUND_RESULT,
//     SHOW_GAME_RESULT,
//     RESULT_DELAY,
//     SHOW_LEADERBOARD
// };

// GameState currentState = MENU;

// unsigned long gameStartTime = 0;
// unsigned long reactionTime[2] = {0, 0};
// unsigned long resultStartTime = 0;
// int currentRound = 0;
// const int TOTAL_ROUNDS = 3;
// int scores[2] = {0, 0};
// int winner = -1;
// bool falseStart[2] = {false, false};

// // Leaderboard
// struct LeaderboardEntry {
//     char name[4];
//     float avgScore;
// };

// const int LEADERBOARD_SIZE = 5;
// LeaderboardEntry leaderboard[LEADERBOARD_SIZE] = {
//     {"---", 0},
//     {"---", 0},
//     {"---", 0},
//     {"---", 0},
//     {"---", 0}
// };

// // Colors
// const uint16_t COLOR_BACKGROUND = vga.rgb(16, 16, 32);
// const uint16_t COLOR_TEXT = vga.rgb(255, 255, 255);
// const uint16_t COLOR_HIGHLIGHT = vga.rgb(255, 255, 0);
// const uint16_t COLOR_PLAYER_1 = vga.rgb(255, 64, 64);
// const uint16_t COLOR_PLAYER_2 = vga.rgb(64, 64, 255);

// // Sound setup
// const int SOUND_CHANNEL = 0;
// const int CLICK_FREQUENCY = 1000;
// const int CLICK_DURATION = 50;

// // Ode to Joy notes and durations
// #define NOTE_C4  262
// #define NOTE_D4  294
// #define NOTE_E4  330
// #define NOTE_F4  349
// #define NOTE_G4  392
// #define NOTE_A4  440
// #define NOTE_B4  494
// #define NOTE_C5  523
// #define NOTE_D5  587
// #define NOTE_E5  659



// int odeToJoyMelody[] = {
//     NOTE_E4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4,
//     NOTE_C4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4,
//     NOTE_E4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4,
//     NOTE_C4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_C4, NOTE_C4
// };

// int odeToJoyDurations[] = {
//     4, 4, 4, 4, 4, 4, 4, 4,
//     4, 4, 4, 4, 4, 4, 2,
//     4, 4, 4, 4, 4, 4, 4, 4,
//     4, 4, 4, 4, 4, 4, 2
// };

// const int tempo = 114; // Beats per minute
// volatile bool isPlaying = false;

// void playTone(int frequency, int duration) {
//     ledcWriteTone(SOUND_CHANNEL, frequency);
//     delay(duration);
//     ledcWrite(SOUND_CHANNEL, 0);
// }

// void playClickSound() {
//     playTone(CLICK_FREQUENCY, CLICK_DURATION);
// }

// void playOdeToJoy() {
//     isPlaying = true;
//     int wholenote = (60000 * 4) / tempo;

//     for (int thisNote = 0; thisNote < sizeof(odeToJoyMelody) / sizeof(odeToJoyMelody[0]); thisNote++) {
//         if (!isPlaying) break;
//         int noteDuration = wholenote / odeToJoyDurations[thisNote];
//         ledcWriteTone(SOUND_CHANNEL, odeToJoyMelody[thisNote]);

//         unsigned long startTime = millis();
//         while (millis() - startTime < noteDuration * 0.9) {
//             if (digitalRead(BUTTON_PIN_1) == LOW || digitalRead(BUTTON_PIN_2) == LOW) {
//                 isPlaying = false;
//                 break;
//             }
//         }

//         ledcWrite(SOUND_CHANNEL, 0); // Turn off tone
//         delay(noteDuration * 0.1); // Brief pause between notes
//     }

//     isPlaying = false;
// }

// void drawCenteredText(const char* text, int y, uint16_t color, const GFXfont* font = &FreeSans9pt7b) {
//     gfx.setFont(font);
//     gfx.setTextSize(1);
//     gfx.setTextColor(color);
    
//     int16_t x1, y1;
//     uint16_t w, h;
//     gfx.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    
//     int x = (SCREEN_WIDTH - w) / 2;
//     gfx.setCursor(x, y + h);
//     gfx.print(text);
// }

// void updateDisplay() {
//     gfx.fillScreen(COLOR_BACKGROUND);

//     int centerY = SCREEN_HEIGHT / 2;

//     switch (currentState) {
//         case WAITING_TO_START:
//             drawCenteredText("Reaction Time Game", 50, COLOR_HIGHLIGHT, &FreeSansBold12pt7b);
//             drawCenteredText("Press any button to start", centerY, COLOR_TEXT);
//             break;
//         case GET_READY:
//             drawCenteredText("Get ready...", centerY, COLOR_HIGHLIGHT, &FreeSansBold12pt7b);
//             break;
//         case WAITING_FOR_PRESS:
//             gfx.fillScreen(vga.rgb(0, 255, 0));  // Green screen
//             drawCenteredText("PRESS NOW!", centerY, vga.rgb(0, 0, 0), &FreeSansBold12pt7b);
//             break;
//         case SHOW_ROUND_RESULT:
//         case RESULT_DELAY:
//             char roundResult[50];
//             sprintf(roundResult, "Round %d Result:", currentRound);
//             drawCenteredText(roundResult, 40, COLOR_HIGHLIGHT, &FreeSansBold12pt7b);
            
//             for (int i = 0; i < 2; i++) {
//                 char playerResult[50];
//                 if (falseStart[i]) {
//                     sprintf(playerResult, "Player %d: False Start!", i + 1);
//                 } else {
//                     sprintf(playerResult, "Player %d: %lu ms", i + 1, reactionTime[i]);
//                 }
//                 drawCenteredText(playerResult, 80 + (i * 40), i == 0 ? COLOR_PLAYER_1 : COLOR_PLAYER_2);
//             }
            
//             char scoreText[50];
//             sprintf(scoreText, "Scores - P1: %d, P2: %d", scores[0], scores[1]);
//             drawCenteredText(scoreText, 160, COLOR_TEXT);
            
//             if (currentState == RESULT_DELAY) {
//                 drawCenteredText("Press any button for next round", 200, COLOR_HIGHLIGHT);
//             } else {
//                 drawCenteredText("Wait...", 200, COLOR_TEXT);
//             }
//             break;
//         case SHOW_GAME_RESULT:
//             drawCenteredText("Game Over!", 40, COLOR_HIGHLIGHT, &FreeSansBold12pt7b);
            
//             char finalScores[50];
//             sprintf(finalScores, "Final Scores - P1: %d, P2: %d", scores[0], scores[1]);
//             drawCenteredText(finalScores, 80, COLOR_TEXT);
            
//             char winnerText[50];
//             if (winner == 0) {
//                 sprintf(winnerText, "Player 1 Wins!");
//                 drawCenteredText(winnerText, 120, COLOR_PLAYER_1);
//             } else if (winner == 1) {
//                 sprintf(winnerText, "Player 2 Wins!");
//                 drawCenteredText(winnerText, 120, COLOR_PLAYER_2);
//             } else {
//                 sprintf(winnerText, "It's a Tie!");
//                 drawCenteredText(winnerText, 120, COLOR_HIGHLIGHT);
//             }
            
//             drawCenteredText("Press any button to see leaderboard", 160, COLOR_TEXT);
//             break;
//         case SHOW_LEADERBOARD:
//             drawCenteredText("Leaderboard", 30, COLOR_HIGHLIGHT, &FreeSansBold12pt7b);
//             for (int i = 0; i < LEADERBOARD_SIZE; i++) {
//                 char entry[50];
//                 sprintf(entry, "%d. %s: %.2f", i + 1, leaderboard[i].name, leaderboard[i].avgScore);
//                 drawCenteredText(entry, 70 + (i * 30), COLOR_TEXT);
//             }
//             drawCenteredText("Press any button to restart", 220, COLOR_HIGHLIGHT);

//             // Start Ode to Joy melody once leaderboard is displayed
//             if (!isPlaying) {
//                 playOdeToJoy();
//             }
//             break;
//     }

//     vga.show();
// }

// void updateLeaderboard(int player, float avgScore) {
//     // Find the position to insert the new score
//     int insertPos = LEADERBOARD_SIZE;
//     for (int i = 0; i < LEADERBOARD_SIZE; i++) {
//         if (avgScore > leaderboard[i].avgScore) {
//             insertPos = i;
//             break;
//         }
//     }

//     // Shift lower scores down
//     for (int i = LEADERBOARD_SIZE - 1; i > insertPos; i--) {
//         leaderboard[i] = leaderboard[i - 1];
//     }

//     // Insert the new score
//     if (insertPos < LEADERBOARD_SIZE) {
//         sprintf(leaderboard[insertPos].name, "P%d", player + 1);
//         leaderboard[insertPos].avgScore = avgScore;
//     }
// }

// void setup() {
//     Menu gameMenu(vga, gfx);
//     vga.bufferCount = 2;
//     if(!vga.init(pins, mode, 8)) while(1) delay(1);  // 8-bit color depth
//     vga.start();

//     pinMode(BUTTON_PIN_1, INPUT_PULLUP);
//     pinMode(BUTTON_PIN_2, INPUT_PULLUP);

//     // Setup speaker
//     ledcSetup(SOUND_CHANNEL, 5000, 8);
//     ledcAttachPin(SPEAKER_PIN, SOUND_CHANNEL);

//     updateDisplay();
// }

// void startNewRound() {
//     currentState = GET_READY;
//     updateDisplay();
//     delay(random(2000, 5000));  // Random delay between 2 to 5 seconds
//     currentState = WAITING_FOR_PRESS;
//     gameStartTime = micros();
//     reactionTime[0] = reactionTime[1] = 0;
//     falseStart[0] = falseStart[1] = false;
//     updateDisplay();
// }

// void determineWinner() {
//     if (scores[0] > scores[1]) {
//         winner = 0;
//     } else if (scores[1] > scores[0]) {
//         winner = 1;
//     } else {
//         winner = -1;  // Tie
//     }
// }

// void loop() {
    
//    switch (currentState) {
//         case MENU:
//             {
//                 Menu gameMenu(vga, gfx, COLOR_HIGHLIGHT);  // Use COLOR_HIGHLIGHT for menu highlight
//                 MenuOption choice = gameMenu.show();
//                 switch (choice) {
//                     case PLAY_GAME:
//                         currentRound = 1;
//                         scores[0] = scores[1] = 0;
//                         currentState = WAITING_TO_START;
//                         break;
//                     case VIEW_LEADERBOARD:
//                         currentState = SHOW_LEADERBOARD;
//                         break;
//                     case QUIT_GAME:
//                         // Implement quit functionality if needed
//                         break;
//                 }
//                 updateDisplay();
//             }
//             break;

//         case WAITING_TO_START:
//             if (digitalRead(BUTTON_PIN_1) == LOW || digitalRead(BUTTON_PIN_2) == LOW) {
//                 playClickSound();
//                 startNewRound();
//             }
//             break;


//         case GET_READY:
//             if (digitalRead(BUTTON_PIN_1) == LOW) {
//                 playClickSound();
//                 falseStart[0] = true;
//             }
//             if (digitalRead(BUTTON_PIN_2) == LOW) {
//                 playClickSound();
//                 falseStart[1] = true;
//             }
//             if (falseStart[0] || falseStart[1]) {
//                 currentState = SHOW_ROUND_RESULT;
//                 resultStartTime = millis();
//                 updateDisplay();
//             }
//             break;

//         case WAITING_FOR_PRESS:
//             if (digitalRead(BUTTON_PIN_1) == LOW && reactionTime[0] == 0) {
//                 playClickSound();
//                 reactionTime[0] = (micros() - gameStartTime) / 1000;
//             }
//             if (digitalRead(BUTTON_PIN_2) == LOW && reactionTime[1] == 0) {
//                 playClickSound();
//                 reactionTime[1] = (micros() - gameStartTime) / 1000;
//             }
//             if (reactionTime[0] != 0 && reactionTime[1] != 0) {
//                 if (!falseStart[0] && !falseStart[1]) {
//                     if (reactionTime[0] < reactionTime[1]) {
//                         scores[0]++;
//                     } else if (reactionTime[1] < reactionTime[0]) {
//                         scores[1]++;
//                     }
//                 } else if (falseStart[0] && !falseStart[1]) {
//                     scores[1]++;
//                 } else if (!falseStart[0] && falseStart[1]) {
//                     scores[0]++;
//                 }
//                 currentState = SHOW_ROUND_RESULT;
//                 resultStartTime = millis();
//                 updateDisplay();
//             }
//             break;

//         case SHOW_ROUND_RESULT:
//             if (millis() - resultStartTime >= 5000) {
//                 currentState = RESULT_DELAY;
//                 updateDisplay();
//             }
//             break;

//         case RESULT_DELAY:
//             if (digitalRead(BUTTON_PIN_1) == LOW || digitalRead(BUTTON_PIN_2) == LOW) {
//                 playClickSound();
//                 delay(200);  // Debounce
//                 if (currentRound < TOTAL_ROUNDS) {
//                     currentRound++;
//                     startNewRound();
//                 } else {
//                     currentState = SHOW_GAME_RESULT;
//                     determineWinner();
//                     // Update leaderboard
//                     updateLeaderboard(0, (float)scores[0] / TOTAL_ROUNDS);
//                     updateLeaderboard(1, (float)scores[1] / TOTAL_ROUNDS);
//                     updateDisplay();
//                 }
//             }
//             break;

//         case SHOW_GAME_RESULT:
//             if (digitalRead(BUTTON_PIN_1) == LOW || digitalRead(BUTTON_PIN_2) == LOW) {
//                 playClickSound();
//                 delay(200);  // Debounce
//                 currentState = SHOW_LEADERBOARD;
//                 updateDisplay();
//             }
//             break;

//         case SHOW_LEADERBOARD:
//             // Stop the song if the player presses a button
//             if (digitalRead(BUTTON_PIN_1) == LOW || digitalRead(BUTTON_PIN_2) == LOW) {
//                 playClickSound();
//                 isPlaying = false; // Stop the song
//                 delay(200);  // Debounce
//                 currentState = MENU;  // Go back to menu
//                 updateDisplay();
//             }
//             break;

//         default:
//             break;
//     }
// }