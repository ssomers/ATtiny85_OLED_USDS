#pragma once
#include "Glyph.h"
#include "OLED.h"

template <typename Device>
class GlyphsOnQuarter : public OLED::QuarterChat<Device> {
    using super = OLED::QuarterChat<Device>;
  private:
    static constexpr byte HEARTBEAT_SEG1 = GlyphExtractor::extractSeg("  # # # ");
    static constexpr byte HEARTBEAT_SEG2 = GlyphExtractor::extractSeg("# # #   ");

    // Should be OLED::Quarter, but that cannot be narrowed until gcc 9.3.
    // Should be const, but AutoFormat screws up.
    uint8_t quarter_bit : 4;
    bool include_heartbeat : 1;

    bool toggle_heartbeat() {
      if (include_heartbeat) {
        include_heartbeat = false;
        static uint8_t heartbeats = 0b0000;
        heartbeats ^= quarter_bit;
        return heartbeats & quarter_bit;
      } else {
        return false;
      }
    }

  public:
    explicit GlyphsOnQuarter(uint8_t start_location,
                             OLED::Quarter quarter, uint8_t xBegin = 0, uint8_t xEnd = OLED::WIDTH - 1,
                             bool include_heartbeat = true)
      : super(start_location, quarter, xBegin, xEnd)
      , quarter_bit(uint8_t(1 << static_cast<uint8_t>(quarter)))
      , include_heartbeat(include_heartbeat) {
    }

    void send(byte seg) {
      bool heartbeat = toggle_heartbeat();
      super::send(seg << 4 | (heartbeat ? HEARTBEAT_SEG1 : 0),
                  seg >> 4 | (heartbeat ? HEARTBEAT_SEG2 : 0));
    }

    void send(Glyph const & glyph, uint8_t margin = 0) {
      sendSpacing(margin);
      for (uint8_t x = 0; x < Glyph::SEGS; ++x) {
        send(glyph.seg(x));
      }
      sendSpacing(margin);
    }

    void sendSpacing(uint8_t width) {
      for (uint8_t i = 0; i < width; ++i) {
        send(0);
      }
    }

    void sendColon() {
      sendSpacing(Glyph::DIGIT_MARGIN);
      for (int x = 0; x < Glyph::POINT_WIDTH - 2 * Glyph::DIGIT_MARGIN; ++x) {
        send(Glyph::COLON_SEG);
      }
      sendSpacing(Glyph::DIGIT_MARGIN);
    }

    void sendPoint() {
      sendSpacing(Glyph::DIGIT_MARGIN);
      for (int x = 0; x < Glyph::POINT_WIDTH - 2 * Glyph::DIGIT_MARGIN; ++x) {
        send(Glyph::POINT_SEG);
      }
      sendSpacing(Glyph::DIGIT_MARGIN);
    }

    void send2hex(uint8_t number) {
      send(Glyph::hex_digit_hi(number), Glyph::DIGIT_MARGIN);
      send(Glyph::hex_digit_lo(number), Glyph::DIGIT_MARGIN);
    }

    void send4hex(uint16_t number) {
      send(Glyph::hex_digit_hi(uint8_t(number >> 8)), Glyph::DIGIT_MARGIN);
      send(Glyph::hex_digit_lo(uint8_t(number >> 8)), Glyph::DIGIT_MARGIN);
      send(Glyph::hex_digit_hi(uint8_t(number)), Glyph::DIGIT_MARGIN);
      send(Glyph::hex_digit_lo(uint8_t(number)), Glyph::DIGIT_MARGIN);
    }

    void send3dec(uint8_t number) {
      uint8_t p1 = number / 100;
      uint8_t p2 = number % 100;
      if (p1 != 0) {
        send(Glyph::dec_digit[p1], Glyph:: DIGIT_MARGIN);
      } else {
        sendSpacing(Glyph::DIGIT_WIDTH);
      }
      if (p1 != 0 || p2 >= 10) {
        send(Glyph::dec_digit[p2 / 10], Glyph::DIGIT_MARGIN);
      } else {
        sendSpacing(Glyph::DIGIT_WIDTH);
      }
      send(Glyph::dec_digit[p2 % 10], Glyph::DIGIT_MARGIN);
    }

    void send4dec(int number) {
      if (number < 0) {
        for (uint8_t x = 0; x < Glyph::DIGIT_WIDTH * 4; ++x) {
          send(Glyph::MINUS_SEG);
        }
        return;
      }
      uint8_t p1 = number / 100;
      uint8_t p2 = number % 100;
      if (p1 >= 100) {
        for (uint8_t x = 0; x < Glyph::DIGIT_WIDTH * 4; ++x) {
          send(0xFF);
        }
        return;
      }
      if (p1 >= 10) {
        send(Glyph::dec_digit[p1 / 10], Glyph::DIGIT_MARGIN);
      } else {
        sendSpacing(Glyph::DIGIT_WIDTH);
      }
      if (p1 != 0) {
        send(Glyph::dec_digit[p1 % 10], Glyph::DIGIT_MARGIN);
      } else {
        sendSpacing(Glyph::DIGIT_WIDTH);
      }
      if (p1 != 0 || p2 >= 10) {
        send(Glyph::dec_digit[p2 / 10], Glyph::DIGIT_MARGIN);
      } else {
        sendSpacing(Glyph::DIGIT_WIDTH);
      }
      send(Glyph::dec_digit[p2 % 10], Glyph::DIGIT_MARGIN);
    }
};
