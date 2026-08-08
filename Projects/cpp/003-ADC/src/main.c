#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

const uint ADC = 0;

static void config_adc (uint _adc) {

    uint adc_gpio[3] = {26, 27, 28};

    // Initialize ADC hardware
    adc_init();

    // Make sure GPIO is high-impedance
    adc_gpio_init(adc_gpio[_adc]);

    // Select ADC 
    adc_select_input(_adc);
}

// 12-bit resolution - assume max value = ADC_REF = 3.3 V
const float conversion_factor = 3.3f / (1 << 12);

float dig_to_volt (uint bit_value) {
    return bit_value * conversion_factor;
}

volatile uint16_t adc_value = 0;

int main()
{
    stdio_init_all();

    config_adc(ADC);

    while (1) {
        adc_value = adc_read();
        printf("ADC %d : %d [%f V]\n", ADC, adc_value, dig_to_volt(adc_value));
        sleep_ms(1000);
    }
}
