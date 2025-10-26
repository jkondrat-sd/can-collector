# Freematics OBD
<p>Esse é um código voltado para a coleta e o envio de telemetria para a plataforma do <a href="google.com">thingsboard</a> no servidor do Inmetro.</p>

<a href="./docs/DOCS.md">docs</a> documentação mais detalhada sobre algumas funcionalidades.

#### Guia simples:
**_platformio.ini_**
Arquivo de configuração do OBD, contendo todas as informações para o compilador.
```ini
; guia: https://docs.platformio.org/en/latest/projectconf/index.html
[env:esp32dev]
platform = espressif32 ; Utiliza bibliotecas e função do ESP32
board = esp-wrover-kit ; Modelo da placa utilizada 
board_build.f_cpu = 160000000L ;
framework = arduino ; Código com base na plataforma Arduino
monitor_speed = 115200 ; Define leitura padrão do monitor
board_build.flash_mode = qio ;
board_build.partitions = huge_app.csv ;
monitor_filters = default, log2file, colorize ; Gera logs a partir do monitor e armazena
; lib_extra_dirs = ../libraries ; Bibliotecas extras instaladas localmente
lib_deps = thingsboard/ThingsBoard@0.15.0 ; Bibliotecas utilizadas no código
```

**_python requiriments_**
Guia de instalação do Platformio e outras dependências em um ambiente virtual em python
```sh
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -r requeriments.txt
# ou 
python3 -m pip install ajsonrpc anyio bitarray bitstring bottle certifi cffi charset-normalizer click colorama cryptography esptool h11 idna intelhex markdown-it-py marshmallow mdurl packaging pip platformio pycparser pyelftools Pygments pyserial PyYAML reedsolo requests rich rich-click semantic-version sniffio starlette tabulate typing_extensions urllib3 uvicorn

#linux
sorce .venv/bin/active
#windows
.\.venv\Scripts\Activate.ps1
```

**_Configurações iniciais_:**
Configuração de credenciais para WiFi, e acessar o Thingsboard
```c
// WiFi credentials
constexpr char WIFI_SSID[] = "WebTeste";
constexpr char WIFI_PASSWORD[] = "123123123";

// ThingsBoard config
constexpr char TOKEN[] = "tokentoken"; // userName
constexpr char THINGSBOARD_SERVER[] = "site.com"; // server link

// Configurações da comunicação:
#define USING_HTTPS true // Habilita a comunicação HTTP, mas para usar o HTTPS precisa ativar o ENCRYPTED e fornecer o certificado
#define ENCRYPTED false // Habilita a comunicação cripotografada TSL/SSL com x509. Serve para comunicação HTTP e MQTT

// Definem o tamanho limite de uma mensagem. Em alguns cenários quando é necessário receber uma grande quantidade de informações é necessário modificar para um limite superior. Aplicavel ao MQTT
constexpr uint16_t MAX_MESSAGE_SEND_SIZE = 128U; // limite de envio
constexpr uint16_t MAX_MESSAGE_RECEIVE_SIZE = 128U; // limite de recebimento

// Caso utilize uma comunicação criptografada, insira o certificado x509
constexpr char ROOT_CERT[] = R"(-----BEGIN CERTIFICATE-----
|---x509---|
-----END CERTIFICATE-----
)";
```

**_Compilando o código_**
Guia de comandos do Platformio para compilar, limpar, monitorar e instalar novas bibliotecas se necessário. 
```sh
pio run # compila o código
pio run -t upload # embarca o código
pio run -t clean # limpa dados compilados. Util em caso de erro persistente em código já compilado
pio device monitor # ler dados do dispositivo via serial
pio pkg install ExampleLib@0.0.1 # baixa uma biblioteca com versão específica
pio pkg uninstall ExampleLib@0.0.1 # remove uma biblioteca em específico
pio pkg list # lista bibliotecas baixadas localmente
```

**_Biblitecas_**
Bibliotecas utilizadas no código e suas respectivas versões
```c
#include <Arduino.h> // Framework Arduino

/* Bibliotecas do Freematics; firmware V5 */
#include <FreematicsPlus.h>    // Biblioteca para o Freematics ONE modelo Plus
#include <FreematicsBase.h> // Classes para determinar alguns modelos de dados prontos, como o GPS
#include "esp_wifi.h" // corrige erro de dependencias do Freematics em questão de algumas bibliotecas. Essa é uma biblioteca nativa do esp32

/* Thingsboard@0.15.0 */
// Bibliotecas para o uso do Thingsboard
/* Caso utilize comunicação HTTP */
#include <Arduino_HTTP_Client.h>
#include <ThingsBoardHttp.h>
/* Caso utilize comunicação MQTT */
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>
```