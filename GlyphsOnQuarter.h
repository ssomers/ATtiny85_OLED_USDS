#pragma once
#include "Glyph.h"
#include "OLED.h"

namespace GlyphsOnQuarter {

static constexpr uint8_t DIGIT_MARGIN = 1;
static constexpr uint8_t DIGIT_WIDTH = DIGIT_MARGIN + Glyph::SEGS + DIGIT_MARGIN;
static constexpr uint8_t POINT_WIDTH = DIGIT_MARGIN + 2 + DIGIT_MARGIN;
static constexpr byte HEARTBEAT_SEG1 = Glyph::extractSeg("  # # # ");
static constexpr byte HEARTBEAT_SEG2 = Glyph::extractSeg("# # #   ");

template <typename Device>
void sendTo(OLED::QuarterChat<Device>& chat, Glyph const& glyph, uint8_t margin = 0, bool heartbeat = false) {
  if (margin) {
    if (heartbeat) {
      heartbeat = false;
      chat.send(HEARTBEAT_SEG1, HEARTBEAT_SEG2);
      chat.sendSpacing(margin - 1);
    } else {
      chat.sendSpacing(margin);
    }
  }
  for (uint8_t x = 0; x < Glyph::SEGS; ++x) {
    byte seg = glyph.seg(x);
    chat.send(seg << 4 | (heartbeat ? HEARTBEAT_SEG1 : 0),
              seg >> 4 | (heartbeat ? HEARTBEAT_SEG2 : 0));
    heartbeat = false;
  }
  chat.sendSpacing(margin);
}

template <typename Device>
void sendSpacingTo(OLED::QuarterChat<Device>& chat, uint8_t width = DIGIT_WIDTH, bool heartbeat = false) {
  if (width) {
    if (heartbeat) {
      chat.send(HEARTBEAT_SEG1, HEARTBEAT_SEG2);
    }
    chat.sendSpacing(width - heartbeat);
  }
}

template <typename Device>
void sendPointTo(OLED::QuarterChat<Device>& chat) {
  chat.sendSpacing(DIGIT_MARGIN);
  chat.send(0, 0b1100);
  chat.send(0, 0b1100);
  chat.sendSpacing(DIGIT_MARGIN);
}

template <typename Device>
void send2hex(OLED::QuarterChat<Device>& chat, uint8_t number) {
  sendTo(chat, Glyph::hex_digit_hi(number), DIGIT_MARGIN);
  sendTo(chat, Glyph::hex_digit_lo(number), DIGIT_MARGIN);
}

template <typename Device>
void send4hex(OLED::QuarterChat<Device>& chat, uint16_t number) {
  sendTo(chat, Glyph::hex_digit_hi(uint8_t(number >> 8)), DIGIT_MARGIN);
  sendTo(chat, Glyph::hex_digit_lo(uint8_t(number >> 8)), DIGIT_MARGIN);
  sendTo(chat, Glyph::hex_digit_hi(uint8_t(number)), DIGIT_MARGIN);
  sendTo(chat, Glyph::hex_digit_lo(uint8_t(number)), DIGIT_MARGIN);
}

template <typename Device>
void send3dec(OLED::QuarterChat<Device>& chat, uint8_t number, bool heartbeat = false) {
  uint8_t p1 = number / 100;
  uint8_t p2 = number % 100;
  if (p1 != 0) {
    sendTo(chat, Glyph::dec_digit[p1], DIGIT_MARGIN, heartbeat);
  } else {
    sendSpacingTo(chat, DIGIT_WIDTH, heartbeat);
  }
  if (p1 != 0 || p2 >= 10) {
    sendTo(chat, Glyph::dec_digit[p2 / 10], DIGIT_MARGIN);
  } else {
    sendSpacingTo(chat);
  }
  sendTo(chat, Glyph::dec_digit[p2 % 10], DIGIT_MARGIN);
}

template <typename Device>
void send4dec(OLED::QuarterChat<Device>& chat, int number, bool heartbeat = false) {
  constexpr uint8_t NUM_ERR_GLYPHS = 5;
  static_assert(DIGIT_WIDTH * 4 == NUM_ERR_GLYPHS * Glyph::SEGS);
  if (number < 0) {
    for (uint8_t i = 0; i < NUM_ERR_GLYPHS; ++i) {
      sendTo(chat, Glyph::minus, heartbeat && i == 0);
    }
    return;
  }
  uint8_t p1 = number / 100;
  uint8_t p2 = number % 100;
  if (p1 >= 100) {
    for (uint8_t i = 0; i < NUM_ERR_GLYPHS; ++i) {
      sendTo(chat, Glyph::X, heartbeat && i == 0);
    }
    return;
  }
  if (p1 >= 10) {
    sendTo(chat, Glyph::dec_digit[p1 / 10], DIGIT_MARGIN, heartbeat);
  } else {
    sendSpacingTo(chat, DIGIT_WIDTH, heartbeat);
  }
  if (p1 != 0) {
    sendTo(chat, Glyph::dec_digit[p1 % 10], DIGIT_MARGIN);
  } else {
    sendSpacingTo(chat);
  }
  if (p1 != 0 || p2 >= 10) {
    sendTo(chat, Glyph::dec_digit[p2 / 10], DIGIT_MARGIN);
  } else {
    sendSpacingTo(chat);
  }
  sendTo(chat, Glyph::dec_digit[p2 % 10], DIGIT_MARGIN);
}

}
