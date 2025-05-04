#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "pio.h"
#include "display.h"
#include <stdio.h>

#define botaoA 5
#define botaoB 6
#define ledVerde 11
#define ledAzul 12
#define ledVermelho 13

static volatile uint32_t last_time_A = 0; // Armazena o tempo do último evento (em microssegundos)
static volatile uint32_t last_time_B = 0;
volatile bool modoNoturno = false;

bool debounce(volatile uint32_t *last_time, uint32_t debounce_time)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - *last_time > debounce_time)
    {
        *last_time = current_time;
        return true;
    }
    return false;
}

// Função de interrupção com debouncing
void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Alternar o modo noturno
    if (gpio == botaoA && debounce(&last_time_A, 300000))
    {
        last_time_A = current_time;
        modoNoturno = !modoNoturno;
    }
}

void vSemaforoNormal()
{
    while (true)
    {
        gpio_put(ledVerde, true); // Garantir que o verde vai ser ativado assim que entrar no laço
        gpio_put(ledVermelho, false);
        if (!modoNoturno)
        {
            gpio_put(ledVerde, true);
            vTaskDelay(pdMS_TO_TICKS(1000));
            if (modoNoturno)
                continue;

            gpio_put(ledVermelho, true);
            vTaskDelay(pdMS_TO_TICKS(1000));
            if (modoNoturno)
                continue;

            gpio_put(ledVerde, false);
            vTaskDelay(pdMS_TO_TICKS(1000));
            if (modoNoturno)
                continue;

            gpio_put(ledVermelho, false);
        }
        else
        {
            gpio_put(ledVerde, true);
            gpio_put(ledVermelho, true);
            vTaskDelay(pdMS_TO_TICKS(1500));
            if (!modoNoturno)
                continue;

            gpio_put(ledVerde, false);
            gpio_put(ledVermelho, false);
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
    }
}

void vMatrizLeds()
{
    while (true)
    {
        if (!modoNoturno)
        {
            set_one_led(0, 0, 20, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
            if (modoNoturno)
                continue;

            set_one_led(1, 20, 20, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
            if (modoNoturno)
                continue;

            set_one_led(2, 20, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        else
        {
            set_one_led(1, 20, 20, 0);
            vTaskDelay(pdMS_TO_TICKS(1500));
            if (!modoNoturno)
                continue;
            set_one_led(4, 0, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
    }
}

void vBuzzer()
{
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void vDisplay()
{
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

int main()
{
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);

    gpio_set_irq_enabled_with_callback(botaoA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(ledAzul);
    gpio_set_dir(ledAzul, GPIO_OUT);

    gpio_init(ledVerde);
    gpio_set_dir(ledVerde, GPIO_OUT);

    gpio_init(ledVermelho);
    gpio_set_dir(ledVermelho, GPIO_OUT);

    stdio_init_all();
    initializePio();
    initI2C();

    xTaskCreate(vSemaforoNormal, "Semaforo Normal", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vMatrizLeds, "Matriz Leds", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBuzzer, "Buzzer", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplay, "Display", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
