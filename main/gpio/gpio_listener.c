#include "gpio_listener.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "../log.h"
#include "time.h"
#include "string.h"

#define GPIO_LISTENER_CHAR_DELIMETER_PERIOD_MS 10
#define GPIO_LISTENER_DATA_MAX_ROW_SIZE 250
#define GPIO_LISTENER_MAX_COLLECTING_MESSAGE_TIME 10

static volatile uint8_t gpio_listener_char = 0;
static volatile bool gpio_listener_on_receiving_char = false;
static xQueueHandle gpio_listener_evt_queue = NULL;
static gpio_listener_data_t last_row = { .data = { 0 }, .size = 0, .start_collecting = 0 };

static gpio_listener_callback_t gpio_listener_callback;

static void IRAM_ATTR gpio_listener_isr_handler(void* arg) {
	gpio_listener_char++;
	gpio_listener_on_receiving_char = true;
}

static void gpio_listener_on_new_char(void* arg) {
	while(true) {
		vTaskDelay(GPIO_LISTENER_CHAR_DELIMETER_PERIOD_MS / portTICK_PERIOD_MS);

		if (gpio_listener_on_receiving_char) {
			gpio_listener_on_receiving_char = false;
		} else if (gpio_listener_char > 0) {
			uint8_t current_char = gpio_listener_char;
			gpio_listener_char = 0;

			time_t now;
			time(&now);

			if (last_row.size == 0) {
				last_row.start_collecting = now;
			}

			last_row.data[last_row.size] = current_char;
			last_row.size++;

			if (last_row.size > GPIO_LISTENER_DATA_MAX_ROW_SIZE || current_char == '\n' || now - last_row.start_collecting > GPIO_LISTENER_MAX_COLLECTING_MESSAGE_TIME) {
				gpio_listener_data_t * temp = malloc(sizeof(gpio_listener_data_t));
				memcpy(temp, &last_row, sizeof(gpio_listener_data_t));
				memset(&last_row, 0, sizeof(gpio_listener_data_t));

				xQueueSend( gpio_listener_evt_queue, (void *) temp, ( TickType_t ) 0 );

				free(temp);
			}
		}
	}
}

static void gpio_listener_on_process_char(void* arg) {
	gpio_listener_data_t * temp = malloc(sizeof(gpio_listener_data_t));
    for(;;) {
    	memset(temp, 0, sizeof(gpio_listener_data_t));

        if(xQueueReceive(gpio_listener_evt_queue, temp, portMAX_DELAY) == pdPASS) {
        	if (temp->size == 0) {
        		continue;
        	}

        	gpio_listener_callback(temp);
        }
    }
}

esp_err_t gpio_listener_init(int gpio, gpio_listener_callback_t callback) {
	gpio_listener_callback = callback;

	gpio_config_t config = {
		.intr_type = GPIO_INTR_POSEDGE,
		.pin_bit_mask = (1ULL << gpio),
		.mode = GPIO_MODE_INPUT,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.pull_up_en = GPIO_PULLUP_ENABLE
	};

	esp_err_t res = gpio_config(&config);
	if (res) {
		ESP_LOGE(GPIO_LISTENER_LOG, "gpio_config error: %d. PIN = %d", res, gpio);
		return res;
	}

	res = gpio_set_intr_type(gpio, GPIO_INTR_POSEDGE);
	if (res) {
		ESP_LOGE(GPIO_LISTENER_LOG, "gpio_set_intr_type error: %d", res);
		return res;
	}

	gpio_listener_evt_queue = xQueueCreate(30, sizeof(gpio_listener_data_t));
	xTaskCreate(gpio_listener_on_new_char, "gpio_listener_on_new_char", 1024, NULL, 10, NULL);
	xTaskCreate(gpio_listener_on_process_char, "gpio_listener_on_process_char", 1024, NULL, 10, NULL);

	res = gpio_install_isr_service(0);
	if (res) {
		ESP_LOGE(GPIO_LISTENER_LOG, "gpio_install_isr_service error: %d", res);
		return res;
	}

    res = gpio_isr_handler_add(gpio, gpio_listener_isr_handler, NULL);
	if (res) {
		ESP_LOGE(GPIO_LISTENER_LOG, "gpio_isr_handler_add A error: %d", res);
		return res;
	}

	ESP_LOGI(GPIO_LISTENER_LOG, "Driver initialied");

	return ESP_OK;
}
