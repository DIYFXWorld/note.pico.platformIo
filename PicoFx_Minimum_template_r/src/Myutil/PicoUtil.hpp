#pragma once
#include <cstdio>
#include <cstdlib>
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h" // Pico SDKを使用する場合
#include <Arduino.h>

inline void setSysClockMhz(int freq_mhz)
{
  // RP2040の周波数レンジに応じた電圧設定
  vreg_voltage voltage = VREG_VOLTAGE_DEFAULT; // デフォルト（通常は1.10V）
  // clang-format off
       if (freq_mhz > 270) voltage = VREG_VOLTAGE_1_30; // 270MHz超：RP2040の内蔵レギュレータ最大値（個体差により限界に近い領域）
  else if (freq_mhz > 240) voltage = VREG_VOLTAGE_1_25; // 240MHz〜270MHz：高クロック向けの電圧
  else if (freq_mhz > 200) voltage = VREG_VOLTAGE_1_20; // 200MHz〜240MHz：マージンを持たせた常用オーバークロック電圧
  else if (freq_mhz > 133) voltage = VREG_VOLTAGE_1_15; // 133MHz〜200MHz：公式ドキュメントでも言及される、200MHz動作に必要な電圧
  
  // 133MHz（定格）以下は VREG_VOLTAGE_DEFAULT (1.10V) のまま  int freq_khz = freq_mhz * 1000;
  // clang-format on

  // 現在より高くする場合：電圧を先に上げる
  // 現在より低くする場合：クロックを先に下げる
  const uint32_t freq_khz = (uint32_t)freq_mhz * 1000;
  uint32_t cur_khz = clock_get_hz(clk_sys) / 1000;

  if (freq_khz >= cur_khz)
  {
    vreg_set_voltage(voltage);
    sleep_ms(10);
    set_sys_clock_khz(freq_khz, true);
  }
  else
  {
    set_sys_clock_khz(freq_khz, true);
    vreg_set_voltage(voltage);
    sleep_ms(10);
  }
//  clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, freq_khz, freq_khz);
}

inline void waitForSerial(int timeout = 10'0000)
{
  uint32_t start = millis();
  while (1)
  {
    if (Serial)
      break;
    if (millis() - start > timeout)
      break;
    delay(100);
  }
}

inline void printClocks()
{
  // clk_sys	システムクロック。CPU（Cortex-M33 / Hazard3）やバスのメイン速度。RP2350では標準150MHz。
  // clk_ref	リファレンスクロック。安定した外部水晶発振器（XOSC）をソースとし、他のクロックの基準になる。通常12MHz。
  // clk_peri	周辺機器用クロック。UART, SPI, I2Cなどの通信モジュール用。通常は clk_sys と同期。
  // clk_usb	USB用クロック。USB通信には正確な 48MHz が必要なため、専用のPLLから供給される。
  // clk_adc	ADC（アナログ・デジタル変換）用。48MHz以下で動作し、サンプリング精度に影響する。
  // clk_gpout0~3	外部出力用クロック。内部のクロックを特定のGPIOピンから外部機器へ供給するために使用。未設定では0Hz
  Serial.printf("Reference Clock\t\t%d MHz\n", frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_REF) / 1000);
  Serial.printf("System Clock\t\t%d MHz\n", frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) / 1000);
  Serial.printf("ADC Clock\t\t%d MHz\n", frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC) / 1000);
  Serial.printf("USB Clock\t\t%d MHz\n", frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB) / 1000);
  Serial.printf("Peripheral Clock\t%d MHz\n", frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI) / 1000);
  Serial.printf("RTC Clock\t\t%d KHz\n", frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC));
  Serial.printf("Free Memory\t\t%d bytes\n", rp2040.getFreeHeap());
}

inline void printHeap()
{
  size_t max = 1024 * 1024; // 1MBまで試す
  void* ptr = nullptr;
  while (max > 0) {
    ptr = malloc(max);
    if (ptr) {
      free(ptr);
      break;
    }
    max -= 1024; // 1KBずつ減らす
  }
  Serial.printf("Approx Free Heap: %u bytes\n", (unsigned int)max);
}


// clang-format off
#ifdef PICO_RP2350
  #define	NOP()				asm volatile ("nop" ::: "memory"); asm volatile ("nop" ::: "memory")  // Cortex-M33は16ビット命令をを同時実行(Limited Dual-Issue)するので、NOPは2命令で1サイクル実行になる。
#elif defined(PICO_RP2040)
  #define	NOP()				asm volatile ("nop" ::: "memory")
#endif
#define	NOP5()			{ NOP();NOP();NOP();NOP();NOP(); }
#define	NOP10()			{ NOP5();NOP5(); }
#define	NOP20()			{ NOP10();NOP10(); }
#define	NOP30()			{ NOP10();NOP20(); }
#define	NOP40()			{ NOP20();NOP20(); }
#define	NOP50()			{ NOP20();NOP30(); }
#define	NOP60()			{ NOP30();NOP30(); }
#define	NOP70()			{ NOP40();NOP30(); }
#define	NOP80()			{ NOP40();NOP40(); }
#define	NOP90()			{ NOP50();NOP40(); }
#define	NOP100()		{ NOP50();NOP50(); }
#define	NOP200()		{ NOP100();NOP100(); }
#define	NOP300()		{ NOP100();NOP200(); }
#define	NOP400()		{ NOP200();NOP200(); }
#define	NOP500()		{ NOP200();NOP300(); }
#define	NOP600()		{ NOP300();NOP300(); }
#define	NOP700()		{ NOP400();NOP300(); }
#define	NOP800()		{ NOP400();NOP400(); }
#define	NOP900()		{ NOP500();NOP400(); }
#define	NOP1000()		{ NOP500();NOP500(); }
#define	NOP2000()		{ NOP1000();NOP1000(); }
#define	NOP3000()		{ NOP2000();NOP1000(); }
#define	NOP4000()		{ NOP2000();NOP2000(); }
#define	NOP5000()		{ NOP2000();NOP3000(); }
#define	NOP6000()		{ NOP3000();NOP3000(); }
#define	NOP7000()		{ NOP4000();NOP3000(); }
#define	NOP8000()		{ NOP4000();NOP4000(); }
#define	NOP9000()		{ NOP5000();NOP4000(); }
#define	NOP10000()	{ NOP5000();NOP5000(); }
#define	NOP20000()	{ NOP10000();NOP10000(); }
#define	NOP30000()	{ NOP10000();NOP20000(); }
#define	NOP40000()	{ NOP20000();NOP20000(); }
#define	NOP50000()	{ NOP20000();NOP30000(); }
// clang-format on
