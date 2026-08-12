#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

/*

    For this project I'm using a GY-91 board - a combo of MPU9250 (accelerometer + gyroscope + magnetometer) and BMP280 (barometric pressure/temperature sensor).

    VIN     - NULL
    3V3     - pin_3v3
    GND     - pin_gnd
    SCL     - GPIO_18
    SDA     - GPIO_19
    SDD/SAO - GPIO_16
    NCS     - pin_3v3
    CSB     - GPIO_17
*/

spi_inst_t *SPI_INST =      spi0;

const int PIN_SCK     =      18;
const int PIN_MOSI    =      19;
const int PIN_MISO    =      16;
const int PIN_CS      =      17;
 
const uint32_t FREQ   =      1000000; // 1 MHz
const uint8_t N_BYTES =      8;

const uint8_t CS_ON   =      0;
const uint8_t CS_OFF  =      1;


// SPI configuration
static void spi_config () {

    spi_init(SPI_INST, FREQ);
    // static void spi_set_format (spi_inst_t * spi, uint data_bits, spi_cpol_t cpol, spi_cpha_t cpha, __unused spi_order_t order)
    spi_set_format(SPI_INST, N_BYTES, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // SCK, MOSI, and MISO
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    // chip setlect gpio
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, CS_OFF); // started on high level - deselected 

}

// ====================== BMP280 ======================

// REGISTER ADDRESS

const uint8_t BMP280_ADDR       =   0x77;
const uint8_t BMP280_CHIP_ID    =   0xD0;
const uint8_t BMP280_RESET      =   0xE0;
const uint8_t BMP280_CTRL_MEAS  =   0xF4;
const uint8_t BMP280_CONFIG     =   0xF5;
// Calibration address
const uint8_t BMP280_CALIB      =   0x88; // 0x88 -> 0xA1
// Pressure address 
const uint8_t BMP280_PRES       =   0xF7; // 0xF7 - 0xF9
// Temperature address
const uint8_t BMP280_TEMP       =   0xFA; // 0xFA - 0xFC

// Reading and writing mode values
const uint8_t READ_MODE  = 0x80;
const uint8_t WRITE_MODE = 0x7F;

const uint8_t READ_BACK = 0x00;

// BMP280 write method
void bmp280_write_reg (uint8_t reg, uint8_t value) {

    uint8_t buf[2];
    buf[0] = reg & WRITE_MODE; // set to 0 MSB - WRITE MODE
    buf[1] = value;

    // Writing process - active CS, write, then desactivate CS
    gpio_put(PIN_CS, CS_ON);
    spi_write_blocking(SPI_INST, buf, 2);
    gpio_put(PIN_CS, CS_OFF);

}

// BMP280 read method
void bmp280_read_regs (uint8_t reg, uint8_t *buf, size_t len) {

    uint8_t addr = reg | READ_MODE; // set to 1 MSB - READ MODE

    // Reading process - active CS, write, read (using 0x00 to send back), then desactivate CS
    gpio_put(PIN_CS, CS_ON);
    spi_write_blocking(SPI_INST, &addr, 1);
    spi_read_blocking(SPI_INST, READ_BACK, buf, len); // send 0x00 - BMP280 ignores MOSI during a read-back phase
    gpio_put(PIN_CS, CS_OFF);
}

// Read Chip ID of BMP280
uint8_t bmp280_read_chip_id () {
    uint8_t id;
    bmp280_read_regs(BMP280_CHIP_ID, &id, 1);
    return id;
}

// BMP280 configuration
// (datasheet - table 20 and 23)
void bmp280_configure () {
    bmp280_write_reg(BMP280_CTRL_MEAS, 0x27); // 0b001 001 11 -> osrs_t=1, osrs_p=1, mode=normal
    bmp280_write_reg(BMP280_CONFIG, 0x00); // 0bx000 000 0 -> standby 0.5ms, filter off, not spi3w_en
}

// Calibration coefficients 
// (datasheet - table 17)
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

// Read calibration of BMP280
void bmp280_read_calibration () {
    uint8_t buf[24];
    bmp280_read_regs(BMP280_CALIB, buf, 24);

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

// BMP280 read raw data
// (datasheet - table 24 and 25)
void bmp280_read_raw (int32_t *raw_temp, int32_t *raw_press) {

    uint8_t buf[6];

    bmp280_read_regs(BMP280_PRES, buf, 6);

    // The data are read out in an unsigned 20-bit format both for pressure and for temperature
    // (datasheet - 3.9)
    // Big-ending (MSB first)
    // 0/3- MSB | 1/4 - LSB | 2/5 - XLSB
    *raw_press  = (int32_t)((buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4));
    *raw_temp   = (int32_t)((buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4));
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

// BMP280 loop read function

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

int main()
{
    stdio_init_all();

    spi_config();

    bmp280_read_calibration();
    bmp280_configure();

    while (true) {
        bmp280_reading_loop(1000);
    }
}
