#include "log.h"

#include "unistd.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "init/init.h"
#include "gpio/gpio_listener.h"
#include "socket/socket_server.h"

void app_main(void)
{
	init_flash();
	init_wifi(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);
	init_snmp();

	socket_server_init(CONFIG_TCP_LISTEN_PORT);
	gpio_listener_init(CONFIG_GPIO_LISTEN, socket_server_on_new_message);

	while(true) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

