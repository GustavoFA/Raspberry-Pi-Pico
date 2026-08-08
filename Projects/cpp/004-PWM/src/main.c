#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

static void config_pwm (uint _gpio) {

    // Set gpio to the PWM
    gpio_set_function(_gpio, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(_gpio);

    pwm_config config = pwm_get_default_config();

    pwm_init(slice_num, &config, true);

}

const uint GPIO = 7;

int main()
{
    stdio_init_all();

    config_pwm(GPIO);

    while (1) {
        printf("Increasing light!\n");
        sleep_ms(1000);
        for (uint32_t k = 0; k < 1<<16; k += 256) {
            pwm_set_gpio_level(GPIO, k);
            sleep_ms(1);
        }

        printf("Decreasing light!\n");
        sleep_ms(1000);
        for (uint32_t k = 1<<16; k > 0; k -= 256) {
            pwm_set_gpio_level(GPIO, k);
            sleep_ms(1);
        }
    }
    
}
