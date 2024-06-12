#include <Arduino.h>
#include "quantendo.h"

#define WIDTH 20
#define HEIGHT 10

quantendo my_quantendo;
unsigned long last_ball_update = millis();
unsigned long last_s_update = millis();
uint8_t count;
int8_t player = 4;
uint8_t ball_x = 4;
uint8_t ball_y = 1;
int8_t bounce = -1;
uint8_t ball_dir = 0;
bool hit_brick = 0;
uint8_t game_state = 0;
uint8_t s_counter = 3;

// Define the digit patterns (5x3 for simplicity)
const int digits[10][5][3] = {
  { {1, 1, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}, {1, 1, 1} },  // 0
  { {0, 1, 0}, {1, 1, 0}, {0, 1, 0}, {0, 1, 0}, {1, 1, 1} },  // 1
  { {1, 1, 1}, {0, 0, 1}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1} },  // 2
  { {1, 1, 1}, {0, 0, 1}, {1, 1, 1}, {0, 0, 1}, {1, 1, 1} },  // 3
  { {1, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 0, 1}, {0, 0, 1} },  // 4
  { {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {0, 0, 1}, {1, 1, 1} },  // 5
  { {1, 1, 1}, {1, 0, 0}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1} },  // 6
  { {1, 1, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1} },  // 7
  { {1, 1, 1}, {1, 0, 1}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1} },  // 8
  { {1, 1, 1}, {1, 0, 1}, {1, 1, 1}, {0, 0, 1}, {1, 1, 1} }   // 9
};


void draw_gui(){
  my_quantendo.rectangle(0, 0, 9, 19, 127, 127, 127);
  my_quantendo.line(1, 0, 8, 0, 0, 0, 0);
}

void draw_level(){
  my_quantendo.box(2, 17, 7, 14, 0, 127, 0);
}

void draw_player(){
  my_quantendo.line(player - 1, 0, player + 1, 0, 127, 0, 127);
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  Serial.println("QuanTendo - Bricker");

  my_quantendo = quantendo();
  my_quantendo.begin();

  Serial.print("Dev mode:");
  Serial.println(my_quantendo.isDev() ? "true" : "false");
  Serial.println(my_quantendo.getNeoPin());

}

void dpln(String msg){
  // Prints to serial if dev mode
  if (my_quantendo.isDev()){
    Serial.println(msg);
  }
}

void draw_digit(uint8_t x, uint8_t y, uint8_t digit, uint8_t r, uint8_t g, uint8_t b){
  // Draws a digit at x, y with colour r, g, b
  my_quantendo.rectangle(x - 1, y - 1, x + 4, y + 6, r, g, b);
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 3; j++) {
      if (digits[digit][i][j] == 1) {
        my_quantendo.setPixel(x + j, y - i, r, g, b);
      }
    }
  }
}

void ball_collision(){
  // Detect collisions when moving in a `dir`
  // 0 = north 
  // 1 = north-east
  // 2 = east, unlikely
  // 3 = south-east
  // 4 = south
  // 5 = south-west
  // 6 = west, unlikely
  // 7 = north-west
  switch(ball_dir){
    case 0: // North
      if (my_quantendo.getPixel(ball_x, ball_y + 1) != 0 ){
        // hit_brick = 1;
        bounce = ball_dir; // Return
      }
      break;
    case 1: // North East
      if (my_quantendo.getPixel(ball_x + 1, ball_y + 1) != 0){
        bounce = ball_dir;
      }
    break;
    case 2: // East
      if (my_quantendo.getPixel(ball_x + 1, ball_y) != 0){
        bounce = ball_dir;
      }
      break;
    case 3: // South East
      if (my_quantendo.getPixel(ball_x + 1, ball_y - 1) != 0){
        bounce = ball_dir;
      }
      break;
    case 4: // North
      if (my_quantendo.getPixel(ball_x, ball_y - 1) != 0 ){
        bounce = ball_dir + (ball_x - player); // Can only be on the bat so bounce depending on the bat side
      }
      break;
    case 5: // South West
      if (my_quantendo.getPixel(ball_x - 1, ball_y - 1) != 0){
        bounce = ball_dir;
      }
      break;
    case 6: // West
      if (my_quantendo.getPixel(ball_x - 1, ball_y) != 0){
        bounce = ball_dir;
      }
      break;
    case 7: // North West
      if (my_quantendo.getPixel(ball_x - 1, ball_y + 1) != 0){
        bounce = ball_dir;
      }
      break;
  }
  if (ball_y == 0){
    // ball below bat
    game_state = 2;
  }
}

