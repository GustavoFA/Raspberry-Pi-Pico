#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

uart_inst_t *UART_INST = uart0;

const int UART_IRQ = 20; // UART0_IRQ = 20

const uint BAUDRATE = 115200;

const int RX_PIN = 1;
const int TX_PIN = 0;

const uint DATA_BITS = 8;
const uint STOP_BITS = 1;
uart_parity_t PARITY = UART_PARITY_NONE;

const uint8_t RX_BUFFER_SIZE = 32;

volatile char rx_buffer[32];
volatile uint8_t rx_index = 0;
volatile bool line_ready = false;

// ISR UART RX
void on_uart_rx () {
    
    while (uart_is_readable(UART_INST)) {
        
        // read a single character
        uint8_t _read_byte = uart_getc(UART_INST);
        
        if (_read_byte == '\n' || _read_byte == '\r') {
            if (rx_index > 0) {
                rx_buffer[rx_index] = '\0'; // null-terminate the string
                line_ready = true;
            }
        } else if (rx_index < RX_BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = _read_byte;
        }

    }

}

// Configure UART
static void config_uart () {

    // Init UART
    uart_init(UART_INST, BAUDRATE);

    // Set GPIOs to UART function
    gpio_set_function(RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(TX_PIN, GPIO_FUNC_UART);

    // standard 8N1  - 8 data bits, 1 stop bit, and no parity
    // (uart_inst_t *uart, uint data_bits, uint stop_bits, uart_parity_t parity)
    uart_set_format(UART_INST, DATA_BITS, STOP_BITS, PARITY);
    
    // Disable hardware flow control (CTS/RTS)
    // (uart_inst_t *uart, bool cts, bool rts)
    uart_set_hw_flow(UART_INST, false, false);

    // Turn off the FIFO - interrupt per received byte
    // (uart_inst_t *uart, bool enabled)
    uart_set_fifo_enabled(UART_INST, false);

    // Register handler for UART0 interrupt line
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Enable the UART to actually generate an interrupt on RX
    // (uart_inst_t *uart, bool rx_has_data, bool tx_needs_data)
    uart_set_irqs_enabled(UART_INST, true, false); // Interrupt only on RX
} 

const char SYS_PROMPT[] = "UART echo test ready. Send something:\n";
const uint32_t HEARTBEAT_MS = 3000;
volatile uint32_t last_time_heartbeat = 0;

void uart_heartbeat (uint32_t time_ms) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_time_heartbeat >= time_ms) {
        uart_puts(UART_INST, SYS_PROMPT);
        last_time_heartbeat = now;
    }
}

void uart_echo () {
    if (line_ready) {
        uart_puts(UART_INST, (char *)rx_buffer);
        uart_puts(UART_INST, "\r\n");

        rx_index = 0;
        line_ready = false;
    }
}

int main() {
    stdio_init_all();

    sleep_ms(2000); // wait some time for USB serial init correctly
    config_uart();

    uart_puts(UART_INST, SYS_PROMPT);

    while (true) {
        uart_echo();
        uart_heartbeat(3000);
    }
    

}
