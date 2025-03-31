#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/rtc.h"
#include <string.h>
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

const int X_PIN_ESQUERDA =  12;
const int Y_PIN_ESQUERDA =  13;

const int X_PIN_DIREITA =  18;
const int Y_PIN_DIREITA =  19;

volatile uint64_t start_us_esquerda = 0;
volatile uint64_t end_us_esquerda = 0;

volatile uint64_t start_us_direita = 0;
volatile uint64_t end_us_direita = 0;

volatile bool timer_fired_esquerda = false;
volatile bool timer_fired_direita = false;
volatile int echo_esquerda = 0;
volatile int echo_direita = 0;

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    timer_fired_esquerda = true;
    return 0;
}

int64_t alarm_callback_direita(alarm_id_t id, void *user_data) {
    timer_fired_direita = true;
    return 0;
}

void btn_callback(uint gpio, uint32_t events) {
    if (gpio == X_PIN_ESQUERDA) {
        if (events == 0x4) { //fall
            end_us_esquerda = to_us_since_boot(get_absolute_time());
        } else if (events == 0x8) { //rise
            start_us_esquerda = to_us_since_boot(get_absolute_time());
        }
        echo_esquerda = 1;
    }
    if (gpio == X_PIN_DIREITA) {
        if (events == 0x4) { //fall
            end_us_direita = to_us_since_boot(get_absolute_time());
        } else if (events == 0x8) { //rise
            start_us_direita = to_us_since_boot(get_absolute_time());
        }
        echo_direita = 1;
    }
}

int main() {    
    stdio_init_all();

    gpio_init(Y_PIN_ESQUERDA);
    gpio_set_dir(Y_PIN_ESQUERDA, GPIO_OUT);

    gpio_init(X_PIN_ESQUERDA);
    gpio_set_dir(X_PIN_ESQUERDA, GPIO_IN);

    gpio_set_irq_enabled_with_callback(X_PIN_ESQUERDA, (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL), true, &btn_callback);

    gpio_init(Y_PIN_DIREITA);
    gpio_set_dir(Y_PIN_DIREITA, GPIO_OUT);

    gpio_init(X_PIN_DIREITA);
    gpio_set_dir(X_PIN_DIREITA, GPIO_IN);

    gpio_set_irq_enabled(X_PIN_DIREITA, (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL), true);

    while (true) {
        sleep_ms(1000);

        gpio_put(Y_PIN_ESQUERDA, 1);
        gpio_put(Y_PIN_DIREITA, 1);
        sleep_us(10);
        gpio_put(Y_PIN_ESQUERDA, 0);
        gpio_put(Y_PIN_DIREITA, 0);
        alarm_id_t alarm_direita = add_alarm_in_ms(5000, alarm_callback_direita, NULL, false);
        alarm_id_t alarm_esquerda = add_alarm_in_ms(5000, alarm_callback, NULL, false);
        uint64_t dt;
        uint64_t dt2;

        if (echo_esquerda){
            cancel_alarm(alarm_esquerda);
        }

        if (echo_direita){
            cancel_alarm(alarm_direita);
        }
        
        if (timer_fired_esquerda) {
            printf("Sensor 1 - dist: falha \n");
            timer_fired_esquerda = 0;
        } else {
            echo_esquerda = 0;
            dt = end_us_esquerda - start_us_esquerda; 
            int distancia_1 = (int) ((dt * 0.0343) / 2.0);
            printf("Sensor 1 - dist: %d cm \n",distancia_1);
        }

        if (timer_fired_direita) {
            printf("Sensor 2 - dist: falha \n");
            timer_fired_direita = 0;
        } else {
            echo_direita = 0;
            dt2 = end_us_direita - start_us_direita; 
            int distancia_2 = (int) ((dt2 * 0.0343) / 2.0);
            printf("Sensor 2 - dist: %d cm \n",distancia_2);
        }
    }
}