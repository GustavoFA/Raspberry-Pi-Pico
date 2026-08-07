#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#define LED 7

volatile bool led_state = false;
const int led_delay = 3000; // miliseconds

void vBlinkTask () {
    for (;;) {
        led_state = !led_state;
        gpio_put(LED, led_state);
        vTaskDelay(pdMS_TO_TICKS(led_delay));
    }
}

void vPrintTask () {
    for (;;) {
        printf(led_state ? "Breathe in!\n" : "Breathe out!\n");
        vTaskDelay(pdMS_TO_TICKS(led_delay));
    }
}

static void config_gpio () {
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
}

int main() {

    stdio_init_all(); // Initialize standard I/O for printf

    config_gpio();

    // configMINIMAL_STACK_SIZE = 256

    xTaskCreate(vPrintTask, "Print Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    xTaskCreate(vBlinkTask, "Blink Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) {
        ;;
    }

}
