# Sistema de Semáforo Inteligente com Modo Noturno e Acessibilidade

Este projeto implementa um **semáforo inteligente** utilizando o microcontrolador **RP2040** da placa BitDogLab, **FreeRTOS** e periféricos:

* **LED RGB** (GPIO)
* **Matriz de LEDs WS2812B** (PIO)
* **Display OLED SSD1306** (I2C)
* **Buzzer** (PWM)
* **Botões** A e B (GPIO + interrupção com debounce)

O sistema opera em dois modos:

1. **Modo Normal**

   * Ciclo: **Verde → Amarelo → Vermelho**, cada um com duração de 4 segundos.
   * **LED RGB**, **matriz de LEDs** e **buzzer** sincronizados:

     * **Verde**: beep curto e indicação sonora de 1s “pode atravessar”.
     * **Amarelo**: beep rápido intermitente por 4s “atenção”.
     * **Vermelho**: beep contínuo curto (500ms on, 1.5s off) repetido duas vezes em 4s “pare”.
   * **Display OLED** mostra o estado atual com setas (`>`) e texto.

2. **Modo Noturno**

   * **Semáforo fixo** no amarelo piscando 1s on / 1s off.
   * **Buzzer** emite tom grave intermitente (2s on / 2s off).
   * Reduz impacto visual e sonoro.

## Botões

* **Botão A**: alterna entre Modo Normal e Modo Noturno (com debounce de 300ms).

## Requisitos de Entrega

1. **Ficha da Tarefa**: preenchida conforme modelo.
2. **Código-fonte**: link para repositório contendo todos os arquivos.
3. **Vídeo de demonstração**:

   * Apresentação pessoal do aluno.
   * Explicação das funcionalidades.
   * Demonstração do funcionamento na placa.
   * Link (YouTube ou Google Drive).

## Hardware Utilizado

* **RP2040** (BitDogLab)
* LEDs RGB
* **Matriz WS2812B** (5×5)
* **OLED SSD1306**
* **Buzzer** (PWM)
* **Botões** A

## Software e Ferramentas

* **FreeRTOS** para multitarefa.
* **SDK da Raspberry Pi Pico**.
* **PIO** para controle de WS2812B.
* **Fontes** e biblioteca SSD1306.
* **CMake** e **Ninja** para build.

## Como Executar o Projeto

1. **Clone** o repositório:

   ```bash
   git clone <seu_repositorio_url>
   cd <nome_do_repositorio>
   ```
2. **Importe** no VS Code (extensão Raspberry Pi Pico).
3. **Ajuste** o caminho do FreeRTOS em `CMakeLists.txt`:

   ```cmake
   set(FREERTOS_PATH "caminho/para/freertos/source")
   ```
4. **Compile**:

   ```bash
   mkdir build && cd build
   cmake -G Ninja ..
   ninja
   ```
5. **Grave** na placa e **execute**.

## Vídeo de Demonstração

* \[Link para o vídeo no YouTube ou Google Drive]

## Licença

Este projeto é **código aberto** e pode ser usado e modificado livremente para fins educacionais e não comerciais.
