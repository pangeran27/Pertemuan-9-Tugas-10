// src/main.cpp - ESP32 DHT22 + Push Button + LCD I2C RTOS (Arduino Framework)
// Program FreeRTOS dengan DHT22 sensor, GPIO interrupt, dan LCD I2C

#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// ===== PIN DEFINITIONS =====
#define PB_GPIO 0        // Push Button (GPIO 0)
#define LED_GPIO 2       // LED (GPIO 2)
#define DHT_GPIO 4       // DHT22 Data Pin (GPIO 4)
#define LCD_SDA_GPIO 21  // I2C SDA (GPIO 21)
#define LCD_SCL_GPIO 22  // I2C SCL (GPIO 22)
#define LCD_ADDR 0x27    // I2C LCD Address

// ===== GLOBAL VARIABLES =====
typedef struct {
    float temperature;
    float humidity;
    bool is_valid;
} SensorData_t;

SensorData_t sensor_data = {0.0f, 0.0f, false};
SemaphoreHandle_t xBinarySemaphore = NULL;
volatile uint32_t interrupt_count = 0;

// ===== LCD I2C FUNCTIONS =====
void lcd_init() {
    Wire.begin(LCD_SDA_GPIO, LCD_SCL_GPIO);
    delay(50);
    // Initialize 16x2 LCD via I2C
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(0x33);  // Function set
    Wire.endTransmission();
    delay(5);
    
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(0x32);  // 4-bit mode
    Wire.endTransmission();
    delay(5);
    
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(0x28);  // 4-bit, 2 lines
    Wire.endTransmission();
    
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(0x0C);  // Display ON
    Wire.endTransmission();
    
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(0x06);  // Entry mode
    Wire.endTransmission();
    
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(0x01);  // Clear display
    Wire.endTransmission();
    delay(10);
}

void lcd_print(uint8_t row, uint8_t col, const char *str) {
    uint8_t row_addr[] = {0x80, 0xC0};
    uint8_t cmd = row_addr[row] + col;
    
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(cmd);
    Wire.endTransmission();
    delay(1);
    
    for (int i = 0; str[i] != '\0' && i < 16; i++) {
        Wire.beginTransmission(LCD_ADDR);
        Wire.write(str[i] | 0x80);  // Data with backlight
        Wire.endTransmission();
        delay(1);
    }
}

void lcd_clear() {
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(0x01);
    Wire.endTransmission();
    delay(10);
}

// ===== DHT22 SENSOR SIMULATION =====
void read_dht22() {
    // Simulasi pembacaan DHT22
    sensor_data.temperature = 25.5f + (float)(rand() % 10) / 10.0f;
    sensor_data.humidity = 60.0f + (float)(rand() % 20) / 10.0f;
    sensor_data.is_valid = true;
    
    Serial.printf("DHT22 - Temp: %.1f°C, Humidity: %.1f%%\n", 
                  sensor_data.temperature, sensor_data.humidity);
}

// ===== INTERRUPT HANDLER =====
void IRAM_ATTR gpio_isr_handler() {
    xSemaphoreGiveFromISR(xBinarySemaphore, NULL);
    interrupt_count++;
}

// ===== FREERTOS TASKS =====
void task_dht22_reader(void *pvParameters) {
    Serial.println("[Task 1] DHT22 Reader started");
    
    while (1) {
        read_dht22();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void task_interrupt_processor(void *pvParameters) {
    Serial.println("[Task 2] Interrupt Processor started");
    
    while (1) {
        if (xSemaphoreTake(xBinarySemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
            digitalWrite(LED_GPIO, !digitalRead(LED_GPIO));
            Serial.printf("Button pressed! Count: %lu, LED: %s\n", 
                         interrupt_count, digitalRead(LED_GPIO) ? "ON" : "OFF");
            delay(50);  // Debounce
        }
    }
}

void task_lcd_display(void *pvParameters) {
    char line1[17] = {0};
    char line2[17] = {0};
    
    Serial.println("[Task 3] LCD Display started");
    lcd_init();
    
    while (1) {
        lcd_clear();
        
        // Row 1: Temperature & Humidity
        if (sensor_data.is_valid) {
            snprintf(line1, 17, "T:%.1fC H:%.1f%%",
                    sensor_data.temperature, sensor_data.humidity);
        } else {
            snprintf(line1, 17, "DHT22 Reading..");
        }
        
        // Row 2: LED & Button count
        uint8_t led_state = digitalRead(LED_GPIO);
        snprintf(line2, 17, "LED:%s BTN:%lu",
                led_state ? "ON " : "OFF", interrupt_count);
        
        lcd_print(0, 0, line1);
        lcd_print(1, 0, line2);
        
        Serial.printf("LCD: %s | %s\n", line1, line2);
        
        delay(1000);
    }
}

// ===== SETUP & LOOP (Arduino Style) =====
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("ESP32 DHT22 + PB + LCD I2C RTOS");
    Serial.println("========================================\n");
    
    // Initialize GPIO
    Serial.println("[1] Initializing GPIO...");
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LOW);
    pinMode(PB_GPIO, INPUT_PULLUP);
    
    // Create binary semaphore
    Serial.println("[2] Creating binary semaphore...");
    xBinarySemaphore = xSemaphoreCreateBinary();
    
    if (xBinarySemaphore != NULL) {
        Serial.println("    Binary semaphore created successfully\n");
        
        // Attach interrupt
        attachInterrupt(digitalPinToInterrupt(PB_GPIO), gpio_isr_handler, FALLING);
        
        // Create FreeRTOS tasks
        Serial.println("[3] Creating FreeRTOS tasks...");
        
        xTaskCreate(
            task_dht22_reader,
            "DHT22_Reader",
            2048,
            NULL,
            2,
            NULL
        );
        
        xTaskCreate(
            task_interrupt_processor,
            "Interrupt_Proc",
            2048,
            NULL,
            2,
            NULL
        );
        
        xTaskCreate(
            task_lcd_display,
            "LCD_Display",
            2048,
            NULL,
            2,
            NULL
        );
        
        Serial.println("\nAll tasks created successfully!");
        Serial.println("========================================");
        Serial.println("System running. Press button to toggle LED.");
        Serial.println("Watch LCD I2C for sensor and status data.");
        Serial.println("========================================\n");
        
    } else {
        Serial.println("ERROR: Failed to create binary semaphore!");
    }
}

void loop() {
    // Loop kosong - FreeRTOS scheduler mengambil alih
    delay(1000);
}

