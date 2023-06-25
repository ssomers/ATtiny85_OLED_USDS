#include <inttypes.h>
#include "OLED.h"
#include "GlyphsOnQuarter.h"

struct OLED_DEVICE {
  static constexpr uint8_t ADDRESS { 0x3C };
  static constexpr USI_TWI_Delay tHSTART { 0 };
  static constexpr USI_TWI_Delay tSSTOP { 0 };
  static constexpr USI_TWI_Delay tIDLE { .6 };
  static constexpr USI_TWI_Delay tPRE_SCL_HIGH { 0 };
  static constexpr USI_TWI_Delay tPOST_SCL_HIGH { 0 };
  static constexpr USI_TWI_Delay tPOST_TRANSFER { 0 };
};

struct USDS_DEVICE {
  static constexpr uint8_t ADDRESS { 0x57 };
  static constexpr USI_TWI_Delay tHSTART { 0 };
  static constexpr USI_TWI_Delay tSSTOP { 0 };
  static constexpr USI_TWI_Delay tIDLE { .6 };
  static constexpr USI_TWI_Delay tPRE_SCL_HIGH { 3 };
  static constexpr USI_TWI_Delay tPOST_SCL_HIGH { 1.5 };
  static constexpr USI_TWI_Delay tPOST_TRANSFER { 0 };
};

static void flashN(uint8_t number) {
  while (number >= 5) {
    number -= 5;
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(700);
    digitalWrite(LED_BUILTIN, LOW);
  }
  while (number >= 1) {
    number -= 1;
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

// Report an error while the display isn't set up.
static void flashError(I2C::Status status) {
  if (status.error) {
    for (;;) {
      delay(1200);
      flashN(status.error);
      delay(600);
      flashN(status.location);
    }
  }
}

// Report an error while we think we can display it.
static void displayError(I2C::Status status) {
  static uint8_t last_line = 0;
  if (status.error) {
    if (++last_line == 4) last_line = 1;
    auto chat = GlyphsOnQuarter<OLED_DEVICE> {0, OLED::Quarter(last_line)};
    chat.send(0, 3);
    chat.send(GlyphPair::err.left);
    chat.send(GlyphPair::err.right);
    chat.send3dec(status.error);
    chat.send(0, 3);
    chat.send(Glyph::at);
    chat.send3dec(status.location);
  }
}

static void displayMillimeter(OLED::Quarter quarter, uint16_t value) {
  uint8_t constexpr width = Glyph::DIGIT_WIDTH * 5 + Glyph::POINT_WIDTH + GlyphPair::WIDTH;
  auto chat = GlyphsOnQuarter<OLED_DEVICE> {10, quarter, 0, uint8_t(width - 1)};
  uint8_t decimal3 = value % 10;
  value /= 10;
  uint8_t decimal2 = value % 10;
  value /= 10;
  uint8_t decimal1 = value % 10;
  value /= 10;
  uint8_t unit = value % 10;
  value /= 10;
  if (value == 0) {
    chat.send(0, Glyph::DIGIT_WIDTH);
  } else if (value < 10) {
    chat.send(Glyph::dec_digit[value]);
  } else {
    chat.send(~0, Glyph::DIGIT_WIDTH);
  }
  chat.send(Glyph::dec_digit[unit]);
  chat.sendPoint();
  chat.send(Glyph::dec_digit[decimal1]);
  chat.send(Glyph::dec_digit[decimal2]);
  chat.send(Glyph::dec_digit[decimal3]);
  chat.send(GlyphPair::m.left);
  chat.send(GlyphPair::m.right);
  displayError(chat.stop());
}

static void order_sample() {
  for (;;) {
    auto err = I2C::Chat<USDS_DEVICE> {7} .send(1).stop();
    switch (err.error) {
      case USI_TWI_OK: return;
      case USI_TWI_NO_ACK_ON_ADDRESS: continue;
      default: displayError(err);
    }
  }
}

static void displayBytes(OLED::Quarter quarter, uint8_t buf[3]) {
  uint8_t constexpr width = 6 * Glyph::DIGIT_WIDTH;
  auto chat = GlyphsOnQuarter<OLED_DEVICE> {20, quarter, OLED::WIDTH - width, OLED::WIDTH - 1, false};
  chat.send2hex(buf[0]);
  chat.send2hex(buf[1]);
  chat.send2hex(buf[2]);
  displayError(chat.stop());
}

static void await_reception(uint8_t buf[], size_t len) {
  // In practice the module responds in at most 156 ms, depending on the distance measured.
  for (;;) {
    delay(10);
    auto err = USI_TWI_Master_Receive<USDS_DEVICE>(buf, len);
    switch (err) {
      case USI_TWI_OK: return;
      case USI_TWI_NO_ACK_ON_ADDRESS: continue;
      default: displayError(I2C::Status { err, 15 });
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  USI_TWI_Master_Initialise();
  auto err = OLED::Chat<OLED_DEVICE> {0}
             .init()
             .set_addressing_mode(OLED::VerticalAddressing)
             .set_column_address()
             .set_page_address()
             .set_contrast(255)
             .set_enabled()
             .start_data()
             .sendN(OLED::BYTES, 0)
             .stop();
  digitalWrite(LED_BUILTIN, LOW);
  flashError(err);
}

void loop() {
  order_sample();

  uint8_t buf[3];
  digitalWrite(LED_BUILTIN, HIGH);
  await_reception(buf, sizeof buf);
  digitalWrite(LED_BUILTIN, LOW);
  displayBytes(OLED::Quarter::B, buf);
  // It's not worth while to have the 3rd byte of the micrometer value, but the device
  // gets angry if we don't read all three. And we need more than 16 bits to scale the
  // number using integer arithmetic.
  uint32_t distance = uint32_t(buf[0]) << 16 | uint32_t(buf[1]) << 8 | uint32_t(buf[2]);
  displayMillimeter(OLED::Quarter::A, uint16_t((distance + 500) / 1000)); // ųm to mm
  delay(500);
}
