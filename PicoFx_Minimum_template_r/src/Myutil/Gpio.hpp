#pragma once
#include <Arduino.h>
//#include "pico/stdlib.h"

struct Gpio
{
  uint number;
  bool out, state = false;

  Gpio(uint number_, bool out_ = true) : number(number_), out(out_)
  {
    pinMode(number, out ? OUTPUT : INPUT);
  }

  void __not_in_flash_func(set)(bool x)
  {
    state = x;
    gpio_put(number, state);
  }

  bool __not_in_flash_func(get)() { return gpio_get(number); }

  void __not_in_flash_func(toggle)()
  {
    state = !state;
    set(state);
  }
};
