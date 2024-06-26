#include <Arduino.h>
#include "quantendo.h"

#define WIDTH 10
#define HEIGHT 20

quantendo my_quantendo;
unsigned long last_ball_update = millis();
unsigned long last_s_update = millis();
uint8_t count;
int ball_speed = 250;
int8_t player = 4;
uint8_t ball_x = 4;
uint8_t ball_y = 1;
int8_t bounce = -1;
int8_t ball_dx = 0;
int8_t ball_dy = 0;
bool hit_brick = 0;
uint8_t game_state = 0;
uint8_t s_counter = 3;
uint8_t brick;

	// - clear, 0
	// - ball, 1
	// - player, 2
	// - edge, 3
	// - wall, 4

// MAPS ARE UPSIDE DOWN!!
const uint8_t INITIAL_MAP_DATA[HEIGHT][WIDTH] = {
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,3,3,3,3,3,3,3,3,0},
    {0,0,3,3,3,3,3,3,0,0},
    {0,3,3,3,3,3,3,3,3,0},
    {0,0,3,3,3,3,3,3,0,0},
    {0,3,3,3,3,3,3,3,3,0},
    {0,0,3,3,3,3,3,3,0,0},
    {0,0,0,0,0,0,0,0,0,0}
  };

uint8_t map_data[HEIGHT][WIDTH];

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

void dpln(String msg){
  // Prints to serial if dev mode
  if (my_quantendo.isDev()){
    Serial.println((String)msg);
  }
}

void draw_gui(){
  my_quantendo.rectangle(0, 0, 9, 19, 127, 127, 127);
  my_quantendo.line(1, 0, 8, 0, 0, 0, 0);
}

uint32_t get_tile_colour(uint8_t t){
  switch(t){
    case 0:
      return my_quantendo.colour(0, 0, 0);
      break;
    case 1:
      return my_quantendo.colour(127, 0, 0);
      break;
    case 2:
      return my_quantendo.colour(127, 0, 127);
      break;
    case 3:
      return my_quantendo.colour(0, 127, 0);
      break;
    case 4:
      return my_quantendo.colour(127, 127, 127);
      break;
  }
  return my_quantendo.colour(0, 0, 0);
}

void draw_level(){
  uint8_t k;
  uint8_t l;
  for (k=0; k<HEIGHT; k++){
    for (l=0; l<WIDTH; l++){
      uint32_t t = get_tile_colour(map_data[k][l]);
      uint8_t r = (t >> 16) & 0xFF;
      uint8_t g = (t >> 8) & 0xFF;
      uint8_t b = t & 0xFF;
      my_quantendo.setPixel(l, k, r, g, b);
    }
  }
  // my_quantendo.box(2, 17, 7, 14, 0, 127, 0);
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
  Serial.println(my_quantendo.isDev() ? " True" : " False");
  Serial.println(my_quantendo.getNeoPin());

  // Initialize the map data
  memcpy(map_data, INITIAL_MAP_DATA, sizeof(INITIAL_MAP_DATA));

}

void draw_digit(uint8_t x, uint8_t y, uint8_t digit, uint8_t r, uint8_t g, uint8_t b){
  // Draws a digit at x, y with colour r, g, b
  my_quantendo.rectangle(x - 2, y + 2, x + 4, y - 6, r, g, b);
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 3; j++) {
      if (digits[digit][i][j] == 1) {
        my_quantendo.setPixel(x + j, y - i, r, g, b);
      }
    }
  }
}

uint8_t detect_wall(uint8_t x, uint8_t y){
  // Return walls around ball
  // Directions
  // [7][0][1]
  // [6][B][2]
  // [5][4][3]


}

bool check_bat_hit(){
  // check if we hit the bat and return true|false
  if ((ball_dy == - 1) && (ball_y == 1)){
    // Check if the balls moves down it will be inline with the bat
    switch(ball_dx){
      case -1:
        dpln("Ball moving left");
        if ((ball_x - 1 >= player - 1) && (ball_x - 1 <= player + 1)){
          return true;
        }
      case 0:
        dpln("Ball moving down");
        if ((ball_x >= player - 1) && (ball_x <= player + 1)){
          return true;
        }
      case 1:
        dpln("Ball moving right");
        if ((ball_x + 1 >= player - 1) && (ball_x + 1 <= player + 1)){
          return true;
        }
    }
  }
  // All other options return False
  return false;
}

