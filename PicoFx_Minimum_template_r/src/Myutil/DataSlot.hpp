#pragma once
#include <cstdint>
#include <type_traits>
//
// loop関数から割り込み関数へのデータの受け渡し用
//
/* 使い方
struct FilterParam { int32_t a0, a1, b1; };
Audio::Slot<FilterParam> gParam = {{{1, 0, 0}, {1, 0, 0}}};

gParam.update({a0, a1, b1});  gParam.update({a0, a1, b1});

// 割り込み側 or core1
if (gParam.isUpdated())
{
  // 係数が変わった直後に1回だけ行う処理(状態のリセットなど)
}
const FilterParam &p = gParam.get();
*/
template <typename T>
struct DataSlot
{
  T buf[2];
  volatile uint32_t active = 0; // 読み手が使う側
  volatile uint32_t seq = 0;    // 書き手が更新のたびに増やす
  uint32_t seenSeq = 0;         // 読み手だけが使う(既読の番号)

  DataSlot()
  {
    static_assert(std::is_pod_v<T>);
  }

  // 書き手(loop)だけが呼ぶ
  void update(const T &data)
  {
    uint32_t back = active ^ 1;
    buf[back] = data;
    __dmb();       // データ書き込みを先に完了させる
    active = back; // 32bit 書き込み1回で切り替え
    __dmb();
    seq = seq + 1; // 最後に更新番号を進める
  }

  // 読み手(割り込み/core1)だけが呼ぶ
  // 更新があれば true を返し、既読にする
  bool isUpdated()
  {
    uint32_t s = seq;
    if (s == seenSeq)
      return false;
    seenSeq = s;
    return true;
  }

  // 読み手だけが呼ぶ
  const T &get() const
  {
    return buf[active];
  }
};
