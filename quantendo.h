#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define WIDTH 10
#define HEIGHT 20

#define START_PIN 2
#define SELECT_PIN 3
#define RIGHT_PIN 4
#define LEFT_PIN 5
#define DOWN_PIN 6
#define UP_PIN 7
#define RED_PIN A2
#define YEL_PIN A3
#define BLU_PIN A4
#define GRN_PIN A5
#define NEO_PIN 10
#define CTRLR_PIN 11
#define DEV_PIN 12

#define BRIGHT_NORM 255
#define BRIGHT_DEV 31
// 127=FE
//  63=FC
//  31=F8
//  15=F0
//   7=E0

enum BUTTON {
  BTN_START,
  BTN_SELECT,
  BTN_RIGHT,
  BTN_LEFT,
  BTN_DOWN,
  BTN_UP,
  BTN_RED,
  BTN_YEL,
  BTN_BLU,
  BTN_GRN,
  COUNT
};

struct button {
  bool buffer;
  bool state;
  bool isKnown;
  uint8_t pin;

  void begin(uint8_t button_pin) {
    isKnown = true;
    pin = button_pin;
    pinMode(pin, INPUT_PULLUP);
    state = !digitalRead(pin);
  }

  bool buttonRead(void) {
    if (state == digitalRead(pin)) {
      // this means that the button has been pressed or released
      // because digitalRead() is inverted, 0=on, 1=off
      // reverse the current level states to make them right
      state = !state;
      // flag that edge value has not been told
      isKnown = false;
      // return true to indicate a state change
      return true;
    } else {
      return false;
    }
  }

  bool isPressed(void) {
    return state;
  }

  bool hasPressed(void) {

    if (isKnown || !state) {
      // if we are known or the button is not pressed
      return false;
    } else {
      // mark as now known and report pressed
      isKnown = true;
      return isKnown;
    }
  }

  bool isReleased(void) {
    return !state;
  }

  bool hasReleased(void) {
    if (isKnown || state) {
      // if we are known already or the button is not released
      return false;
    } else {
      // mark as now known and report released
      isKnown = true;
      return isKnown;
    }
  }
};

class quantendo {
public:
  quantendo();

  void begin(void);
  void show(void);
  bool setPixel(int8_t x, int8_t y, uint8_t r, uint8_t g, uint8_t b);
  uint32_t getPixel(int8_t x, int8_t y);
  uint8_t getBrightness();
  uint32_t getOriginalColour(uint32_t scaledColour, uint8_t brightness);
  bool line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b);
  bool rectangle(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint8_t r, uint8_t g, uint8_t b);
  bool box(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint8_t r, uint8_t g, uint8_t b);
  uint8_t readButtons(void);
  bool hasPressed(BUTTON btn);
  bool hasReleased(BUTTON btn);
  bool isPressed(BUTTON btn);
  bool isReleased(BUTTON btn);
  bool isDev(void);
  int16_t getNeoPin(void);
  uint32_t colour(uint8_t r, uint8_t g, uint8_t b);

private:
  Adafruit_NeoPixel pixels;
  bool m_dev;
  button buttons[COUNT];
  button* read(BUTTON btn);
  uint8_t toPixel(uint8_t x, uint8_t y);
  uint8_t btn_pins[COUNT] = { START_PIN, SELECT_PIN, RIGHT_PIN, LEFT_PIN, DOWN_PIN, UP_PIN, RED_PIN, YEL_PIN, BLU_PIN, GRN_PIN };
};
