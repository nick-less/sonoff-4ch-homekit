/*
 * Example of using esp-homekit library to control
 * a simple Sonoff 4cha using HomeKit.
 * The esp-wifi-config library is also used in this
 * example. This means you don't have to specify
 * your network's SSID and password before building.
 *
 * In order to flash the sonoff basic you will have to
 * have a 3,3v (logic level) FTDI adapter.
 *
 * To flash this example connect 3,3v, TX, RX, GND
 * in this order, beginning in the (square) pin header
 * next to the button.
 * Next hold down the button and connect the FTDI adapter
 * to your computer. The sonoff is now in flash mode and
 * you can flash the custom firmware.
 *
 * WARNING: Do not connect the sonoff to AC while it's
 * connected to the FTDI adapter! This may fry your
 * computer and sonoff.
 *
 *
 *  GPIO0	Button Channel 1	E-FW
    GPIO2	Pin1 Prog Header	SDA
    GPIO4	Relay + LED Channel 3, Active High	IO4
    GPIO5	Relay + LED Channel 2, Active High	IO5
    GPIO7	Not connected, empty space for a 2.5mm jack (see TH16)	GPIO07
    GPIO8	Not connected, empty space for a 2.5mm jack (see TH16)	GPIO08
    GPIO9	Button Channel 2	IO9
    GPIO10	Button Channel 3	IO10
    GPIO12	Relay + LED Channel 1, Active High	IO12
    GPIO13	Blue WiFi LED, Active Low	PWM1
    GPIO14	Button Channel 4	IO14
    GPIO15	Relay + LED Channel 4, Active High	IO15

 */

#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>

#include "button.h"

// The GPIO pin that is connected to the relay 1 on the Sonoff 4ch.
const int relay_gpio1 = 12;
const int relay_gpio2 = 05;
const int relay_gpio3 = 04;
const int relay_gpio4 = 15;
// The GPIO pin that is connected to the blue LED on the Sonoff 4ch.
const int led_gpio = 13;
// The GPIO pin that is oconnected to the button on the Sonoff 4ch.
#define BUTTON_GPIO1 0
#define BUTTON_GPIO2 9
#define BUTTON_GPIO3 10
#define BUTTON_GPIO4 14

