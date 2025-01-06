# Projeto de Monitoramento de Temperatura, Umidade e Controle de Relé com ESP8266

Este projeto utiliza o **ESP8266** para monitorar a temperatura e a umidade utilizando o sensor **DHT** e enviar esses dados para a plataforma **ThingSpeak**. Também é possível controlar um relé, que pode ser usado para ligar ou desligar dispositivos, e interagir com o **Sinric** para controle via WebSockets.

## Requisitos

- **ESP8266**: microcontrolador que executa o código.
- **Sensor DS18B20**: sensor de temperatura.
- **Relé**: para controlar dispositivos.
- **ThingSpeak**: para receber os dados do sensor e visualizar no gráfico.
- **Sinric**: plataforma para controlar dispositivos via WebSocket.

## Bibliotecas Utilizadas

- [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi): Biblioteca para conexão Wi-Fi com o ESP8266.
- [WebSocketsClient](https://github.com/Links2004/arduinoWebSockets): Biblioteca para comunicação via WebSocket com a plataforma Sinric.
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson): Para manipulação de dados JSON recebidos do Sinric.
- [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-and-Humidity-Sensor-Library): Biblioteca para interação com o sensor de temperatura DS18B20.
- [OneWire](https://github.com/PaulStoffregen/OneWire): Biblioteca para comunicação com o sensor DS18B20.

## Conexões de Hardware

- **Sensor de Temperatura DS18B20**: Conectado ao pino **D4 (GPIO 4)**.
- **Relé**: Conectado ao pino **D6 (GPIO 12)**.
- **LED**: Usado para indicar o status da conexão, conectado ao pino **LED_BUILTIN**.

## Configurações de Rede

O código utiliza múltiplos SSIDs para tentar conectar-se à rede Wi-Fi. Você deve definir suas credenciais de rede Wi-Fi nos seguintes campos:

```cpp
#define ssid1 "SeuSSID1"
#define password1 "SuaSenha1"
#define ssid2 "SeuSSID2"
#define password2 "SuaSenha2"
#define ssid3 "SeuSSID3"
#define password3 "SuaSenha3"
```

Além disso, insira sua API Key do ThingSpeak e ID do dispositivo para o controle via Sinric:

```cpp
String chave_escrita_thingspeak = "SuaChaveEscritaDoThingSpeak";
#define MyApiKey "SuaAPIKey"
#define DispositivoID "IDDoDispositivoSinric"
```
## Funcionalidades

Monitoramento de Temperatura e Umidade

O código lê a temperatura do sensor DS18B20 conectado ao pino GPIO 4 (D4). Os dados são enviados a cada 30 segundos para a plataforma ThingSpeak via requisição HTTP POST. O dado de temperatura será enviado para o campo 1 do seu canal ThingSpeak.

### Controle de Relé

O relé conectado ao pino GPIO 12 (D6) pode ser controlado via comandos recebidos pela plataforma Sinric. Através de um comando On/Off, o relé pode ser ativado ou desativado para controlar dispositivos conectados a ele.

### WebSocket com Sinric

A interação com a plataforma Sinric é feita via WebSockets. O ESP8266 escuta comandos de ligar/desligar dispositivos conectados ao relé. Quando um comando On ou Off é recebido, o relé é ativado ou desativado respectivamente.

### Funções:

- **envia_informacoes_thingspeak:** Envia os dados de temperatura para o ThingSpeak.
- **turnOn:** Liga o dispositivo controlado pelo relé.
- **turnOff:** Desliga o dispositivo controlado pelo relé.
- **webSocketEvent:** Processa os comandos recebidos via WebSocket do Sinric.

### Passo a Passo para Configuração
1. Configuração de Rede Wi-Fi:

Configure suas credenciais de Wi-Fi nas variáveis ssid1, ssid2, ssid3, e suas respectivas senhas.
   
2. Configuração do ThingSpeak:

  1. Crie uma conta no ThingSpeak.
  2. Crie um canal e pegue sua API Key para inseri-la na variável chave_escrita_thingspeak.
   
3. Configuração do Sinric:

  1. Crie uma conta no Sinric.
  2. Adicione um dispositivo virtual e pegue o Device ID para inseri-lo na variável DispositivoID.
     
4. Carregar o Código:

  1. Abra o código no seu editor de Arduino (ou PlatformIO).
  2. Selecione a placa ESP8266 e a porta correta.
  3. Carregue o código para o ESP8266.
     
5. Monitoramento no ThingSpeak:

Acesse seu canal no ThingSpeak e visualize os dados de temperatura sendo atualizados a cada 30 segundos.

6. Controle do Relé via Sinric:

Envie comandos para ligar ou desligar o relé a partir da interface WebSocket do Sinric.

### Exemplo de Saída

O monitor serial exibirá os dados de temperatura sendo coletados a partir do sensor DS18B20.
O relé será controlado via comandos recebidos do Sinric, e a temperatura será enviada para o ThingSpeak a cada 30 segundos.

## Licença

Este projeto está licenciado sob a MIT License.
