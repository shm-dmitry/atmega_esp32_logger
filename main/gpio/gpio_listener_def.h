#ifndef MAIN_SOCKET_GPIO_LISTENER_DEF_H_
#define MAIN_SOCKET_GPIO_LISTENER_DEF_H_

#include "stdint.h"

typedef struct gpio_listener_data_t {
	uint8_t data[256];
	uint8_t size;
	time_t start_collecting;
} gpio_listener_data_t;

typedef void (* gpio_listener_callback_t)(const gpio_listener_data_t * message);

#endif /* MAIN_SOCKET_GPIO_LISTENER_DEF_H_ */
