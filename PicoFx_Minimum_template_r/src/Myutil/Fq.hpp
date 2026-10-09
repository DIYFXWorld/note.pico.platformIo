#pragma once
#include <cstdint>
#include <limits>
#include "./fpm/fixed.hpp"
#include "./fpm/ios.hpp"
#include "./fpm/math.hpp"

using f24q8  = fpm::fixed_24_8;
using f16q16 = fpm::fixed_16_16;
using f8q24  = fpm::fixed_8_24;

using f2q30 = fpm::fixed<std::int32_t, std::int64_t, 30>;
using f1q31 = fpm::fixed<std::int32_t, std::int64_t, 31>;

// unsigned
using uf24q8 = fpm::fixed<std::uint32_t, std::uint64_t, 8>;
using uf16q16 = fpm::fixed<std::uint32_t, std::uint64_t, 16>;
using uf8q24 = fpm::fixed<std::uint32_t, std::uint64_t, 24>;

// 16ビットサンプリングデータを正規化変換する関数
inline constexpr f16q16 S16Tof16q16(int32_t x) { return f16q16::from_raw_value(x << 1); }
inline constexpr int32_t f16q16ToS16(f16q16 x) { return x.raw_value() >> 1; }
inline constexpr f8q24  S16Tof8q24(int32_t x)  { return f8q24::from_raw_value(x << 9); }
inline constexpr int32_t f8q24ToS16(f8q24 x)   { return x.raw_value() >> 9; }
inline constexpr f24q8  S16Tof24q8(int32_t x)  { return f24q8::from_raw_value(x >> 7); }
inline constexpr int32_t f24q8ToS16(f24q8 x)   { return x.raw_value() << 7; }
inline constexpr f2q30  S16Tof2q30(int32_t x)  { return f2q30::from_raw_value(x << 15); }
inline constexpr int32_t f2q30ToS16(f2q30 x)   { return x.raw_value() >> 15; }
inline constexpr f1q31  S16Tof1q31(int32_t x)  { return f1q31::from_raw_value(x << 16); }
inline constexpr int32_t f1q31ToS16(f1q31 x)   { return x.raw_value() >> 16; }

template <typename fq_t, int FRAC_BITS>
inline constexpr fq_t S16ToFq(int32_t x)
{
    constexpr int shift = FRAC_BITS - 15;

    if constexpr (shift >= 0)
        return fq_t::from_raw_value(x * (int32_t(1) << shift));
    else
        return fq_t::from_raw_value(x >> -shift);
}

template <typename fq_t, int FRAC_BITS>
inline constexpr int16_t FqToS16(fq_t x)
{
    constexpr int shift = FRAC_BITS - 15;

    if constexpr (shift >= 0)
        return static_cast<int16_t>(x.raw_value() >> shift);
    else
        return static_cast<int16_t>(x.raw_value() * (int32_t(1) << -shift));
}

// -------------------------------------------------------------
// 定数定義 (1.0表現および各型の全ビット限界値)
// -------------------------------------------------------------
constexpr f16q16 f16q16_ZERO = f16q16::from_raw_value(0);
constexpr f16q16 f16q16_ONE  = f16q16::from_raw_value(1 << 16);
constexpr f16q16 f16q16_MIN  = f16q16::from_raw_value(INT32_MIN);
constexpr f16q16 f16q16_MAX  = f16q16::from_raw_value(INT32_MAX);

constexpr f8q24 f8q24_ZERO  = f8q24::from_raw_value(0);
constexpr f8q24 f8q24_ONE   = f8q24::from_raw_value(1 << 24);
constexpr f8q24 f8q24_MIN   = f8q24::from_raw_value(INT32_MIN);
constexpr f8q24 f8q24_MAX   = f8q24::from_raw_value(INT32_MAX);

constexpr f24q8 f24q8_ZERO  = f24q8::from_raw_value(0);
constexpr f24q8 f24q8_ONE   = f24q8::from_raw_value(1 << 8);
constexpr f24q8 f24q8_MIN   = f24q8::from_raw_value(INT32_MIN);
constexpr f24q8 f24q8_MAX   = f24q8::from_raw_value(INT32_MAX);

constexpr f2q30 f2q30_ZERO  = f2q30::from_raw_value(0);
constexpr f2q30 f2q30_ONE   = f2q30::from_raw_value(1 << 30);
constexpr f2q30 f2q30_MIN   = f2q30::from_raw_value(INT32_MIN);
constexpr f2q30 f2q30_MAX   = f2q30::from_raw_value(INT32_MAX);

constexpr f1q31 f1q31_ZERO  = f1q31::from_raw_value(0);
// f1q31 は +1.0 を表現できないため、MAX(=0x7FFFFFFF) を 1.0 近似値として利用するか注意が必要
constexpr f1q31 f1q31_MIN   = f1q31::from_raw_value(INT32_MIN);
constexpr f1q31 f1q31_MAX   = f1q31::from_raw_value(INT32_MAX);

// unsigned
constexpr uf24q8 Uf24q8_ZERO = uf24q8::from_raw_value(0);
constexpr uf24q8 Uf24q8_ONE  = uf24q8::from_raw_value(1U << 8);
constexpr uf24q8 Uf24q8_MIN  = uf24q8::from_raw_value(0);
constexpr uf24q8 Uf24q8_MAX  = uf24q8::from_raw_value(UINT32_MAX);

constexpr uf16q16 Uf16q16_ZERO = uf16q16::from_raw_value(0);
constexpr uf16q16 Uf16q16_ONE  = uf16q16::from_raw_value(1U << 16);
constexpr uf16q16 Uf16q16_MIN  = uf16q16::from_raw_value(0);
constexpr uf16q16 Uf16q16_MAX  = uf16q16::from_raw_value(UINT32_MAX);

constexpr uf8q24 Uf8q24_ZERO = uf8q24::from_raw_value(0);
constexpr uf8q24 Uf8q24_ONE  = uf8q24::from_raw_value(1U << 24);
constexpr uf8q24 Uf8q24_MIN  = uf8q24::from_raw_value(0);
constexpr uf8q24 Uf8q24_MAX  = uf8q24::from_raw_value(UINT32_MAX);

template <int Q>
using vfq32 = fpm::fixed<std::int32_t, std::int64_t, Q>;