void check_bounce(){
  if (bounce != -1){
    Serial.println(bounce);
    switch(bounce){
      case 0: // North
        ball_dir = 4; // South
        break;
      case 1: // North East
        ball_dir = 7; // North East
        break;
      case 2: // East, wont happen but best add it!
        ball_dir = 6; // West
        break;
      case 3: // South East
        ball_dir = 1; // North East
        break;
      case 4: // South
        ball_dir = 0;  // North
        break;
      case 5: // South West
        ball_dir = 1; // North West
        break;
      case 6: // West, if we end up here there is a problem
        ball_dir = 2; // East
      case 7: // North West
        ball_dir = 1; // North East
        break;
    }
  }
  bounce = -1;
}

void move_ball(){
  // Move ball in direction
  switch(ball_dir){
    case 0:
      ball_y = ball_y + 1;
      break;
    case 1:
      ball_y = ball_y + 1;
      ball_x = ball_x + 1;
      break;
    case 4:
      ball_y = ball_y - 1;
      break;
    case 7:
      ball_y = ball_y + 1;
      ball_x = ball_x - 1;
      break;
  }
  // Check for bounce and collisions
  ball_collision();
  check_bounce();
  last_ball_update = millis();
}
 
void draw_ball(){
  // Render
  my_quantendo.setPixel(ball_x, ball_y, 127, 0, 0);
}

void button_trigger(){
switch(game_state){
  case 1:
    // Left Button
    if (my_quantendo.hasPressed(BTN_LEFT)) {
          if (player > 2) {
            player--;
          }
        }

    // Right Button
    if (my_quantendo.hasPressed(BTN_RIGHT)) {
      if (player < 7) {
        player++;
      }
    }

    // Fire buttons
    if (my_quantendo.hasPressed(BTN_RED) || my_quantendo.hasPressed(BTN_YEL) || my_quantendo.hasPressed(BTN_GRN) || my_quantendo.hasPressed(BTN_BLU)){
      Serial.println("FIRE!!!!");
    }
    break;
  case 2:
    delay(500);
    restart_game();
    break;
}
}
void clear(){
  my_quantendo.box(0, 0, 9, 19, 0, 0, 0);
}

void restart_game(){
  last_ball_update = millis();
  last_s_update = millis();
  player = 4;
  ball_x = 4;
  ball_y = 1;
  bounce = -1;
  ball_dir = 0;
  hit_brick = 0;
  s_counter = 3;
  game_state = 0;
}

void loop() {
  clear();
  switch(game_state){
    case 0:
      // Starting
      if (millis() > last_s_update + 250){
        s_counter--;
        last_s_update = millis();
      }
      if (s_counter == 0){
        game_state = 1;
      }
      draw_digit(4, 7, s_counter, 0,0,200);
      break;

    case 1:
      //  Running
      count = my_quantendo.readButtons();
      if (count > 0) {
        button_trigger();
      }
      draw_gui();
      draw_level();
      draw_player();
      if (millis() > last_ball_update + 250){
        move_ball();
      }
      draw_ball();
      break;
    case 2:
      // Dead
      count = my_quantendo.readButtons();
      if (count > 0) {
        button_trigger();
      }
      my_quantendo.line(0, 9, 9, 0, 255, 0, 0);
      my_quantendo.line(9, 9, 0, 0, 255, 0, 0);
  }
  
  my_quantendo.show();
  delay(2);
}
