#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MAKE_COLOR(R, G, B) (R << 16 | G << 8 | B)
const uint32_t COLOR_WHITE = MAKE_COLOR(25, 25, 25);
const uint32_t COLOR_BLACK = MAKE_COLOR(0, 0, 0);
const uint32_t COLOR_RED = MAKE_COLOR(30, 0, 0);
const uint32_t COLOR_GREEN = MAKE_COLOR(0, 30, 0);
const uint32_t COLOR_BLUE = MAKE_COLOR(0, 0, 30);
const uint32_t COLOR_YELLOW = MAKE_COLOR(30, 30, 0);
const uint32_t COLOR_MAGENTA = MAKE_COLOR(30, 0, 30);
const uint32_t COLOR_CYAN = MAKE_COLOR(0, 30, 30);

struct StateLed
{
  Adafruit_NeoPixel led;
  //
  // PL9832 = NEO_RGB
  // WS2812 = NEO_GRB
  //
  StateLed(int led_pin) : led(1, led_pin, NEO_RGB + NEO_KHZ800)
  {
  }

  void begin()
  {
    led.begin();
    send(0);
  }

  void send(uint32_t color)
  {
    led.setPixelColor(0, color);
    led.show();
  }
};