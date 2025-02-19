# Controle Interativo com Display SSD1306 e Joystick

## Descrição
Este projeto utiliza o Raspberry Pi Pico para ler os valores de um joystick analógico e exibir um quadrado de 8x8 pixels no display OLED SSD1306. O quadrado inicia centralizado e se move proporcionalmente aos valores capturados pelo joystick, proporcionando uma interface gráfica interativa.

## Materiais Utilizados
- **Raspberry Pi Pico** - Placa microcontroladora
- **Display OLED SSD1306** - Tela OLED para exibição gráfica via I2C
- **Joystick Analógico** - Dispositivo de entrada analógica
- **LED RGB** - Indicação visual (opcional)
- **2 Botões de Pressão** - Para interações adicionais
- **Resistores** - Conforme necessário para adequação de circuitos

## Funcionalidades
- Leitura dos valores do joystick via ADC (Analog-to-Digital Converter)
- Exibição de um quadrado no display SSD1306 baseado na entrada do joystick
- Controle de LEDs RGB via PWM com intensidade baseada no movimento do joystick
- Uso de botões para interações adicionais, como alteração do estilo da borda
- Implementação de interrupções para os botões

## Estrutura do Código
O código é composto pelas seguintes seções:
- **Definição de pinos**: Mapeia os pinos do Raspberry Pi Pico para os componentes
- **Função `gpio_callback()`**: Trata as interrupções dos botões
- **Função `setup_pwm()`**: Configura o PWM para controle dos LEDs
- **Função `setup()`**: Inicializa os periféricos, ADC, I2C e GPIOs
- **Função `loop()`**: Executa a lógica principal, atualizando os LEDs e a interface gráfica
- **Função `main()`**: Chama `setup()` e entra em um loop infinito de execução

## Vídeo Demostrativo

[Vídeo de demonstração no YouTube](https://www.youtube.com/watch?v=p_FdOkpqZj4)

## Autor
Desenvolvido por **Leonardo Rodrigues**

