#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "rom/ets_sys.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "sdkconfig.h"
#include "DHT11.h"
#include "HD44780.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include <string.h>

#include "esp_event.h"
#include "esp_netif.h"

#include "esp_http_client.h"
// cert
#include "esp_crt_bundle.h"
// wifi 
#include "connect_wifi.h"
//----------------------- Server -------------------------------------
#define JSON_URL "https://api.jsonbin.io/v3/b/643934a2c0e7653a05a44ad6"
char data_to_update[200];
char data_default[] = "{\"temparature\":\"%s\",\"humi\":\"%s\",\"detect\":\"%s\",\"device_1\":\"%s\",\"device_2\":\"%s\",\"device_3\": \"%s\"}";
char temp[6];
char humi[6];
char detect[5];
char device_1[5];
char device_2[5];
char device_3[5];
bool mcu_priority = false;
char X_MASTER_KEY[] = "$2b$10$1mN5y.XkrvbsFt6ovDD.R.2t.YaD262gKkvTZyfJr0S/3Oq/j1kQ6";
char X_ACCESS_KEY[] = "$2b$10$O.WIHfTbkkgTb6IB9XKlFuL9CIYC5kcrnXnA8Hi299J1MMHSt1UpW";
//-------------------------------------------------------------------

#define ESP_INR_FLAG_DEFAULT 0
#define LED_PIN_1 23
#define LED_PIN_2 25
#define LED_PIN_3 26
#define LED_PIN_APP 32

#define speaker_PIN 27
#define PUSH_BUTTON_PIN_APP 13
#define PUSH_BUTTON_PIN_1 14
#define PUSH_BUTTON_PIN_2 15
#define PUSH_BUTTON_PIN_3 16

#define pin_sensor 17
#define DHT_PIN 33

// LCD
#define LCD_ADDR 0x27
#define SDA_PIN 21
#define SCL_PIN 22
#define LCD_COLS 16
#define LCD_ROWS 2



//-------------------------------------------------------------------------

esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        // printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
		char *data_string = (char *)evt->data;
		strncpy(temp, data_string+26, 5);
		strncpy(humi, data_string+41, 5);
		strncpy(detect, data_string+58, 2);
		strncpy(device_1, data_string+74, 2);
		strncpy(device_2, data_string+90, 2);
		strncpy(device_3, data_string+106, 2);

        break;

    default:
        break;
    }
    return ESP_OK;
}
esp_err_t client_event_put_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        // printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
		printf("Your data updated to server !\n");
        break;

    default:
        break;
    }
    return ESP_OK;
}

void rest_get()
{
    esp_http_client_config_t config_get = {
        .url = JSON_URL,
        .method = HTTP_METHOD_GET,
		.transport_type = HTTP_TRANSPORT_OVER_SSL,  //Specify transport type
		.crt_bundle_attach = esp_crt_bundle_attach, //Attach the certificate bundle 
		.event_handler =client_event_get_handler
	};
	esp_http_client_handle_t client = esp_http_client_init(&config_get);
    // Add header to request
	esp_http_client_set_header(client, "X-Master-Key", X_MASTER_KEY);
	esp_http_client_set_header(client, "X-Access-Key", X_ACCESS_KEY);
	// Perform 
    esp_http_client_perform(client);
	// clean
    esp_http_client_cleanup(client);
}

void rest_put(char data[])
{
    esp_http_client_config_t config_get = {
        .url = JSON_URL,
        .method = HTTP_METHOD_PUT,
		.transport_type = HTTP_TRANSPORT_OVER_SSL,  //Specify transport type
		.crt_bundle_attach = esp_crt_bundle_attach, //Attach the certificate bundle 
		.event_handler =client_event_put_handler
	};
	esp_http_client_handle_t client = esp_http_client_init(&config_get);
    // Add header to request
	esp_http_client_set_header(client, "Content-Type", "application/json");
	esp_http_client_set_header(client, "X-Master-Key", X_MASTER_KEY);
	esp_http_client_set_header(client, "X-Access-Key", X_ACCESS_KEY);
	// add post data 
	esp_http_client_set_post_field(client, data, strlen(data));
	// Perform 
    esp_http_client_perform(client);
	// clean
    esp_http_client_cleanup(client);
}

