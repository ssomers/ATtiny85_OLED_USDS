#pragma once
#include <Arduino.h>

class Glyph {
  public:
    static uint8_t constexpr SEGS = 8;
    // Separate decimanl and hex arrays so that the latter is not linked in unless the main code references it.
    static Glyph PROGMEM const dec_digit[10];
    static Glyph PROGMEM const ABCDEF[6];
    static Glyph PROGMEM const X;
    static Glyph PROGMEM const minus;
    static Glyph PROGMEM const plus;
    static Glyph PROGMEM const colon;
    static Glyph PROGMEM const pin[2];

    static Glyph const& hex_digit_hi(uint8_t n) {
      return hex_digit(n >> 4);
    }

    static Glyph const& hex_digit_lo(uint8_t n) {
      return hex_digit(n & 0xF);
    }

  private:
    static Glyph const& hex_digit(uint8_t n) {
      return n < 10 ? dec_digit[n] : ABCDEF[n - 10];
    }

    byte const seg0;
    byte const seg1;
    byte const seg2;
    byte const seg3;
    byte const seg4;
    byte const seg5;
    byte const seg6;
    byte const seg7;

    static constexpr bool pixel(char c) {
      return c != ' ';
    }

    static constexpr byte extractSegAt(int x, const char* col_per_row) {
      return 0
             | pixel(col_per_row[x + SEGS * 0]) << 0
             | pixel(col_per_row[x + SEGS * 1]) << 1
             | pixel(col_per_row[x + SEGS * 2]) << 2
             | pixel(col_per_row[x + SEGS * 3]) << 3
             | pixel(col_per_row[x + SEGS * 4]) << 4
             | pixel(col_per_row[x + SEGS * 5]) << 5
             | pixel(col_per_row[x + SEGS * 6]) << 6
             | pixel(col_per_row[x + SEGS * 7]) << 7;
    }

    // Private to keep all instances in this class and as PROGMEM.
    constexpr Glyph(const char* col_per_row)
      : seg0(extractSegAt(0, col_per_row))
      , seg1(extractSegAt(1, col_per_row))
      , seg2(extractSegAt(2, col_per_row))
      , seg3(extractSegAt(3, col_per_row))
      , seg4(extractSegAt(4, col_per_row))
      , seg5(extractSegAt(5, col_per_row))
      , seg6(extractSegAt(6, col_per_row))
      , seg7(extractSegAt(7, col_per_row))
    {}

  public:
    byte seg(uint8_t x) const {
      switch (x) {
        case 0: return pgm_read_byte(&seg0);
        case 1: return pgm_read_byte(&seg1);
        case 2: return pgm_read_byte(&seg2);
        case 3: return pgm_read_byte(&seg3);
        case 4: return pgm_read_byte(&seg4);
        case 5: return pgm_read_byte(&seg5);
        case 6: return pgm_read_byte(&seg6);
        case 7: return pgm_read_byte(&seg7);
      }
      __builtin_unreachable();
    }
};
