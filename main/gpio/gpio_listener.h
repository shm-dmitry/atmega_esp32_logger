#ifndef MAIN_GPIO_GPIO_LISTENER_H_
#define MAIN_GPIO_GPIO_LISTENER_H_

#include "esp_err.h"
#include "gpio_listener_def.h"

esp_err_t gpio_listener_init(int gpio, gpio_listener_callback_t callback);

#endif /* MAIN_GPIO_GPIO_LISTENER_H_ */
