#pragma once
#include "SAMPLE_RATE.hpp"
#include <cstdint>
#include <cmath>

const float F_PI = 3.141592653589793238462643383279502884;
const float F_TAU = 2 * F_PI;
const double D_PI = 3.141592653589793238462643383279502884;
const double D_TAU = 2 * D_PI;

////////// sat //////////

////////// template version

inline int16_t sat8(int16_t v)
{
  // clang-format off
  if (v >  127) v =  127;
  if (v < -128) v = -128;
  // clang-format on
  return v;
}

inline int32_t sat16(int32_t v)
{
  // clang-format off
  if (v >  32767) v =  32767;
  if (v < -32768) v = -32768;
  // clang-format on
  return v;
}

inline float satf(float v)
{
  // clang-format off
  if (v >=  1.f) v =  0.99999997f;
  if (v <= -1.f        ) v = -1.f;
  // clang-format on
  return v;
}

inline double satd(double v)
{
  // clang-format off
  if (v >=  0.99999999999999994) v =  0.99999999999999994;
  if (v <= -1.0                ) v = -1.0;
  // clang-format on
  return v;
}

////////// 3 argument version

template <typename T>
inline T clamping(const T &l, T v, const T &h)
{
  // clang-format off
  if (v > h) v = h;
  if (v < l) v = l;
  // clang-format on
  return v;
}

template <int lo, int hi>
inline int clamping(int v)
{
  // clang-format off
  if (v > hi) v = hi;
  if (v < lo) v = lo;
  // clang-format on
  return v;
}

template <typename T>
inline T mapping(const T &value, const T &fromLow, const T &fromHigh, const T &toLow, const T &toHigh)
{
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

template <int fromLow, int fromHigh, int toLow, int toHigh>
inline int mapping(const int &value)
{
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

#define _MAPPING(value, fromLow, fromHigh, toLow, toHigh) (((value) - (fromLow)) * ((toHigh) - (toLow)) / ((fromHigh) - (fromLow)) + (toLow))

inline unsigned long xorshift()
{
  static unsigned long x = 123456789, y = 362436069, z = 521288629, w = 88675123;
  unsigned long t = (x ^ (x << 11));
  x = y;
  y = z;
  z = w;
  return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
}

inline int8_t randomNoise8()
{
  return ((xorshift() & 0xFF000000) >> 24) - 127;
}

inline int randomNoise16()
{
  return ((xorshift() & 0xFFFF0000) >> 16) - 32768;
}

inline float randomNoiseF() { return (float)randomNoise16() / 32768.f; }
inline float randomNoiseD() { return (double)randomNoise16() / 32768.0; }

inline constexpr int us2length(uint32_t us, uint32_t fs = SAMPLE_RATE) { return us * fs / 1000000; }

template <typename T>
inline constexpr int ms2length(T ms, uint32_t fs = SAMPLE_RATE) { return ms * fs / 1000; }

inline constexpr int ms2length(float ms, uint32_t fs = SAMPLE_RATE) { return ms * fs / 1000; }

inline constexpr int length2ms(uint32_t length, uint32_t fs = SAMPLE_RATE)
{
  return length * 1000 / fs;
}

inline constexpr int sec2length(float sec, uint32_t fs = SAMPLE_RATE)
{
  return sec * fs;
}

inline constexpr float length2sec(uint32_t len, uint32_t fs = SAMPLE_RATE) { return (float)len / fs; }

#include <cmath>

inline float vol2db(float vol) { return 20.f * log10(vol); }
inline float db2vol(float db) { return pow(10.f, db / 20); }

inline float pow2db(float vol) { return 10.f * log10(vol); }
inline float db2pow(float db) { return pow(10.f, db / 10); }

struct Vol2Db { float operator()(float v) const { return vol2db(v); } };
struct Db2Vol { float operator()(float v) const { return db2vol(v); } };

template <typename T>
inline T square(T v) { return v * v; }

template <typename T>
inline T rad2deg(T th) { return 180 / F_PI * th; }

template <typename T>
inline T deg2rad(T deg) { return F_PI / 180 * deg; }
