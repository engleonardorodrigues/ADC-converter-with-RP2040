/***************************************************************************************************** 
 * Título        : Controle Interativo com Display SSD1306 e Joystick  
 * Desenvolvedor : Leonardo Rodrigues                                  
 * Versão        : 1.0.0                                              
 *                                                                                
 * Descrição:                                                                   
 * Este projeto utiliza o Raspberry Pi Pico para ler os valores de um joystick e    
 * exibir um quadrado de 8x8 pixels no display SSD1306. O quadrado inicia           
 * centralizado e se move proporcionalmente aos valores capturados pelo joystick,  
 * proporcionando uma interface gráfica interativa.                               
 *                                                                                 
 * Materiais utilizados:                                                           
 *  1 - Raspberry Pi Pico                  // Placa microcontroladora  
 *  1 - Display OLED SSD1306               // Tela OLED para exibição gráfica  
 *  1 - Joystick analógico                 // Dispositivo de entrada analógica  
 *  1 - LED RGB                           // LED RGB para indicações visuais (opcional)  
 *  2 - Botões de pressão                  // Botões para interações adicionais  
 *  1 - Resistores (conforme necessário)   // Resistores para adequação de circuitos  
 ******************************************************************************************************/

 #include <stdio.h>                                                                                 // Inclui a biblioteca padrão de entrada/saída
 #include "pico/stdlib.h"                                                                           // Inclui as definições padrão da SDK do Raspberry Pi Pico
 #include "hardware/adc.h"                                                                          // Inclui a biblioteca para controle do ADC
 #include "hardware/pwm.h"                                                                          // Inclui a biblioteca para controle do PWM
 #include "hardware/i2c.h"                                                                          // Inclui a biblioteca para controle do I2C
 #include "lib/ssd1306.h"                                                                           // Inclui a biblioteca para controle do display SSD1306
 #include "lib/font.h"                                                                              // Inclui a fonte para desenhar caracteres no display
 
 #define LED_B_PIN 11                                                                               // Define o pino do LED azul como 11
 #define LED_R_PIN 12                                                                               // Define o pino do LED vermelho como 12
 #define LED_G_PIN 13                                                                               // Define o pino do LED verde como 13
 #define JOY_X_PIN 26                                                                               // Define o pino ADC para o eixo X do joystick como 26
 #define JOY_Y_PIN 27                                                                               // Define o pino ADC para o eixo Y do joystick como 27
 #define JOY_BTN_PIN 22                                                                             // Define o pino do botão do joystick como 22
 #define BTN_A_PIN 5                                                                                // Define o pino do botão A como 5
 #define I2C_SDA 14                                                                                 // Define o pino SDA do I2C como 14
 #define I2C_SCL 15                                                                                 // Define o pino SCL do I2C como 15
 
 #define ADC_MAX 4095                                                                               // Define o valor máximo do ADC (12 bits)
 #define ADC_MID 2048                                                                               // Define o valor médio do ADC para centralização
 #define PWM_MAX 255                                                                                // Define o valor máximo do PWM (8 bits)
 
 bool led_enabled = true;                                                                           // Variável que indica se os LEDs estão habilitados
 bool led_green_state = false;                                                                      // Estado atual do LED verde (ligado/desligado)
 uint8_t border_style = 0;                                                                          // Variável para o estilo de borda (para uso futuro)
 
 void gpio_callback(uint gpio, uint32_t events) {                                                   // Função de callback para interrupções de GPIO
     static absolute_time_t last_time = {0};                                                        // Armazena o tempo da última interrupção (para debouncing)
     absolute_time_t now = get_absolute_time();                                                     // Obtém o tempo atual
     if (absolute_time_diff_us(last_time, now) < 200000) return;                                    // Se a diferença for menor que 200ms, ignora a interrupção
     last_time = now;                                                                               // Atualiza o tempo da última interrupção
     if (gpio == JOY_BTN_PIN) {                                                                     // Se a interrupção veio do botão do joystick
         led_green_state = !led_green_state;                                                        // Inverte o estado do LED verde
         gpio_put(LED_G_PIN, led_green_state);                                                      // Atualiza o LED verde com o novo estado
         border_style = (border_style + 1) % 3;                                                     // Alterna o estilo de borda (ciclando entre 0, 1 e 2)
     } else if (gpio == BTN_A_PIN) {                                                                // Se a interrupção veio do botão A
         led_enabled = !led_enabled;                                                                // Inverte o estado de habilitação dos LEDs
         pwm_set_gpio_level(LED_R_PIN, 0);                                                          // Desliga o LED vermelho via PWM
         pwm_set_gpio_level(LED_B_PIN, 0);                                                          // Desliga o LED azul via PWM
         pwm_set_gpio_level(LED_G_PIN, 0);                                                          // Desliga o LED verde via PWM
     }
 }                                                                                                  // Fim da função gpio_callback
 
 void setup_pwm(uint pin) {                                                                         // Função para configurar o PWM em um pino específico
     gpio_set_function(pin, GPIO_FUNC_PWM);                                                         // Define a função do pino como PWM
     uint slice = pwm_gpio_to_slice_num(pin);                                                       // Obtém o slice do PWM associado ao pino
     pwm_set_wrap(slice, PWM_MAX);                                                                  // Configura o valor máximo (wrap) do PWM
     pwm_set_enabled(slice, true);                                                                  // Habilita o PWM para o slice
 }                                                                                                  // Fim da função setup_pwm
 
 void setup() {                                                                                     // Função para inicializar configurações do sistema
     
     stdio_init_all();                                                                              // Inicializa a comunicação padrão (USB serial)
     
     adc_init();                                                                                    // Inicializa o ADC
     
     adc_gpio_init(JOY_X_PIN);                                                                      // Configura o pino do eixo X do joystick para entrada ADC
     adc_gpio_init(JOY_Y_PIN);                                                                      // Configura o pino do eixo Y do joystick para entrada ADC
     
     gpio_init(LED_G_PIN);                                                                          // Inicializa o pino do LED verde
     gpio_set_dir(LED_G_PIN, GPIO_OUT);                                                             // Define o pino do LED verde como saída
     
     gpio_init(JOY_BTN_PIN);                                                                        // Inicializa o pino do botão do joystick
     gpio_set_dir(JOY_BTN_PIN, GPIO_IN);                                                            // Define o pino do botão do joystick como entrada
     gpio_pull_up(JOY_BTN_PIN);                                                                     // Ativa o resistor de pull-up no botão do joystick
     
     gpio_set_irq_enabled_with_callback(JOY_BTN_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);     // Configura interrupção de borda de descida no botão do joystick com callback
     
     gpio_init(BTN_A_PIN);                                                                          // Inicializa o pino do botão A
     gpio_set_dir(BTN_A_PIN, GPIO_IN);                                                              // Define o pino do botão A como entrada
     gpio_pull_up(BTN_A_PIN);                                                                       // Ativa o resistor de pull-up no botão A
     
     gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);       // Configura interrupção de borda de descida no botão A com callback
     
     setup_pwm(LED_R_PIN);                                                                          // Configura o PWM para o LED vermelho
     setup_pwm(LED_B_PIN);                                                                          // Configura o PWM para o LED azul
     
     i2c_init(i2c0, 400 * 1000);                                                                    // Inicializa o I2C0 com velocidade de 400 kHz
     
     gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                                                     // Configura o pino SDA para função I2C
     gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                                                     // Configura o pino SCL para função I2C
     
     gpio_pull_up(I2C_SDA);                                                                         // Ativa o resistor de pull-up no pino SDA
     gpio_pull_up(I2C_SCL);                                                                         // Ativa o resistor de pull-up no pino SCL
     //ssd1306_init();                                                                              // Linha comentada: inicialização do display SSD1306 (pode ser ativada quando necessário)
 }                                                                                                  // Fim da função setup
 
 void loop() {                                                                                      // Função principal do loop de execução
     adc_select_input(0);                                                                           // Seleciona o canal 0 do ADC para o eixo X do joystick
     uint16_t joy_x = adc_read();                                                                   // Lê o valor do ADC para o eixo X do joystick
     adc_select_input(1);                                                                           // Seleciona o canal 1 do ADC para o eixo Y do joystick
     uint16_t joy_y = adc_read();                                                                   // Lê o valor do ADC para o eixo Y do joystick
     if (led_enabled) {                                                                             // Se os LEDs estiverem habilitados
         uint16_t pwm_r = (abs((int)joy_x - ADC_MID) * PWM_MAX) / ADC_MID;                          // Calcula o PWM para o LED vermelho com base na diferença do eixo X
         uint16_t pwm_b = (abs((int)joy_y - ADC_MID) * PWM_MAX) / ADC_MID;                          // Calcula o PWM para o LED azul com base na diferença do eixo Y
         pwm_set_gpio_level(LED_R_PIN, pwm_r);                                                      // Define o nível PWM do LED vermelho com o valor calculado
         pwm_set_gpio_level(LED_B_PIN, pwm_b);                                                      // Define o nível PWM do LED azul com o valor calculado
     }
     int x_pos = (joy_x * 120) / ADC_MAX;                                                           // Mapeia o valor do eixo X para uma posição horizontal no display (0 a 120)
     int y_pos = (joy_y * 56) / ADC_MAX;                                                            // Mapeia o valor do eixo Y para uma posição vertical no display (0 a 56)
     //ssd1306_clear();                                                                             // Linha comentada: limpa o buffer do display SSD1306
     //ssd1306_draw_rect(x_pos, y_pos, 8, 8);                                                       // Linha comentada: desenha um quadrado de 8x8 pixels na posição mapeada
     //ssd1306_update();                                                                            // Linha comentada: envia os dados do buffer para o display SSD1306
     sleep_ms(50);                                                                                  // Aguarda 50 milissegundos antes da próxima iteração do loop
 }                                                                                                  // Fim da função loop
 
 int main() {                                                                                       // Função principal do programa
     setup();                                                                                       // Executa a função setup para inicializações
     while (1) {                                                                                    // Inicia um loop infinito
         loop();                                                                                    // Chama a função loop em cada iteração
     }
 }                                                                                                  // Fim da função main
 