#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

const uint LED = 8;
const uint BUTTON = 7;
const int WAIT_INIT = 2000;
volatile bool led_state = false;

int64_t timer_alarm_initialization (alarm_id_t id, __unused void *user_data) {
    printf("TIMER ALARM HAS BEEN STARTED\n");
    return 0; // if 0 - the alarm will not reschedule (one-shot)| if positive - reschedule after that many microseconds | if negative - reschedule at that exact interval from the original target time
}

void repeating_timer_alarm (__unused struct repeating_timer *t) {
    led_state = !led_state;
    gpio_put(LED, led_state);
    printf(led_state ? "WAKE UP!\n" : "FALL ASLEEP\n");
}

// Create a repeating alarm
struct repeating_timer timer;

static void config_timers () {

    // Create a one time alarm
    add_alarm_in_ms(WAIT_INIT, timer_alarm_initialization, NULL, false);

    // If the time delay is > 0 then this is the delay between the previous callback ending and the next starting.
    // If the time delay is < 0 then the time will run exactly after been called
    add_repeating_timer_ms(-1000, repeating_timer_alarm, NULL, &timer);
    

}

const uint32_t DEBOUNCE = 200;
volatile uint32_t last_button_time = 0;

void button_callback (uint gpio, uint32_t events) {
    if (gpio == BUTTON) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_button_time > DEBOUNCE) {
            last_button_time = now;
            if (gpio_get(gpio)) {
                printf("Button pressed!\n");
            }
        }
    }
}

static void config_gpios () {

    gpio_init(LED);
    gpio_set_dir(LED, true);
    gpio_put(LED, led_state);

    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, false); // true = output | false = input
    // gpio_pull_down(BUTTON);
    gpio_pull_up(BUTTON);

    // Interruption
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_callback);
}

int main()
{
    stdio_init_all();

    config_timers();
    config_gpios();
    
    while (1) {
        ;;
    }
}
