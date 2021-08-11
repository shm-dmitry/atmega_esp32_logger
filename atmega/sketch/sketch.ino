void setup() {
  gpio_logger_init();
}

int counter = 0;

void loop() {
  gpio_logger_send_message("Hello from ATMEGA! Counter now is %i", counter++);
  delay(1000);
}
