/*	MQTT (over TCP) Example

	This example code is in the Public Domain (or CC0 licensed, at your option.)

	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_mac.h" // esp_base_mac_addr_get
#include "mqtt_client.h"
#include "SensorPacket.h"

#define EMG_TOPIC "iomt/sensors/emg"
#define EEG_TOPIC "iomt/sensors/eeg"
#define EKG_TOPIC "iomt/sensors/ekg"
#define MAX_TOPIC "iomt/sensors/max"

#define SINGLE_JSON_BUF 128

static int publish_single_adc(esp_mqtt_client_handle_t client,
                           const char   *topic,
                           uint32_t      ts,
                           uint16_t      value)
{
    char buf[SINGLE_JSON_BUF];
    int  len = snprintf(buf, sizeof(buf),
                        "{\"ts\":%" PRIu32 ",\"v\":%u}",
                        ts, value);
    if (len < 0 || len >= (int)sizeof(buf)) return -1;
    return esp_mqtt_client_publish(client, topic, buf, len, 1, 0);
}

static int publish_single_max(esp_mqtt_client_handle_t client,
                           const char   *topic,
                           uint32_t      ts,
                           const MAX3010x_Data* data)
{
    char buf[SINGLE_JSON_BUF];
    int  len = snprintf(buf, sizeof(buf),
                        "{"
                        "\"ts\":"             "%" PRIu32 ","
                        "\"heartRate\":"      "%" PRId32 ","
                        "\"validHeartRate\":" "%d,"
                        "\"spo2\":"           "%" PRId32 ","
                        "\"validSPO2\":"      "%d"
                        "}",
                        ts,
                        data->heartRate,
                        data->validHeartRate,
                        data->spo2,
                        data->validSPO2);
    if (len < 0 || len >= (int)sizeof(buf)) return -1;
    return esp_mqtt_client_publish(client, topic, buf, len, 1, 0);
}

static const char *TAG = "PUB";

extern const uint8_t root_cert_pem_start[] asm("_binary_root_cert_pem_start");
extern const uint8_t root_cert_pem_end[] asm("_binary_root_cert_pem_end");

EventGroupHandle_t mqtt_status_event_group;
#define MQTT_CONNECTED_BIT BIT2

extern MessageBufferHandle_t xMessageBufferRx;
extern size_t xItemSize;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	esp_mqtt_event_handle_t event = event_data;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xEventGroupSetBits(mqtt_status_event_group, MQTT_CONNECTED_BIT);
			break;
		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			xEventGroupClearBits(mqtt_status_event_group, MQTT_CONNECTED_BIT);
			break;
		case MQTT_EVENT_SUBSCRIBED:
			ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_UNSUBSCRIBED:
			ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_PUBLISHED:
			ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_DATA:
			ESP_LOGI(TAG, "MQTT_EVENT_DATA");
			break;
		case MQTT_EVENT_ERROR:
			ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
			break;
		case MQTT_EVENT_BEFORE_CONNECT:
			ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
			break;
		default:
			ESP_LOGW(TAG, "Other event id:%d", event->event_id);
			break;
	}
	return;
}

esp_err_t query_mdns_host(const char * host_name, char *ip);
void convert_mdns_host(char * from, char * to);

void mqtt_pub(void *pvParameters)
{
	ESP_LOGI(TAG, "Start CONFIG_MQTT_BROKER=[%s]", CONFIG_MQTT_BROKER);

	// Create Event Group
	mqtt_status_event_group = xEventGroupCreate();
	configASSERT( mqtt_status_event_group );
	xEventGroupClearBits(mqtt_status_event_group, MQTT_CONNECTED_BIT);

	// Set client id from mac
	uint8_t mac[8];
	ESP_ERROR_CHECK(esp_base_mac_addr_get(mac));
	for(int i=0;i<8;i++) {
		ESP_LOGD(TAG, "mac[%d]=%x", i, mac[i]);
	}
	char client_id[64];
	sprintf(client_id, "pub-%02x%02x%02x%02x%02x%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	ESP_LOGI(TAG, "client_id=[%s]", client_id);

    // Resolve mDNS host name
    char ip[128];
    char uri[138];
    ESP_LOGI(TAG, "CONFIG_MQTT_BROKER=[%s]", CONFIG_MQTT_BROKER);
    convert_mdns_host(CONFIG_MQTT_BROKER, ip);
    ESP_LOGI(TAG, "ip=[%s]", ip);
#if CONFIG_MQTT_TRANSPORT_OVER_TCP
    ESP_LOGI(TAG, "MQTT_TRANSPORT_OVER_TCP");
    sprintf(uri, "mqtt://%.60s:%d", ip, CONFIG_MQTT_PORT_TCP);
#elif CONFIG_MQTT_TRANSPORT_OVER_SSL
    ESP_LOGI(TAG, "MQTT_TRANSPORT_OVER_SSL");
    sprintf(uri, "mqtts://%.60s:%d", ip, CONFIG_MQTT_PORT_SSL);
#elif CONFIG_MQTT_TRANSPORT_OVER_WS
    ESP_LOGI(TAG, "MQTT_TRANSPORT_OVER_WS");
    sprintf(uri, "ws://%.60s:%d/mqtt", ip, CONFIG_MQTT_PORT_WS);
#elif CONFIG_MQTT_TRANSPORT_OVER_WSS
    ESP_LOGI(TAG, "MQTT_TRANSPORT_OVER_WSS");
    sprintf(uri, "wss://%.60s:%d/mqtt", ip, CONFIG_MQTT_PORT_WSS);
#endif
    ESP_LOGI(TAG, "uri=[%s]", uri);

    // Initialize MQTT configuration structure
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = uri,
#if CONFIG_MQTT_TRANSPORT_OVER_TCP
#elif CONFIG_MQTT_TRANSPORT_OVER_SSL
        .broker.verification.certificate = (const char *)root_cert_pem_start,
#elif CONFIG_MQTT_TRANSPORT_OVER_WS
#elif CONFIG_MQTT_TRANSPORT_OVER_WSS
        .broker.verification.certificate = (const char *)root_cert_pem_start,
#endif
#if CONFIG_BROKER_AUTHENTICATION
        .credentials.username = CONFIG_AUTHENTICATION_USERNAME,
        .credentials.authentication.password = CONFIG_AUTHENTICATION_PASSWORD,
#endif
        .credentials.client_id = client_id
    };

#if CONFIG_MQTT_PROTOCOL_V_3_1_1
	ESP_LOGI(TAG, "MQTT_PROTOCOL_V_3_1_1");
	mqtt_cfg.session.protocol_ver = MQTT_PROTOCOL_V_3_1_1;
#elif CONFIG_MQTT_PROTOCOL_V_5
	ESP_LOGI(TAG, "MQTT_PROTOCOL_V_5");
	mqtt_cfg.session.protocol_ver = MQTT_PROTOCOL_V_5;
#endif

	esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
	esp_mqtt_client_start(mqtt_client);
	xEventGroupWaitBits(mqtt_status_event_group, MQTT_CONNECTED_BIT, false, true, portMAX_DELAY);
	ESP_LOGI(TAG, "Connected to MQTT Broker");

	uint8_t buffer[xItemSize];
	while (1) {
		size_t received = xMessageBufferReceive(xMessageBufferRx, buffer, sizeof(buffer), portMAX_DELAY);
		ESP_LOGI(TAG, "xMessageBufferReceive received=%d", received);
		if (received > 0) {
			ESP_LOGD(TAG, "xMessageBufferReceive buffer=[%.*s]",received, buffer);
			EventBits_t EventBits = xEventGroupGetBits(mqtt_status_event_group);
			ESP_LOGI(TAG, "EventBits=0x%"PRIx32, EventBits);
			if (EventBits & MQTT_CONNECTED_BIT) {
				const SensorPacket *pkt = (const SensorPacket *)buffer;
				int msg_id;
				//Extract fields from buffer
				uint8_t type = pkt->header.type;//buffer[0];
				uint8_t count = pkt->header.count;//buffer[1];

				uint32_t timestamp = pkt->header.timestamp;
					//buffer[2] |
					//(buffer[3] << 8) |
					//(buffer[4] << 16) |
					//(buffer[5] << 24);

				uint8_t  *payload   = (uint8_t *)pkt->payload;
				ESP_LOG_BUFFER_HEX("payload buffer mqttpub", payload, 6); 

				ESP_LOGI(TAG, "rx=%u, type=0x%02x, count=%u, ts=%"PRIu32, received,type,count,timestamp);
				ESP_LOG_BUFFER_HEX(TAG, buffer, received); 


				// type == ADC_COMBINED
				if(type == ADC_COMBINED ){
					uint16_t emg_data, ekg_data, eeg_data;
					uint8_t* p = payload;

					// extract payload
					for(uint8_t i = 0; i < count; i++) {
						emg_data = p[0] | (p[1] << 8);
						eeg_data = p[2] | (p[3] << 8);
						ekg_data = p[4] | (p[5] << 8);

						publish_single_adc(mqtt_client, EMG_TOPIC, timestamp, emg_data);
						publish_single_adc(mqtt_client, EEG_TOPIC, timestamp, eeg_data);
						publish_single_adc(mqtt_client, EKG_TOPIC, timestamp, ekg_data);

						p+=6;
					}
				} else if(type == MAX1030x) {
					uint8_t validHr, validSP02;
					uint32_t sp02, heartRate;
					uint8_t* p = payload;


					for(uint8_t i = 0; i < count; i++){
						sp02 = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
						validSP02 = p[4];
						heartRate = p[5] | (p[6] << 8) | (p[7] << 16) | (p[8] << 24);
						validHr = p[9];
						
						const MAX3010x_Data data= {.spo2 = sp02, .validSPO2 = validSP02, .heartRate= heartRate, .validHeartRate=validHr};
						publish_single_max(mqtt_client, MAX_TOPIC, timestamp, &data);

						p+=10;
					}
				}



				int _received = received;
				//ESP_LOGI(TAG, "esp_mqtt_client_publish buffer=[%.*s]",_received, buffer);
				//int msg_id = esp_mqtt_client_publish(mqtt_client, CONFIG_MQTT_PUB_TOPIC, buffer, _received, 1, 0);
				//ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
			} else {
				ESP_LOGE(TAG, "Disconnected from MQTT Broker");
				break;
			}
		} else {
			 ESP_LOGE(TAG, "xMessageBufferReceive fail");
			 break;
		}
	} // end while

	// Stop connection
	ESP_LOGI(TAG, "Task Delete");
	esp_mqtt_client_stop(mqtt_client);
	vTaskDelete(NULL);
}