void auto_update(void)
{
	while(1)
	{	
		
        // format data_default replace %s
		if(!mcu_priority) {
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            sprintf(data_to_update, data_default, temp, humi, detect, device_1, device_2, device_3);
            rest_put(data_to_update);
		    printf("data: %s\n", data_to_update);
            printf("ESP32 selected, data updated");
            // next get
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }else{
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            printf("Aplication selected, data not updated");
            
		    rest_get();
		    // update value in MCU 
		    // strcmp return 
		    if(strcmp(device_1,"OF")){
			    printf("LED_1 ON\n");
			    gpio_set_level(LED_PIN_1, true);
		        }else{
			    printf("LED_1 OF\n");
			    gpio_set_level(LED_PIN_1,false);
		    }
		    if(strcmp(device_2,"OF")){
			    printf("LED_2 ON\n");
			    gpio_set_level(LED_PIN_2, true);
		    }else{
			    printf("LED_2 OF\n");
			    gpio_set_level(LED_PIN_2,false);
		    }
		    if(strcmp(device_3,"OF")){
			printf("LED_3 ON\n");
			gpio_set_level(LED_PIN_3, true);

		    }else{
			printf("LED_3 OF\n");
			gpio_set_level(LED_PIN_3,false);
		    }
        }
        

	}
	
	
}
//-------------------------------------------------------------------------
//                   MCU
//-------------------------------------------------------------------------
TaskHandle_t ISR_1 = NULL;
TaskHandle_t ISR_2 = NULL;
TaskHandle_t ISR_3 = NULL;
TaskHandle_t ISR_4 = NULL;
TaskHandle_t ISR_5 = NULL;


void IRAM_ATTR button_isr_handler_1(void *arg)
{
    xTaskResumeFromISR(ISR_1);
}

void IRAM_ATTR button_isr_handler_2(void *arg)
{
    xTaskResumeFromISR(ISR_2);
}

void IRAM_ATTR button_isr_handler_3(void *arg)
{
    xTaskResumeFromISR(ISR_3);
}

void IRAM_ATTR button_isr_handler_4(void *arg)
{
    xTaskResumeFromISR(ISR_4);
}
void IRAM_ATTR button_isr_handler_5(void *arg)
{
    xTaskResumeFromISR(ISR_5);
}

void interrupt_task_1(void *arg)
{
    bool led_status = false;
    while (1)
    {
        vTaskSuspend(NULL);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        led_status = !led_status;
        if(led_status == true){
            strcpy(device_1, "ON");

        }else{
            strcpy(device_1, "OF");
        }
        gpio_set_level(LED_PIN_1, led_status);
        printf("Button pressed_task1!\n");
    }
}

void interrupt_task_2(void *arg)
{
    bool led_status = false;
    while (1)
    {
        vTaskSuspend(NULL);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        led_status = !led_status;
        if(led_status == true){
            strcpy(device_2, "ON");
        }else{
            strcpy(device_2, "OF");
        }
        gpio_set_level(LED_PIN_2, led_status);
        printf("Button pressed_task2!\n");
    }
}

void interrupt_task_3(void *arg)
{
    bool led_status = false;
    while (1)
    {
        vTaskSuspend(NULL);
vTaskDelay(200 / portTICK_PERIOD_MS);
        led_status = !led_status;
        if(led_status == true){
            strcpy(device_3, "ON");
        }else{
            strcpy(device_3, "OF");
        }
        gpio_set_level(LED_PIN_3, led_status);
        printf("Button pressed_task3!\n");
    }
}

void interrupt_task_4(void *arg)
{
    bool speaker_status;
    while (1)
    {
        vTaskSuspend(NULL);
        speaker_status = true;
        strcpy(detect, "ON");
        gpio_set_level(speaker_PIN, speaker_status);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
       speaker_status= !speaker_status;
         gpio_set_level(speaker_PIN, speaker_status);
    } 
}

void interrupt_task_5(void *arg)
{
    bool led_status = false;
    while (1)
    {
        vTaskSuspend(NULL);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        led_status = !led_status;
        mcu_priority = led_status;
        gpio_set_level(LED_PIN_APP, led_status);
        if(!mcu_priority){
            printf("ESP32 send data\n");
        }else{
            printf("ESP32 recieve data\n");
        }

    }
}