void ball_collision(){
  if (check_bat_hit()){
    dpln("Hit bat");
    ball_dx = (ball_x - player); 
    ball_dy = 1;
    return;
  }
  check_bounce();
  if (ball_y == 0){
    // ball below bat
    game_state = 2;
  }
}

void remove_wall(uint8_t x, uint8_t y){
  dpln("Removing brick");
  map_data[y][x] = 0;
  ball_speed = ball_speed * 0.95;
  dpln((String)ball_speed);
}

void check_bounce() {
  bool bounced = false;
  uint8_t old_ball_dx = ball_dx;
  uint8_t old_ball_dy = ball_dy;

  // Check horizontal (x-axis) collision
  if (ball_dx != 0) {
    int next_x = ball_x + ball_dx;
    if (next_x < 0 || next_x >= WIDTH || map_data[ball_y][next_x] != 0) {
      ball_dx = -ball_dx;
      bounced = true;
      if (map_data[ball_y][next_x] != 0) {
        remove_wall(next_x, ball_y);
      }
    }
  }
  
  // Check vertical (y-axis) collision
  if (ball_dy != 0) {
    int next_y = ball_y + ball_dy;
    if (next_y < 0 || next_y >= HEIGHT || map_data[next_y][ball_x] != 0) {
      ball_dy = -ball_dy;
      bounced = true;
      if (map_data[next_y][ball_x] != 0) {
        remove_wall(ball_x, next_y);
      }
    }
  }
  // If a brick was hit, remove it
  if ((bounced == 1)  && (map_data[ball_y + old_ball_dy][ball_x + old_ball_dx] != 0)) {
    dpln("Bounced");
    dpln((String)map_data[ball_y + old_ball_dy][ball_x + old_ball_dx]);
    remove_wall(ball_x + old_ball_dx, ball_y + old_ball_dy);
  }
}

void move_ball() {
  // Check for bounce and collisions
  ball_collision();
  
  // Move ball in direction
  ball_x += ball_dx;
  ball_y += ball_dy;
  
  last_ball_update = millis();
}

 
void draw_ball(){
  // Render
  my_quantendo.setPixel(ball_x, ball_y, 127, 0, 0);
}



void button_trigger(){
  dpln((String)game_state);
  switch(game_state){
    case 1:
      // Left Button
      if (my_quantendo.hasPressed(BTN_LEFT)) {
        if (player > 1) {
          player--;
        }

      }

      // Right Button
      if (my_quantendo.hasPressed(BTN_RIGHT)) {
        if (player < 8) {
          player++;
        }
        
      }

      // Fire buttons
      if (my_quantendo.hasPressed(BTN_RED) || my_quantendo.hasPressed(BTN_YEL) || my_quantendo.hasPressed(BTN_GRN) || my_quantendo.hasPressed(BTN_BLU)){
          // detect_wall(ball_x, ball_y);
      }
      break;
    case 2:
      delay(500);
      restart_game();
      break;
    case 3:
          // Left Button
      if (my_quantendo.hasPressed(BTN_LEFT)) {
        if (player > 1) {
          player--;
          ball_x = player;
        }
      }
      // Right Button
      if (my_quantendo.hasPressed(BTN_RIGHT)) {
        if (player < 8) {
          player++;
          ball_x = player;
        }
      }
    if (my_quantendo.hasPressed(BTN_RED) || my_quantendo.hasPressed(BTN_YEL) || my_quantendo.hasPressed(BTN_GRN) || my_quantendo.hasPressed(BTN_BLU)){
      // Ball on bat
      if ((ball_dx == 0) && (ball_dy == 0)){
        ball_dy = 1;
        game_state = 1;
      }
    }
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
  // bounce = -1;
  ball_dx = 0;
  ball_dy = 0;
  hit_brick = 0;
  s_counter = 3;
  game_state = 0;

// Reset the map data
  memcpy(map_data, INITIAL_MAP_DATA, sizeof(INITIAL_MAP_DATA));
}

void draw_stats(){
  // char output[50];
  // sprintf(output, "Ball_X: %d, Ball_y: %d, Ball_dir: %d, Player: %d", ball_x, ball_y, ball_dir, player);
  // dpln(output);
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
        game_state = 3;
      }
      draw_digit(3, 7, s_counter, 0, 0, 200);
      break;
    case 1:
    case 3:
      //  Running
      count = my_quantendo.readButtons();
      if (count > 0) {
        button_trigger();
      }
      // draw_gui();
      draw_level();
      draw_player();
      if (millis() > last_ball_update + ball_speed){
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
