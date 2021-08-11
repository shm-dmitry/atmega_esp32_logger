This project allows to dump some messages from ATMega microcontroller to your computer without JTAG interface.

In my case I using USBasp programmator and it does not allows me to debug firmware (no debug, no console)

This project helps me to solve this problem.
Firmware on ESP32 starts simple SOCKET server, witch listen some port.
ATMega sends message to ESP32 via one GPIO line using PWM (number of impulses == symbol code).
Firmware on ESP32 readed this codes from GPIO, converts it into chars, collect to internal buffer and sends to client when

- symbol '\n' received
- internal buffer (255 chars) overflowed
- no symbols received during 10 seconds

Be carefull: ATMega works on 5V line, but ESP32 works on 3V3 line, so, you needs for level-converter between ATMega and ESP32.
Variants of simple level-converters:
- two resisters
- one potenciometer
- resister + zener diode
