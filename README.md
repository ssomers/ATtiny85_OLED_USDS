# ATtiny85_OLED_USDS

Working demo of a Digispark clone (i.e. a wired up ATtiny85 microcontroller briefly identifying as an USB device after booting) connected to two I²C devices:
- an [ultrasonic distance sensor](https://joy-it.net/en/products/SEN-US01) configured for I²C by soldering a 10k resistor at the right place, as indicated on the board's silkscreen and contrary to the incorrect manual,
- a [tiny OLED display](https://joy-it.net/en/products/SBC-OLED01).

We only need 2 pins for the communication, and 1 pin (the Digispark's builtin LED) for diagnosis in case the display doesn't work.

Currently it just continuously measures distance and displays it.
