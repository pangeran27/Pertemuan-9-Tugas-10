// include/config.h untuk ESP32_02_External_Interrupt_RTOS
// Header guard untuk mencegah include berulang
#ifndef CONFIG_H
#define CONFIG_H

// ===== GPIO PIN DEFINITIONS =====
// Mendefinisikan GPIO untuk interrupt input (push button)
#define PB_GPIO 0

// Mendefinisikan GPIO untuk LED indikator
#define LED_GPIO 2

// Mendefinisikan GPIO untuk sensor DHT22 (data pin)
#define DHT_GPIO 4

// Mendefinisikan GPIO untuk I2C LCD (SDA - Serial Data)
#define LCD_SDA_GPIO 21

// Mendefinisikan GPIO untuk I2C LCD (SCL - Serial Clock)
#define LCD_SCL_GPIO 22

// ===== COMMUNICATION SETTINGS =====
// Mendefinisikan baud rate UART untuk komunikasi serial
#define UART_BAUD_RATE 115200

// Mendefinisikan I2C bus frequency (100kHz standard, 400kHz fast)
#define I2C_FREQ_HZ 100000

// Mendefinisikan I2C address untuk LCD (default 0x27 untuk 16x2 LCD)
#define LCD_ADDR 0x27

// ===== TASK SETTINGS =====
// Mendefinisikan delay untuk task sensor DHT dalam tick FreeRTOS (1000ms)
#define DHT_TASK_DELAY pdMS_TO_TICKS(2000)

// Mendefinisikan ukuran stack untuk task
#define TASK_STACK_SIZE 2048

// Mendefinisikan prioritas untuk task pemroses interrupt
#define TASK_PRIORITY 2

// Mendefinisikan prioritas untuk task sensor
#define DHT_PRIORITY 2

// ===== GPIO LEVEL DEFINITIONS =====
// Mendefinisikan nilai HIGH untuk GPIO
#define GPIO_HIGH 1

// Mendefinisikan nilai LOW untuk GPIO
#define GPIO_LOW 0

// Mengakhiri header guard
#endif
