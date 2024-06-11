#include "quantendo.h"

quantendo::quantendo() {
}

void quantendo::begin(void) {
  pinMode(DEV_PIN, INPUT_PULLUP);
  m_dev = !digitalRead(DEV_PIN);

  for (uint8_t i = 0; i < BUTTON::COUNT; i++) {
    buttons[i].begin(btn_pins[i]);
  }

  pixels.setPin(NEO_PIN);
  pixels.updateType(NEO_GRB + NEO_KHZ800);
  pixels.updateLength(WIDTH * HEIGHT);
  pixels.begin();
  pixels.clear();
  pixels.setBrightness(m_dev ? BRIGHT_DEV : BRIGHT_NORM);
  pixels.show();
}

void quantendo::show(void) {
  pixels.show();
}

bool quantendo::isDev(void) {
  return m_dev;
}

int16_t quantendo::getNeoPin(void) {
  return NEO_PIN;
}

bool quantendo::setPixel(int8_t x, int8_t y, uint8_t r, uint8_t g, uint8_t b) {
  if (x < WIDTH && y < HEIGHT) {
    pixels.setPixelColor(toPixel(x, y), r, g, b);
    return 0;
  } else {
    return 1;
  }
}

uint32_t quantendo::getPixel(int8_t x, int8_t y) {
  uint32_t ret = pixels.getPixelColor(toPixel(x, y));
  return ret;
}

uint32_t quantendo::colour(uint8_t r, uint8_t g, uint8_t b) {
  return pixels.Color(r, g, b);
}



bool quantendo::line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, uint8_t g, uint8_t b) {
  if (x0 < WIDTH && x1 < WIDTH && y0 < HEIGHT && y1 < HEIGHT) {
    int8_t dx = abs(x1 - x0);
    int8_t dy = abs(y1 - y0);
    int8_t sx = (x0 < x1) ? 1 : -1;
    int8_t sy = (y0 < y1) ? 1 : -1;

    int8_t err = dx - dy;

    while (true) {
      pixels.setPixelColor(toPixel(x0, y0), r, g, b);
      if (x0 == x1 && y0 == y1) {
        break;
      }

      int8_t e2 = 2 * err;
      if (e2 > -dy) {
        err -= dy;
        x0 += sx;
      }

      if (e2 < dx) {
        err += dx;
        y0 += sy;
      }
    }
    return 0;
  } else {
    return 1;
  }
}

bool quantendo::rectangle(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint8_t r, uint8_t g, uint8_t b) {
  if (x0 < WIDTH && x1 < WIDTH && y0 < HEIGHT && y1 < HEIGHT) {
    line(x0, y0, x1, y0, r, g, b);
    line(x1, y0, x1, y1, r, g, b);
    line(x1, y1, x0, y1, r, g, b);
    line(x0, y1, x0, y0, r, g, b);

    return 0;
  } else {
    return 1;
  }
}

bool quantendo::box(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint8_t r, uint8_t g, uint8_t b) {
  if (x0 < WIDTH && x1 < WIDTH && y0 < HEIGHT && y1 < HEIGHT) {
    int8_t s = x0 < x1 ? x0 : x1;
    int8_t e = x0 < x1 ? x1 : x0;

    for (int8_t x = s; x <= e; x++) {
      line(x, y0, x, y1, r, g, b);
    }

    return 0;
  } else {
    return 1;
  }
}

uint8_t quantendo::readButtons(void) {
  // returns the number of buttons that have changed since last read
  uint8_t count = 0;
  for (uint8_t i = 0; i < BUTTON::COUNT; i++) {
    if (buttons[i].buttonRead()) {
      count++;
    }
  }
  return count;
}

bool quantendo::isPressed(BUTTON btn) {
  return buttons[btn].isPressed();
}

bool quantendo::isReleased(BUTTON btn) {
  return buttons[btn].isReleased();
}

bool quantendo::hasPressed(BUTTON btn) {
  return buttons[btn].hasPressed();
}

bool quantendo::hasReleased(BUTTON btn) {
  return buttons[btn].hasReleased();
}

button* quantendo::read(BUTTON btn) {
  return &buttons[btn];
}
// ***********************

uint8_t quantendo::toPixel(uint8_t x, uint8_t y) {
  // (0,0) is bottom left
  // first pixel is bottom right
  //y = HEIGHT - y - 1;
  x = WIDTH - x - 1;

  return (WIDTH * y) + (x * -(((y % 2) * 2) - 1)) + ((WIDTH - 1) * (y % 2));
}
