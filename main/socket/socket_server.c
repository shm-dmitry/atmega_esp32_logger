#include "socket_server.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "../log.h"

static int socket_server_client_socket = -1;
#define SOCKET_SERVER_WELLCOME_MESSAGE "\n*** ATMega ESP32 Logger***\nWellcome!\n\n"

bool socket_server_send_message(int socket, const uint8_t * message, int message_length) {
	int index = 0;

	while (message_length > 0) {
		int sended = send(socket, message + index, message_length, 0);
		if (sended < 0) {
			return false;
		}

		message_length -= sended;
		index += sended;
	}

	return true;
}


void socket_server_on_new_message(const gpio_listener_data_t * message) {
	int sock = socket_server_client_socket;
	if (sock < 0) {
		return;
	}

	if (message->size == 0) {
		return;
	}

	char buff[20] = { 0 };
	snprintf(buff, sizeof(buff), "%li: ", message->start_collecting);
	if (!socket_server_send_message(sock, (const uint8_t *) buff, strlen(buff))) {
        ESP_LOGE(SOCKET_SERVER_LOG, "Error occurred during sending date: errno %d", errno);
        socket_server_client_socket = -1;
        return;
	}

	if (!socket_server_send_message(sock, message->data, message->size)) {
        ESP_LOGE(SOCKET_SERVER_LOG, "Error occurred during sending message: errno %d", errno);
        socket_server_client_socket = -1;
	}
}

bool socket_server_send_wellcome_message(int socket) {
	if (!socket_server_send_message(socket, (const uint8_t *) SOCKET_SERVER_WELLCOME_MESSAGE, strlen(SOCKET_SERVER_WELLCOME_MESSAGE))) {
        ESP_LOGE(SOCKET_SERVER_LOG, "Cant send welcome message: errno %d", errno);
        return false;
	}

	return true;
}

static void socket_server_listener(void * arg) {
	int listen_sock = (int) arg;

    ESP_LOGI(SOCKET_SERVER_LOG, "Socket server :: thread :: listen socket == %i", listen_sock);

    while (true) {
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);

        if (sock < 0) {
            ESP_LOGE(SOCKET_SERVER_LOG, "Unable to accept connection: errno %d", errno);
            break;
        }

        char addr_str[128];
        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        ESP_LOGI(SOCKET_SERVER_LOG, "Socket accepted ip address: %s", addr_str);

        int i = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&i, sizeof(i));

        if (socket_server_send_wellcome_message(sock)) {
			socket_server_client_socket = sock;
			while (socket_server_client_socket >= 0) {
				vTaskDelay(100 / portTICK_PERIOD_MS);
				uint8_t temp;
				if (read(sock, &temp, 1) < 0) {
			        ESP_LOGI(SOCKET_SERVER_LOG, "Client disconnected from %s", addr_str);
			        socket_server_client_socket = -1;
				}
			}
        }

        shutdown(sock, 0);
        close(sock);
    }

    close(listen_sock);
    vTaskDelete(NULL);
}

esp_err_t socket_server_init(int port) {
	struct sockaddr_storage dest_addr;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(port);

    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(SOCKET_SERVER_LOG, "Unable to create socket: errno %d", errno);
        return ESP_FAIL;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(SOCKET_SERVER_LOG, "Socket unable to bind: errno %d", errno);
        close(listen_sock);
        return ESP_FAIL;
    }

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(SOCKET_SERVER_LOG, "Error occurred during listen: errno %d", errno);
        close(listen_sock);
        return ESP_FAIL;
    }

    ESP_LOGI(SOCKET_SERVER_LOG, "Socket server :: listen socket == %i", listen_sock);

    uint32_t arg = listen_sock;
    xTaskCreate(socket_server_listener, "tcp_server_listener", 4096, (void *) arg, 5, NULL);

    ESP_LOGI(SOCKET_SERVER_LOG, "Socket server initialized on port %d", port);

	return ESP_OK;
}
