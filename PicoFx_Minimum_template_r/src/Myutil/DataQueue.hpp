#pragma once
#include <cstdint>
#include <type_traits>
//
// 割り込み関数からloop関数へデータを送るのに使うキュー
//
/* 使い方
DataQueue<int16_t, 1024> gMeter;

gMeter.push(y); // 割り込み側 or core1

// loop() 側
int16_t v;
while (gMeter.pop(v))
{
  // 表示や集計など
}
*/
template <typename T, uint32_t N>
struct DataQueue
{
  static_assert(N >= 2 && (N & (N - 1)) == 0, "N は 2 のべき乗にしてください");

  T buf[N];
  volatile uint32_t wr = 0;      // 書き手だけが更新
  volatile uint32_t rd = 0;      // 読み手だけが更新
  volatile uint32_t dropped = 0; // 満杯で捨てた数(書き手だけが更新)

  DataQueue()
  {
    static_assert(std::is_pod_v<T>);
  }

  // ---- 書き手だけが呼ぶ(割り込み/core1)。待たない ----
  bool push(const T &v)
  {
    uint32_t w = wr;
    if (w - rd >= N) // 満杯なら捨てる
    {
      dropped = dropped + 1;
      return false;
    }
    buf[w & (N - 1)] = v;
    __dmb();    // データ書き込みを先に完了させる
    wr = w + 1; // 最後に位置を進める
    return true;
  }

  // ---- 読み手だけが呼ぶ(loop) ----
  bool pop(T &v)
  {
    uint32_t r = rd;
    if (r == wr) // 空
      return false;
    __dmb(); // wr を見た後にデータを読む順序を保証
    v = buf[r & (N - 1)];
    __dmb(); // 読み終わってから領域を解放
    rd = r + 1;
    return true;
  }

  // 読み手が呼ぶ: 溜まっている個数
  uint32_t available() const { return wr - rd; }

  // 読み手が呼ぶ: 溜まっているデータを全部捨てる
  void clear() { rd = wr; }
};
