#define SAMPLE_RATE 40'690
#include <Arduino.h>
#include <Myutil/Audio.hpp>
#include <Myutil/PicoUtil.hpp>
#include <Myutil/StateLed.hpp>
#include <ezButton.h>
//
// ADCのオーバーサンプリングとDMAを利用して、Dual PWM DACで出力するサンプルコード
// デュアルコア版
//
// クローズドシステム前提で
// サンプリングレートは一般的な48kHz/44.1kHzに合わせていません
//
StateLed STATE_LED(13); // フルカラーLED
// clang-format off
uint32_t COLORS[6] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_MAGENTA, COLOR_WHITE, COLOR_BLACK };
// clang-format on

// clang-format off
ezButton btn[6] = { ezButton(0), ezButton(1), ezButton(2), ezButton(3), ezButton(4), ezButton(5), };
// clang-format on
//
void setup()
{
  setSysClockMhz(250);
  Serial.begin(9600);
  delay(2000);

  initAudio();
  printClocks();
  STATE_LED.begin();
  STATE_LED.send(COLORS[0]);
  //
  for (int i = 0; i < 6; ++i)
    btn[i].loop();
  //
  enableCore0();
}
//
// スイッチやLEDなどの処理はloop関数で行います
//

float ratio = 1.0;

void loop()
{
  btn[0].loop();
  if (btn[0].isReleased())
  {
    static int i;
    ++i;
    i %= 6;
    STATE_LED.send(COLORS[i]);
  }

  btn[1].loop();
  if (btn[1].isReleased())
  {
    ratio += 0.1;
    if (ratio > 1.0)
      ratio = 0;
 }

 delay(100);
}
//
// setup1はCore1で動作する関数です
void setup1() { enableCore1(); } // Core1の準備が出来たことを通知
//
// loop1関数Audioライブラリで使っているのでユーザーは使わないで
// 代わりに下のAUDIO_CALLBACK_1関数を使います
// Core1は音声処理に全振りするので、できれば他の処理(UI処理とか)はしないようにします
//
////////// ////////// ////////// ////////// //////////
//
// Core0の音声処理関数
//
void __not_in_flash_func(AUDIO_CALLBACK_CORE_0)(AudioSample &sample)
{
  // sample.L/sample.Rに対してここで何か音声処理します。
  // sampleは参照なのでL/Rへ値を代入してください。
}
//
// Core1の音声処理関数
//
void __not_in_flash_func(AUDIO_CALLBACK_CORE_1)(AudioSample &sample)
{
  // sample.L/sample.Rに対してここで何か音声処理します。
  // sampleは参照なのでL/Rへ値を代入してください。
  sample.L *= ratio;
  sample.R *= ratio;
}