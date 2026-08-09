#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/*

    For this project I'm using a GY-91 board - a combo of MPU9250 (accelerometer + gyroscope + magnetometer) and BMP280 (barometric pressure/temperature sensor).

    VIN     - NULL
    3V3     - pin_3v3
    GND     - pin_gnd
    SCL     - GPIO_27
    SDA     - GPIO_26
    SDD/SAO - pin_3v3
    NCS     - pin_3v3
    CSB     - pin_3v3

    Using i2c_scan() I found 0x69 (MPU9250) and 0x77 (BMP280) for the GY-91 board.
*/


// ================= I2C SCAN and configuration ==============

i2c_inst_t *I2C_PORT = i2c1;
const uint SDA = 26;
const uint SCL = 27;
const uint BAUDRATE = 400 * 1000; // 400kHz

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

const uint8_t MAX_ADDRESS = 10;

// Scanning method
int i2c_scan (uint8_t *addrs, int max_addrs) {

    int addr_count = 0;

    printf("\nScanning I2C bus .");
    for (int addr = 0; addr < (1<<7); addr++) {
        
        uint8_t rxdata;
        int ret; 
        
        if (!reserved_addr(addr)) {
            ret = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false);
            if (ret >= 0) {
                printf("\nFound device at 0x%02X\n", addr);
                if (addr_count < max_addrs) {
                    addrs[addr_count] = (uint8_t)addr;
                    addr_count++;
                }
            } else {
                printf(".");
            }
        }
    }

    if (!addr_count) {
        printf("\nNo device was found.\nPlease check wire connection!\n");
    } else {
        printf("\nScan complete!\n%d device(s) found.\n", addr_count);
    }

    return addr_count;
}

// I2C configuration
static void i2c_config (i2c_inst_t *port, uint sda, uint scl, uint baudrate) {
    i2c_init(port, baudrate);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);
}

// ====================== BMP280 ======================

const uint8_t BMP280_ADDR       =   0x77;
const uint8_t BMP280_CHIP_ID    =   0xD0;
const uint8_t BMP280_RESET      =   0xE0;
const uint8_t BMP280_CTRL_MEAS  =   0xF4;
const uint8_t BMP280_CONFIG     =   0xF5;

// Calibration address
const uint8_t BMP280_CALIB      =   0x88; // 0x88 -> 0xA1

// Pressure address 
const uint8_t BMP280_PRES       =   0xF7;
// Temperature address
const uint8_t BMP280_TEMP       =   0xFA;

// 
uint8_t bmp280_read_chip_id () {

    uint8_t reg = BMP280_CHIP_ID;
    uint8_t id;

    // (i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop)
    i2c_write_blocking(I2C_PORT, BMP280_ADDR, &reg, 1, true); 
    // (i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop)
    i2c_read_blocking(I2C_PORT, BMP280_ADDR, &id, 1, false);

    return id;
}

// 

// Calibration coefficients (from datasheet, signedness matters!)
uint16_t dig_T1;
int16_t  dig_T2;
int16_t  dig_T3;

uint16_t dig_P1;
int16_t  dig_P2;
int16_t  dig_P3;
int16_t  dig_P4;
int16_t  dig_P5;
int16_t  dig_P6;
int16_t  dig_P7;
int16_t  dig_P8;
int16_t  dig_P9;

void bmp280_read_calibration() {
    uint8_t reg = 0x88;
    uint8_t buf[24];

    i2c_write_blocking(I2C_PORT, BMP280_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, BMP280_ADDR, buf, 24, false);

    // Little-endian (LSB first)
    dig_T1 = (uint16_t)(buf[1] << 8 | buf[0]);
    dig_T2 = (int16_t)(buf[3] << 8 | buf[2]);
    dig_T3 = (int16_t)(buf[5] << 8 | buf[4]);

    dig_P1 = (uint16_t)(buf[7] << 8 | buf[6]);
    dig_P2 = (int16_t)(buf[9] << 8 | buf[8]);
    dig_P3 = (int16_t)(buf[11] << 8 | buf[10]);
    dig_P4 = (int16_t)(buf[13] << 8 | buf[12]);
    dig_P5 = (int16_t)(buf[15] << 8 | buf[14]);
    dig_P6 = (int16_t)(buf[17] << 8 | buf[16]);
    dig_P7 = (int16_t)(buf[19] << 8 | buf[18]);
    dig_P8 = (int16_t)(buf[21] << 8 | buf[20]);
    dig_P9 = (int16_t)(buf[23] << 8 | buf[22]);
}