void switch_on_callback1(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void switch_on_callback2(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void switch_on_callback3(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void switch_on_callback4(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void button_callback(uint8_t gpio, button_event_t event);

void relay_write1(bool on) {
    gpio_write(relay_gpio1, on ? 1 : 0);
}

void relay_write2(bool on) {
    gpio_write(relay_gpio2, on ? 1 : 0);
}
void relay_write3(bool on) {
    gpio_write(relay_gpio3, on ? 1 : 0);
}
void relay_write4(bool on) {
    gpio_write(relay_gpio4, on ? 1 : 0);
}

void led_write(bool on) {
    gpio_write(led_gpio, on ? 0 : 1);
}

void reset_configuration_task() {
    //Flash the LED first before we start the reset
    for (int i=0; i<3; i++) {
        led_write(true);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        led_write(false);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    printf("Resetting Wifi Config\n");
    
    wifi_config_reset();
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    printf("Resetting HomeKit Config\n");
    
    homekit_server_reset();
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    printf("Restarting\n");
    
    sdk_system_restart();
    
    vTaskDelete(NULL);
}

void reset_configuration() {
    printf("Resetting Sonoff configuration\n");
    xTaskCreate(reset_configuration_task, "Reset configuration", 256, NULL, 2, NULL);
}

homekit_characteristic_t switch_on1 = HOMEKIT_CHARACTERISTIC_(
    ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(switch_on_callback1)
);
homekit_characteristic_t switch_on2 = HOMEKIT_CHARACTERISTIC_(
    ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(switch_on_callback2)
);
homekit_characteristic_t switch_on3 = HOMEKIT_CHARACTERISTIC_(
    ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(switch_on_callback3)
);
homekit_characteristic_t switch_on4 = HOMEKIT_CHARACTERISTIC_(
    ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(switch_on_callback4)
);

void gpio_init() {
    gpio_enable(led_gpio, GPIO_OUTPUT);
    led_write(false);
    gpio_enable(relay_gpio1, GPIO_OUTPUT);
    gpio_enable(relay_gpio2, GPIO_OUTPUT);
    gpio_enable(relay_gpio3, GPIO_OUTPUT);
    gpio_enable(relay_gpio4, GPIO_OUTPUT);

    gpio_enable(BUTTON_GPIO1, GPIO_INPUT);
    gpio_enable(BUTTON_GPIO2, GPIO_INPUT);
    gpio_enable(BUTTON_GPIO3, GPIO_INPUT);
    gpio_enable(BUTTON_GPIO4, GPIO_INPUT);

    relay_write1(switch_on1.value.bool_value);
    relay_write2(switch_on2.value.bool_value);
    relay_write3(switch_on3.value.bool_value);
    relay_write4(switch_on4.value.bool_value);
}

void switch_on_callback1(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_write1(switch_on1.value.bool_value);
}
void switch_on_callback2(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_write2(switch_on2.value.bool_value);
}
void switch_on_callback3(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_write3(switch_on3.value.bool_value);
}
void switch_on_callback4(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_write4(switch_on4.value.bool_value);
}


void button_callback(uint8_t gpio, button_event_t event) {
    
    switch (event) {
        case button_event_single_press:
        switch (gpio) {
            case BUTTON_GPIO1:
                printf("Toggling relay 1\n");
                switch_on1.value.bool_value = !switch_on1.value.bool_value;
                relay_write1(switch_on1.value.bool_value);
                homekit_characteristic_notify(&switch_on1, switch_on1.value);
            break;
            case BUTTON_GPIO2:
                printf("Toggling relay 2\n");
                switch_on2.value.bool_value = !switch_on2.value.bool_value;
                relay_write2(switch_on2.value.bool_value);
                homekit_characteristic_notify(&switch_on2, switch_on2.value);
            break;
            case BUTTON_GPIO3:
                printf("Toggling relay 3\n");
                switch_on3.value.bool_value = !switch_on3.value.bool_value;
                relay_write3(switch_on3.value.bool_value);
                homekit_characteristic_notify(&switch_on3, switch_on3.value);
            break;
            case BUTTON_GPIO4:
                printf("Toggling relay 4\n");
                switch_on4.value.bool_value = !switch_on4.value.bool_value;
                relay_write4(switch_on4.value.bool_value);
                homekit_characteristic_notify(&switch_on4, switch_on4.value);
            break;
        }
          
            break;
        case button_event_long_press:
            if (gpio == BUTTON_GPIO1) {
                reset_configuration();
            }
            break;
        default:
            printf("Unknown button event: %d\n", event);
    }
}

void switch_identify_task(void *_args) {
    // We identify the Sonoff by Flashing it's LED.
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            led_write(true);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            led_write(false);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    led_write(false);

    vTaskDelete(NULL);
}

void switch_identify(homekit_value_t _value) {
    printf("Switch identify\n");
    xTaskCreate(switch_identify_task, "Switch identify", 128, NULL, 2, NULL);
}

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "Sonoff 4ch");

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            &name,
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "iTEAD"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "037A2BABF19D"),
            HOMEKIT_CHARACTERISTIC(MODEL, "4CH"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1.6"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, switch_identify),
            NULL
        }),
        HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Sonoff 4CH SW1"),
            &switch_on1,
            NULL
        }),
        HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Sonoff 4CH SW2"),
            &switch_on2,
            NULL
        }),
        HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Sonoff 4CH SW3"),
            &switch_on3,
            NULL
        }),
        HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Sonoff 4CH SW4"),
            &switch_on4,
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111"
};

void on_wifi_ready() {
    homekit_server_init(&config);
}

void create_accessory_name() {
    uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);
    
    int name_len = snprintf(NULL, 0, "Sonoff Switch-%02X%02X%02X",
                            macaddr[3], macaddr[4], macaddr[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, "Sonoff Switch-%02X%02X%02X",
             macaddr[3], macaddr[4], macaddr[5]);
    
    name.value = HOMEKIT_STRING(name_value);
}

void user_init(void) {
    uart_set_baud(0, 115200);

    create_accessory_name();
    
    wifi_config_init("sonoff-4ch", NULL, on_wifi_ready);
    gpio_init();

    if (button_create(BUTTON_GPIO1, 0, 4000, button_callback)) {
        printf("Failed to initialize button 1\n");
    }
    if (button_create(BUTTON_GPIO2, 0, 4000, button_callback)) {
        printf("Failed to initialize button 2\n");
    }
    if (button_create(BUTTON_GPIO3, 0, 4000, button_callback)) {
        printf("Failed to initialize button 3\n");
    }
    if (button_create(BUTTON_GPIO4, 0, 4000, button_callback)) {
        printf("Failed to initialize button 4\n");
    }     
}