void DHT_reader_task(void *pvParameter)
{
    setDHTgpio(DHT_PIN);
    while (1)
    {
		char num[20];
        int ret = readDHT();
        errorHandler(ret);
        if (getHumidity() >= (float)90.00)
        {
            vTaskResume(ISR_4);
        }
        printf("Humidity %.2f %%\n", getHumidity());
		sprintf(num, "Hum_%.2f_%%", getHumidity());
        strncpy(humi, num+4, 5);
        printf("Temperature %.2f degC\n\n", getTemperature());
		sprintf(num, "Temp_%.2f_degC", getTemperature());
        strncpy(temp, num+5, 5);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

// hien thi lcd

void LCD_DemoTask(void *param)
{
    char num[20];
    while (true)
    {
        LCD_home();
        LCD_clearScreen();

        sprintf(num, "Hum_%.2f_%%", getHumidity());
        LCD_writeStr(num);
        LCD_setCursor(0, 1);
        sprintf(num, "Temp_%.2f_degC", getTemperature());
        LCD_writeStr(num);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void set_up()
{
	// init value
	strcpy(temp,"00.00");
	strcpy(humi,"00.00");
	strcpy(detect,"OF");
	strcpy(device_1,"ON");
	strcpy(device_2,"OF");
	strcpy(device_3,"OF");
    // set up chan gpio
    esp_rom_gpio_pad_select_gpio(PUSH_BUTTON_PIN_1);
    esp_rom_gpio_pad_select_gpio(PUSH_BUTTON_PIN_2);
    esp_rom_gpio_pad_select_gpio(PUSH_BUTTON_PIN_3);
    esp_rom_gpio_pad_select_gpio(PUSH_BUTTON_PIN_APP);

    esp_rom_gpio_pad_select_gpio(pin_sensor);
    esp_rom_gpio_pad_select_gpio(LED_PIN_1);
    esp_rom_gpio_pad_select_gpio(LED_PIN_2);
    esp_rom_gpio_pad_select_gpio(LED_PIN_3);
    esp_rom_gpio_pad_select_gpio(LED_PIN_APP);
    esp_rom_gpio_pad_select_gpio(speaker_PIN);
   // set in out
    gpio_set_direction(PUSH_BUTTON_PIN_1, GPIO_MODE_INPUT);
    gpio_set_direction(PUSH_BUTTON_PIN_2, GPIO_MODE_INPUT);
    gpio_set_direction(PUSH_BUTTON_PIN_3, GPIO_MODE_INPUT);
    gpio_set_direction(PUSH_BUTTON_PIN_APP, GPIO_MODE_INPUT);

    gpio_set_direction(pin_sensor, GPIO_MODE_INPUT);
    gpio_set_direction(LED_PIN_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PIN_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PIN_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_PIN_APP, GPIO_MODE_OUTPUT);

       gpio_set_direction(speaker_PIN, GPIO_MODE_OUTPUT);
    // set ngat cáº¡nh lÃªn vÃ  mode kÃ©o xuá»‘ng trÃªn button
    gpio_set_intr_type(PUSH_BUTTON_PIN_1, GPIO_INTR_POSEDGE);
    gpio_set_pull_mode(PUSH_BUTTON_PIN_1, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(PUSH_BUTTON_PIN_2 ,GPIO_INTR_POSEDGE);
    gpio_set_pull_mode(PUSH_BUTTON_PIN_2, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(PUSH_BUTTON_PIN_3 ,GPIO_INTR_POSEDGE);
    gpio_set_pull_mode(PUSH_BUTTON_PIN_3, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(PUSH_BUTTON_PIN_APP ,GPIO_INTR_POSEDGE);
    gpio_set_pull_mode(PUSH_BUTTON_PIN_APP, GPIO_PULLDOWN_ONLY);
// set ngáº¯t cáº¡nh xuá»‘ng vÃ  mode kÃ©o lÃªn trÃªn sensor SR602
    gpio_set_intr_type(pin_sensor, GPIO_INTR_NEGEDGE);
    gpio_set_pull_mode(pin_sensor, GPIO_PULLUP_ONLY);
// táº¡o cac ham ngáº¯t tÆ°Æ¡ng á»©ng vá»›i cÃ¡c chÃ¢n
    gpio_install_isr_service(ESP_INR_FLAG_DEFAULT);
    gpio_isr_handler_add(PUSH_BUTTON_PIN_1, button_isr_handler_1, NULL);
    gpio_isr_handler_add(PUSH_BUTTON_PIN_2, button_isr_handler_2, NULL);
    gpio_isr_handler_add(PUSH_BUTTON_PIN_3, button_isr_handler_3, NULL);
    gpio_isr_handler_add(PUSH_BUTTON_PIN_APP, button_isr_handler_5, NULL);
    gpio_isr_handler_add(pin_sensor, button_isr_handler_4, NULL);
}

void app_main(void)
{
	set_up();
	// Initialize NVS.
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	// connect wifi
	LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	connect_wifi();
	
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	xTaskCreate(&DHT_reader_task, "DHT_reader_task", 2048, NULL, 10, NULL);
	xTaskCreate(auto_update, "auto_update", 4096, NULL, 10, NULL);
    xTaskCreate(interrupt_task_1, "interrupt_task_1", 4096, NULL, 10, &ISR_1);
    xTaskCreate(interrupt_task_2, "interrupt_task_2", 4096, NULL, 10, &ISR_2);
    xTaskCreate(interrupt_task_3, "interrupt_task_3", 4096, NULL, 10, &ISR_3);
    xTaskCreate(interrupt_task_4, "interrupt_task_4", 4096, NULL, 10, &ISR_4);
    xTaskCreate(interrupt_task_5, "MCU MODE", 4096, NULL, 10, &ISR_5);

	xTaskCreate(&LCD_DemoTask, "Demo Task", 8192, NULL, 10, NULL);



}
