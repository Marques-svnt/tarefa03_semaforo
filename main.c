#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "buzzer.h"
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

void vBotaoA()
{
    while (true)
    {
        uint32_t current_time = to_us_since_boot(get_absolute_time());

        // Alternar o modo noturno
        if (gpio_get(botaoA) == false && debounce(&last_time_A, 300000))
        {
            last_time_A = current_time;
            modoNoturno = !modoNoturno;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vSemaforoNormal()
{
    while (true)
    {
        gpio_put(ledVerde, true);
        gpio_put(ledVermelho, false);

        if (!modoNoturno)
        {
            // Verde ligado por 4s
            for (int t = 0; t < 4000 && !modoNoturno; t += 500)
                vTaskDelay(pdMS_TO_TICKS(500));
            if (modoNoturno)
                continue;

            // Vermelho ligado junto por 4s
            gpio_put(ledVermelho, true);
            for (int t = 0; t < 4000 && !modoNoturno; t += 500)
                vTaskDelay(pdMS_TO_TICKS(500));
            if (modoNoturno)
                continue;

            // Verde desliga, vermelho continua 4s
            gpio_put(ledVerde, false);
            for (int t = 0; t < 4000 && !modoNoturno; t += 500)
                vTaskDelay(pdMS_TO_TICKS(500));
            if (modoNoturno)
                continue;

            // Vermelho desliga
            gpio_put(ledVermelho, false);
        }
        else
        {
            // Piscar sincronizado no modo noturno (2s ON, 2s OFF)
            gpio_put(ledVerde, true);
            gpio_put(ledVermelho, true);
            for (int t = 0; t < 2000 && modoNoturno; t += 500)
                vTaskDelay(pdMS_TO_TICKS(500));
            if (!modoNoturno)
                continue;

            gpio_put(ledVerde, false);
            gpio_put(ledVermelho, false);
            for (int t = 0; t < 2000 && modoNoturno; t += 500)
                vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void vMatrizLeds()
{
    while (true)
    {
        if (!modoNoturno)
        {
            set_one_led(0, 0, 20, 0); // Verde
            for (int t = 0; t < 4000 && !modoNoturno; t += 500)
                vTaskDelay(pdMS_TO_TICKS(500));
            if (modoNoturno)
                continue;

            set_one_led(1, 20, 20, 0); // Amarelo
            for (int t = 0; t < 4000 && !modoNoturno; t += 500)
                vTaskDelay(pdMS_TO_TICKS(500));
            if (modoNoturno)
                continue;

            set_one_led(2, 20, 0, 0); // Vermelho
            for (int t = 0; t < 4000 && !modoNoturno; t += 500)
                vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
            set_one_led(1, 20, 20, 0); // Luz amarela ligada
            for (int t = 0; t < 2000 && modoNoturno; t += 500)
                vTaskDelay(pdMS_TO_TICKS(500));
            if (!modoNoturno)
                continue;

            set_one_led(4, 0, 0, 0); // Apaga
            for (int t = 0; t < 2000 && modoNoturno; t += 500)
                vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void vBuzzer()
{
    buzzer_stop(); // Garante que começa desligado

    while (true)
    {
        if (!modoNoturno)
        {
            // Beep 1000 Hz por 1 segundo
            buzz(1000);
            for (int t = 0; t < 1000 && !modoNoturno; t += 100)
                vTaskDelay(pdMS_TO_TICKS(100));
            buzzer_stop();
            for (int t = 0; t < 3000 && !modoNoturno; t += 100)
                vTaskDelay(pdMS_TO_TICKS(100));
            if (modoNoturno)
                continue;

            // Beep intermitente (500 Hz, 200ms ON/OFF, total ~4s)
            for (int i = 0; i < 10 && !modoNoturno; i++)
            {
                buzz(500);
                for (int t = 0; t < 200 && !modoNoturno; t += 100)
                    vTaskDelay(pdMS_TO_TICKS(100));
                buzzer_stop();
                for (int t = 0; t < 200 && !modoNoturno; t += 100)
                    vTaskDelay(pdMS_TO_TICKS(100));
            }
            if (modoNoturno)
                continue;

            // Beep contínuo e curto (800 Hz, 500ms ON, 1.5s OFF) ×2 = ~4s
            for (int i = 0; i < 2 && !modoNoturno; i++)
            {
                buzz(800);
                for (int t = 0; t < 500 && !modoNoturno; t += 100)
                    vTaskDelay(pdMS_TO_TICKS(100));
                buzzer_stop();
                for (int t = 0; t < 1500 && !modoNoturno; t += 100)
                    vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
        else
        {
            // Modo noturno: beep lento a cada 4s (2s ON + 2s OFF)
            buzz(400);
            for (int t = 0; t < 2000 && modoNoturno; t += 100)
                vTaskDelay(pdMS_TO_TICKS(100));
            buzzer_stop();
            for (int t = 0; t < 2000 && modoNoturno; t += 100)
                vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void vDisplay()
{
    while (true)
    {
        // Títulos fixos no display
        display("VERDE", 52, 16);
        display("AMARELO", 44, 26);
        display("VERMELHO", 40, 36);

        if (modoNoturno)
        {
            display("Modo Noturno", 16, 48);

            // Modo noturno: amarelo piscando
            display("^", 24, 16); // Verde apagado
            display(">", 24, 26); // Amarelo aceso
            display("^", 24, 36); // Vermelho apagado

            // Espera responsiva por 1 segundo
            for (int i = 0; i < 2000; i += 100)
            {
                if (!modoNoturno)
                    break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            if (!modoNoturno)
                continue;

            display("^", 24, 16); // Verde apagado
            display("^", 24, 26); // Amarelo apagado
            display("^", 24, 36); // Vermelho apagado

            for (int i = 0; i < 2000; i += 100)
            {
                if (!modoNoturno)
                    break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            continue;
        }
        else
        {
            display("            ", 16, 48); // Limpa aviso "Modo Noturno"

            // Verde aceso
            display(">", 24, 16);
            display("^", 24, 26);
            display("^", 24, 36);
            for (int i = 0; i < 4000; i += 100)
            {
                if (modoNoturno)
                    break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            if (modoNoturno)
                continue;

            // Amarelo aceso
            display("^", 24, 16);
            display(">", 24, 26);
            display("^", 24, 36);
            for (int i = 0; i < 4000; i += 100)
            {
                if (modoNoturno)
                    break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            if (modoNoturno)
                continue;

            // Vermelho aceso
            display("^", 24, 16);
            display("^", 24, 26);
            display(">", 24, 36);
            for (int i = 0; i < 4000; i += 100)
            {
                if (modoNoturno)
                    break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }
}

int main()
{
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);

    gpio_init(ledAzul);
    gpio_set_dir(ledAzul, GPIO_OUT);

    gpio_init(ledVerde);
    gpio_set_dir(ledVerde, GPIO_OUT);

    gpio_init(ledVermelho);
    gpio_set_dir(ledVermelho, GPIO_OUT);

    stdio_init_all();
    initializePio();
    initI2C();
    buzzer_init();

    xTaskCreate(vBotaoA, "Botão A", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
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
