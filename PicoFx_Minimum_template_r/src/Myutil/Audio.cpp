#include <Arduino.h>
#include <cstdint>
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "Audio.hpp"
#include "Gpio.hpp"
#include "PicoUtil.hpp"
#include "SumBuf.hpp"
#include "DualPwmDacStereo.hpp"

#define SAMPLES_PER_BUFFER 64 // バッファごとのサンプル数

// ダブルバッファ（Ping / Pong）
uint16_t buffer_ping[SAMPLES_PER_BUFFER], buffer_pong[SAMPLES_PER_BUFFER];

int dma_chan_ping, dma_chan_pong;

DualPwmDac PWM_DAC(DUAL_PWM_DAC_L_HI, DUAL_PWM_DAC_L_LO, DUAL_PWM_DAC_R_HI, DUAL_PWM_DAC_R_LO);

static Gpio SAMPLE_RATE_OUT_0(SAMPLE_RATE_OUT_0_PIN), SAMPLE_RATE_OUT_1(SAMPLE_RATE_OUT_1_PIN);

void __not_in_flash_func(AUDIO_CALLBACK)(AudioSample &);
//
// DMA割り込みハンドラ
//
static void __not_in_flash_func(dma_handler)()
{
  SAMPLE_RATE_OUT_0.set(true);

  // Pingチャンネルの転送が完了した場合
  if (dma_hw->ints0 & (1u << dma_chan_ping))
  {
    dma_hw->ints0 = 1u << dma_chan_ping; // 割り込みクリア
    dma_channel_set_write_addr(dma_chan_ping, buffer_ping, false);
    AudioSample sample;
    sample.L = SUM_BUF_64(buffer_ping);
    sample.L = sample.L / 4 - 32768;
    sample.R = sample.L;
    AUDIO_CALLBACK(sample);
    PWM_DAC.setOutput(sample.L, sample.R);
  }

  // Pongチャンネルの転送が完了した場合
  if (dma_hw->ints0 & (1u << dma_chan_pong))
  {
    dma_hw->ints0 = 1u << dma_chan_pong; // 割り込みクリア
    dma_channel_set_write_addr(dma_chan_pong, buffer_pong, false);
    AudioSample sample;
    sample.L = SUM_BUF_64(buffer_pong);
    sample.L = sample.L / 4 - 32768;
    sample.R = sample.L;
    AUDIO_CALLBACK(sample);
    PWM_DAC.setOutput(sample.L, sample.R);
  }
  SAMPLE_RATE_OUT_0.set(false);
}

void initAudio()
{
  PWM_DAC.begin();

  // --- 1. ADCの初期化 ---
  adc_init();
  adc_gpio_init(ADC_GPIO_NUM);
  adc_select_input(ADC_GPIO_NUM - 26);

  // ADC FIFOの設定
  adc_fifo_setup(
      true,  // FIFOを有効化
      true,  // DMAデータリクエスト(DREQ)を有効化
      1,     // 1サンプルごとにDREQを発行
      false, // エラービットを含めない
      false  // 8bitにシフトせず12bitのまま取得
  );

  // ADCクロックのオーバークロック
#if PICO_RP2040
  volatile uint32_t *vreg = (volatile uint32_t *)(CLOCKS_BASE + 0x60);
#elif PICO_RP2350
  volatile uint32_t *vreg = (volatile uint32_t *)(CLOCKS_BASE + 0x6c);
#endif
  *vreg = 0x820; // bit 5をセットしてadcをsys_clkで駆動し、enableにする

  adc_set_clkdiv(0); // 最速で回す

  // --- 2. DMAチャンネルの初期化 ---
  dma_chan_ping = dma_claim_unused_channel(true);
  dma_chan_pong = dma_claim_unused_channel(true);

  // --- Ping チャンネルの設定 ---
  dma_channel_config c_ping = dma_channel_get_default_config(dma_chan_ping);
  channel_config_set_transfer_data_size(&c_ping, DMA_SIZE_16);
  channel_config_set_read_increment(&c_ping, false);   // ADC FIFOアドレスは固定
  channel_config_set_write_increment(&c_ping, true);   // メモリ側はインクリメント
  channel_config_set_dreq(&c_ping, DREQ_ADC);          // ADCのDREQに同期
  channel_config_set_chain_to(&c_ping, dma_chan_pong); // 終了したらPongを自動起動！

  dma_channel_configure(
      dma_chan_ping,
      &c_ping,
      buffer_ping,   // 転送先
      &adc_hw->fifo, // 転送元 (ADC FIFO)
      SAMPLES_PER_BUFFER,
      false // まだ起動しない
  );

  // --- Pong チャンネルの設定 ---
  dma_channel_config c_pong = dma_channel_get_default_config(dma_chan_pong);
  channel_config_set_transfer_data_size(&c_pong, DMA_SIZE_16);
  channel_config_set_read_increment(&c_pong, false);
  channel_config_set_write_increment(&c_pong, true);
  channel_config_set_dreq(&c_pong, DREQ_ADC);
  channel_config_set_chain_to(&c_pong, dma_chan_ping); // 終了したらPingを自動起動！

  dma_channel_configure(
      dma_chan_pong,
      &c_pong,
      buffer_pong,   // 転送先
      &adc_hw->fifo, // 転送元
      SAMPLES_PER_BUFFER,
      false // まだ起動しない
  );

  // --- 3. 割り込みの設定 ---
  dma_set_irq0_channel_mask_enabled((1u << dma_chan_ping) | (1u << dma_chan_pong), true);
  irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
  irq_set_enabled(DMA_IRQ_0, true);

  // --- 4. 転送開始 ---
  // まずPingチャンネルをトリガーして待機状態にし、ADC自体のフリーランニングをONにする
  dma_channel_start(dma_chan_ping);
  adc_run(true);
}

