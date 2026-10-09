#pragma once
#include <Arduino.h>
#include "hardware/pwm.h"

#define GPIO_0_1 0, 1
#define GPIO_2_3 2, 3
#define GPIO_4_5 4, 5
#define GPIO_6_7 6, 7
#define GPIO_8_9 8, 9
#define GPIO_10_11 10, 11
#define GPIO_12_13 12, 13
#define GPIO_14_15 14, 15
#define GPIO_16_17 16, 17
#define GPIO_18_19 18, 19
#define GPIO_20_21 20, 21
#define GPIO_22_23 22, 23
#define GPIO_24_25 24, 25
#define GPIO_26_27 26, 27
#define GPIO_28_29 28, 29

struct DualPwmDac
{
  int pinHighL, pinLowL, pinHighR, pinLowR;
  uint sliceNumL, sliceNumR;
  volatile bool enable = true;

  DualPwmDac(int pin_high_l, int pin_low_l, int pin_high_r, int pin_low_r)
      : pinHighL(pin_high_l), pinLowL(pin_low_l),
        pinHighR(pin_high_r), pinLowR(pin_low_r) {}

  void begin()
  {
    //
    // L ch
    //
    gpio_set_function(pinHighL, GPIO_FUNC_PWM);
    gpio_set_function(pinLowL, GPIO_FUNC_PWM);
    sliceNumL = pwm_gpio_to_slice_num(pinHighL);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.f);
    pwm_config_set_wrap(&config, 255);
    pwm_init(sliceNumL, &config, true);
    //
    // R ch
    //
    gpio_set_function(pinHighR, GPIO_FUNC_PWM);
    gpio_set_function(pinLowR, GPIO_FUNC_PWM);
    sliceNumR = pwm_gpio_to_slice_num(pinHighR);
    pwm_config_set_clkdiv(&config, 1.f);
    pwm_config_set_wrap(&config, 255);
    pwm_init(sliceNumR, &config, true);
  }

  void __not_in_flash_func(setOutput)(int16_t L, int16_t R) // -32768 <= x <= 32767
  {
    if (enable)
    {
      uint16_t u = L + 32768;
      pwm_set_both_levels(sliceNumL, (u >> 8) & 0xFF, u & 0xFF);

      u = R + 32768;
      pwm_set_both_levels(sliceNumR, (u >> 8) & 0xFF, u & 0xFF);
    }
  }
};
