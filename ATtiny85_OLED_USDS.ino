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
  static constexpr USI_TWI_Delay tHSTART { 5 };
  static constexpr USI_TWI_Delay tSSTOP { 0 };
  static constexpr USI_TWI_Delay tIDLE { 5 };
  static constexpr USI_TWI_Delay tPRE_SCL_HIGH { 5 };
  static constexpr USI_TWI_Delay tPOST_SCL_HIGH { 5 };
  static constexpr USI_TWI_Delay tPOST_TRANSFER { 0 };
};

static int constexpr MICROS_PER_CM = 29;

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

// Report error when the display isn't set up.
static void flashError(I2C::Status status) {
  if (status.errorlevel) {
    for (;;) {
      delay(1200);
      flashN(status.errorlevel);
      delay(600);
      flashN(status.location);
    }
  }
}

// Report error when we think we can display it.
static void displayError(I2C::Status status) {
  if (status.errorlevel) {
    auto chat = OLED::QuarterChat<OLED_DEVICE> {3};
    GlyphsOnQuarter::sendTo(chat, Glyph::X);
    GlyphsOnQuarter::send3dec(chat, status.errorlevel);
    GlyphsOnQuarter::sendTo(chat, Glyph::X);
    GlyphsOnQuarter::send3dec(chat, status.location);
    GlyphsOnQuarter::sendTo(chat, Glyph::X);
    flashError(status);
  }
}

static void displayValue(uint8_t quarter, int value) {
  bool heartbeat = false;
  if (quarter == 0) {
    static bool toggle = 0;
    toggle ^= 1;
    heartbeat = toggle;
  }
  uint8_t constexpr width = GlyphsOnQuarter::DIGIT_WIDTH * 4 + 3 + 2 * Glyph::SEGS;
  auto chat = OLED::QuarterChat<OLED_DEVICE> {quarter, 0, uint8_t(width - 1)};
  GlyphsOnQuarter::send4dec(chat, value, heartbeat);
  chat.sendSpacing(3);
  GlyphsOnQuarter::sendTo(chat, GlyphPair::cm.left);
  GlyphsOnQuarter::sendTo(chat, GlyphPair::cm.right);
  displayError(chat.stop());
}

static void order_sample() {
  for (;;) {
    auto err = I2C::Chat<USDS_DEVICE> {7} .send(1).stop();
    switch (err.errorlevel) {
      case USI_TWI_OK: return;
      case USI_TWI_NO_ACK_ON_ADDRESS: continue;
      default: displayError(err);
    }
  }
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
             .set_enabled()
             .start_data()
             .sendN(OLED::BYTES, 0)
             .stop();
  digitalWrite(LED_BUILTIN, LOW);
  flashError(err);
}

void loop() {
  order_sample();

  digitalWrite(LED_BUILTIN, HIGH);
  uint8_t buf[3];
  await_reception(buf, sizeof buf);
  digitalWrite(LED_BUILTIN, LOW);
  uint32_t distance = uint32_t(buf[0]) << 16 | uint32_t(buf[1]) << 8 | uint32_t(buf[2]);
  displayValue(0, (distance + 5000) / 10000); // ųm to cm
  delay(500);
}