void enableDac()
{
  PWM_DAC.enable = true;
}

//
// 以下デュアルコア同期関連
//
static volatile bool CORE0_READY = false;
static volatile bool CORE1_READY = false;

void enableCore0()
{
  CORE0_READY = true;
  delay(1000);
}

void enableCore1() { CORE1_READY = true; }

static AudioSample toCore1;   // 送りデータ
static AudioSample fromCore1; // 戻りデータ

static volatile uint32_t seqReq = 0; // core0 が進める
static volatile uint32_t seqAck = 0; // core1 が進める

//
// core0 音声処理
//
void __not_in_flash_func(AUDIO_CALLBACK)(AudioSample &sample)
{
  if (!(CORE0_READY && CORE1_READY)) // コアが起動していないなら戻る
    return;

  static AudioSample lastData; // 前回のデータ

  toCore1 = lastData; // Core1へ送るデータをセット

  __dmb(); // toCore1 の書き込みを確定させてから合図
  uint32_t req = seqReq + 1;
  seqReq = req;
  __sev();

  lastData = sample;
  //
  // ここで音声処理を行う
  //
  AUDIO_CALLBACK_CORE_0(lastData);

  while (seqAck != req)
    __wfe(); // core1からのイベントを待つ
  __dmb();   // 合図を見た後に fromCore1 を読む

  sample = fromCore1;
}
//
// core1 音声処理
//
void __not_in_flash_func(loop1)()
{
  if (!(CORE0_READY && CORE1_READY)) // コアが起動していないなら戻る
    return;

  uint32_t seen = seqAck; // 「最後に応答した番号」から開始
  static AudioSample lastData;

  while (1)
  {
    while (seqReq == seen)
      __wfe(); // Core0 からのイベント待ち
    __dmb();   // メモリバリア
    seen = seqReq;

    SAMPLE_RATE_OUT_1.set(true);

    AudioSample input = toCore1; // ack の前に入力を確保
    fromCore1 = lastData; // 前回の結果をCore0へ戻す

    __dmb();
    seqAck = seen;
    __sev(); // Core0へイベントを送る
    //
    // ここで音声処理を行う
    //
    lastData = input;
    AUDIO_CALLBACK_CORE_1(lastData);

    SAMPLE_RATE_OUT_1.set(false);
  }
}
/*
Seq は Sequence(シーケンス、連番) の略です。
seqReq = Sequence of Request(リクエストの連番)
seqAck = Sequence of Acknowledge(確認応答の連番)
Req と Ack は、通信の世界でよく使われる組み合わせです。TCP の SYN/ACK や、ハンドシェイク方式の信号線(REQ/ACK)と同じ考え方です。
core0 が「データを置いたよ、処理して」と番号を進めるのが Req(要求)
core1 が「受け取って返答を置いたよ」と番号を進めるのが Ack(応答)
seqReq == seqAck なら「要求と応答が釣り合っている(待ち状態)」、seqReq の方が進んでいれば「core1 の返答待ち」という見方ができます。
*/