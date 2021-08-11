#ifndef MAIN_SOCKET_SOCKET_SERVER_H_
#define MAIN_SOCKET_SOCKET_SERVER_H_

#include "esp_err.h"
#include "../gpio/gpio_listener_def.h"

void socket_server_on_new_message(const gpio_listener_data_t * message);

esp_err_t socket_server_init(int port);

#endif /* MAIN_SOCKET_SOCKET_SERVER_H_ */