//
void bmp280_configure() {
    uint8_t buf[2];

    // CTRL_MEAS (0xF4): temp oversampling x1, pressure oversampling x1, normal mode
    buf[0] = 0xF4;
    buf[1] = 0x27; // 0b001 001 11 -> osrs_t=1, osrs_p=1, mode=normal
    i2c_write_blocking(I2C_PORT, BMP280_ADDR, buf, 2, false);

    // CONFIG (0xF5): standby 0.5ms, filter off
    buf[0] = 0xF5;
    buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, BMP280_ADDR, buf, 2, false);
}

//
void bmp280_read_raw (int32_t *raw_temp, int32_t *raw_press) {

    uint8_t buf[6];

    i2c_write_blocking(I2C_PORT, BMP280_ADDR, &BMP280_PRES, 1, true);

    i2c_read_blocking(I2C_PORT, BMP280_ADDR, buf, 6, false);

    // Big-ending (MSB first)
    *raw_press = (int32_t)((buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4));
    *raw_temp  = (int32_t)((buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4));

}

// Compensation formulas (following the Bosch recommendation)

// ----------------- Temperature -------------------------
int32_t t_fine;
// Returns temperature in units of 0.01°C (e.g. 5123 = 51.23°C)
int32_t bmp280_compensate_temp(int32_t adc_T) {
    int32_t var1, var2;

    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;

    t_fine = var1 + var2;

    int32_t T = (t_fine * 5 + 128) >> 8;
    return T;
}

// ---------------- Pressure ------------------------------

// Returns pressure in Pa, in Q24.8 format (24 integer bits, 8 fractional bits)
// Divide the result by 256 to get Pa as a normal integer, or by 25600.0f for hPa as a float
uint32_t bmp280_compensate_pressure(int32_t adc_P) {
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) {
        return 0; // avoid divide by zero
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    return (uint32_t)p;
}

int32_t raw_temp, raw_press;
int32_t temp, press;
float temp_c, press_hpa;

void bmp280_reading_loop (int32_t time_ms) {

    bmp280_read_raw(&raw_temp, &raw_press);

    temp = bmp280_compensate_temp(raw_temp);
    press = bmp280_compensate_pressure(raw_press);

    temp_c = temp / 100.0f;
    press_hpa = (press / 256.0f) / 100.0f;

    printf("[BMP280]\nTemperature: %.2f °C\nPressure: %.2f hPa\n-------\n", temp_c, press_hpa);
    sleep_ms(time_ms);
}

// ====================================================

const bool SCAN = false;
const int TIME_WAITING_S = 5;

int main() {

    stdio_init_all();
    
    // I2C init
    i2c_config(I2C_PORT, SDA, SCL, BAUDRATE);

    // Scanning address 
    if (SCAN) {

        // Slow initialization 
        printf("Device under initialization!\n");
        printf("waiting .");
        for (uint8_t k = 0; k < TIME_WAITING_S; k++) {
            printf(".");
            sleep_ms(1000);
        }

        uint8_t addrs_found[MAX_ADDRESS];
    
        int addrs_n = i2c_scan(addrs_found, MAX_ADDRESS);
        printf("Listing devices found:\n");
        for (uint8_t j = 0; j < addrs_n; j++) {
            printf("* 0x%02X\n", addrs_found[j]);
        }
    
        int _id = bmp280_read_chip_id();
        printf("BMP280 decimal = %d\n", _id); // 0d88 = 0x58
    }

    bmp280_read_calibration();
    bmp280_configure();

    while (true) {
        bmp280_reading_loop(1000);
    }
}