#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

i2c_inst_t *I2C_PORT = i2c1;
const uint SDA = 26;
const uint SCL = 27;
const uint BAUDRATE = 400 * 1000; // 400kHz

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

// Scanning method
int i2c_scan () {
    printf("Scanning I2C bus .");
    for (int addr = 0; addr < (1<<7); addr++) {
        
        uint8_t rxdata;
        int ret; 
        
        if (!reserved_addr(addr)) {
            ret = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false);
            if (ret >= 0) {
                printf("Found device at 0x%02X\n", addr);
                return addr;
            } else {
                printf(".");
            }
        }
    }
    printf("\nNo device was found.\nPlease check wire connection!\n");
    return 255;
}

// I2C configuration
static void i2c_config (i2c_inst_t *port, uint sda, uint scl, uint baudrate) {
    i2c_init(port, baudrate);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);
}

int main()
{
    stdio_init_all();
    
    sleep_ms(2000);

    i2c_config(I2C_PORT, SDA, SCL, BAUDRATE);

    printf("I2C configured");
    sleep_ms(2000);

    int addr = i2c_scan();

    while (true) {
        tight_loop_contents();
    }
}
