#pragma once
#include <cstdint>
#include "DataSlot.hpp"
#include "DataQueue.hpp"
//
// ADCをオーバークロックしてシステムクロックと同じ周波数にします
// ADCは変換に96クロック掛かります
// DMAバッファを64にして64倍オーバーサンプリングします
// サンプルレート = 280MHz / 96 / 64 = 45.572kHz
// サンプルレート = 250MHz / 96 / 64 = 40.690kHz
// サンプルレート = 133MHz / 96 / 64 = 21.647kHz
//
#define ADC_GPIO_NUM 26 // 入力ADCチャンネル (GPIO26 = ADC0)

const int DUAL_PWM_DAC_L_HI = 14; // GPIO14 3.9k ohm
const int DUAL_PWM_DAC_L_LO = 15; // GPIO15 1M ohm
const int DUAL_PWM_DAC_R_HI = 16; // GPIO16 3.9k ohm
const int DUAL_PWM_DAC_R_LO = 17; // GPIO17 1M ohm

struct Message
{
  int32_t message, data;
};

inline DataSlot<Message> MESSAEG_SLOT; // loop関数から割り込み関数へ

struct AudioSample
{
  int32_t L, R; //-32768 <= x < 32767
  Message msg;
};

void __not_in_flash_func(AUDIO_CALLBACK_CORE_0)(AudioSample &sample);
void __not_in_flash_func(AUDIO_CALLBACK_CORE_1)(AudioSample &sample);

void initAudio();
void enableDac();

void enableCore0();
void enableCore1();

const int SAMPLE_RATE_OUT_0_PIN = 21;
const int SAMPLE_RATE_OUT_1_PIN = 22;
