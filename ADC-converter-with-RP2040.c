#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define LED_B_PIN 11
#define LED_R_PIN 12
#define LED_G_PIN 13
#define JOY_X_PIN 26
#define JOY_Y_PIN 27
#define JOY_BTN_PIN 22
#define BTN_A_PIN 5
#define I2C_SDA 14
#define I2C_SCL 15

#define ADC_MAX 4095
#define ADC_MID 2048
#define PWM_MAX 255

bool led_enabled = true;
bool led_green_state = false;
uint8_t border_style = 0;

void gpio_callback(uint gpio, uint32_t events) {
    static absolute_time_t last_time = {0};
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_time, now) < 200000) return;
    last_time = now;
    
    if (gpio == JOY_BTN_PIN) {
        led_green_state = !led_green_state;
        gpio_put(LED_G_PIN, led_green_state);
        border_style = (border_style + 1) % 3;
    } else if (gpio == BTN_A_PIN) {
        led_enabled = !led_enabled;
        pwm_set_gpio_level(LED_R_PIN, 0);
        pwm_set_gpio_level(LED_B_PIN, 0);
        pwm_set_gpio_level(LED_G_PIN, 0);

    }
}

void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice, PWM_MAX);
    pwm_set_enabled(slice, true);
}

void setup() {
    stdio_init_all();
    
    adc_init();
    adc_gpio_init(JOY_X_PIN);
    adc_gpio_init(JOY_Y_PIN);
    
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    
    gpio_init(JOY_BTN_PIN);
    gpio_set_dir(JOY_BTN_PIN, GPIO_IN);
    gpio_pull_up(JOY_BTN_PIN);
    gpio_set_irq_enabled_with_callback(JOY_BTN_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    
    setup_pwm(LED_R_PIN);
    setup_pwm(LED_B_PIN);
    
    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    //ssd1306_init();
}

void loop() {
    adc_select_input(0);
    uint16_t joy_x = adc_read();
    adc_select_input(1);
    uint16_t joy_y = adc_read();
    
    if (led_enabled) {
        uint16_t pwm_r = (abs((int)joy_x - ADC_MID) * PWM_MAX) / ADC_MID;
        uint16_t pwm_b = (abs((int)joy_y - ADC_MID) * PWM_MAX) / ADC_MID;
        pwm_set_gpio_level(LED_R_PIN, pwm_r);
        pwm_set_gpio_level(LED_B_PIN, pwm_b);
    }
    
    int x_pos = (joy_x * 120) / ADC_MAX;
    int y_pos = (joy_y * 56) / ADC_MAX;
    
    //ssd1306_clear();
    //ssd1306_draw_rect(x_pos, y_pos, 8, 8);
    //ssd1306_update();
    
    sleep_ms(50);
}

int main() {
    setup();
    while (1) {
        loop();
    }
}
