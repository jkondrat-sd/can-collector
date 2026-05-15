## Raspberry Pi com Freematics OBD-II Emulator MK2

## Propósito do Projeto    
<!-- Realizar a leitura da rede CAN do emuladador OBD2 MK2 utilizando o Raspberry pi -->
O projeto tem por objtivo a leitura da rede <a href="https://en.wikipedia.org/wiki/CAN_bus">CANbus</a> do emulador MK2(<a href="https://freematics.com/pages/products/freematics-obd-emulator-mk2/">Freematics OBD-II Emulator MK2</a>) utilizando o <a href="">Raspberry Pi</a>.

## Como Funciona?
---
O Raspberry Pi realiza a leitura da rede CANbus através do módulo PiCAN2, reponsável por realizar a comunicação com o emulador MK2, ele torna possível o envio e o recebimento de informações na rede. A comunição é relizada com o envio de uma mensagem em (<a href="https://pt.wikipedia.org/wiki/Sistema_de_numera%C3%A7%C3%A3o_hexadecimal">hexadecimal</a>), contendo um ID de identificação da rede CAN(CAN ID), o tamnho da mensagem, o <a href="https://en.wikipedia.org/wiki/OBD-II_PIDs#Services_/_Modes">serviço/modo</a> e o pid requisitado pertencente a esse mesmo serviço/modo. O emulador MK2 retorna uma resposta contendo o valor atual do pid requisitado.

## Configuração da rede CANbus no Raspberry Pi
---
```sh
#Requisitos para o sistema operacional
sudo apt-get update
sudo apt-get upgrade
sudo apt install can-utils
```
```sh
#requisitos python3
pip install can python-can python-dotenv
pip install tb-mqtt-client # thigsboard with suport for mqtt and http
```
```sh
# Abra o arquivo de configuração da rede CANbus:
sudo nano boot/config.txt
```
```sh
# Adicione essas três linhas ao arquivo "config.txt", salve as modificações e resete o dispositivo
dtparam=spi=on
dtoverlay=mcp2515-can0-overlay,oscillator=16000000,interrupt=25
dtoverlay=spi-bcm2835-overlay
```
```sh
# Subindo a interface:
sudo /sbin/ip link set can0 up type can bitrate 500000
```
```sh
# Teste a insterface com o comando abaixo:
cansend can0 7DF#02010C0000000000
```
```sh
# Crie um arquivo na raiz do código para adiocionar as credencias do servidor
nano .env
```
```sh
# Configurações de credencias para enviar telemetria ao servidor
URL_SERVER ="https://mobilidade.inmetro.gov.br"
TOKEN_DEVICE="token" # O token deve ser obtido pela plataforma
```

## Detalhes da comunicação
---

### ISO 15765-4 CAN (11 bit ID,500 Kbaud)

Formato da mensagem da CAN OBDII, segundo a ISO 15765-4 com a configuração de velocidade 500 Kbaud e tamhano da mensagem de 11 bit ID.

> \<CAN ID>#\<tamanho da mensagem>.<serviço/modo>.\<PID>.00.00.00.00.00

***Exemplo:***

Abra um terminal e execute:
Esse comando permite ver as respostas da rede CAN mesmo que utilize o código python ou digite os comandos diretemente pelo terminal.
```sh
candump can0
```

Abra um segundo terminal e envie uma requisição para RPM(0x0C):
```sh
cansend 7DF#02.01.0C.00.00.00.00.00
```

Resposta do emulador MK2:
```sh
7E8# 04 41 0C 07 D0 00 00 00
```

O valor do pid solicitado se encontra após hex 0x0C, no caso o valor de RPM. Sendo esse valor "07 D0 00 00 00". Converta a respota de hexadecimal para decimal e depois aplique os valores à <a href="https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_-_Show_current_data">fómula de RPM</a>:
    
    A=07 B=D0 C=00 D=00 E=00

    fórmula de RPM:
        (256 * A)+B/4

    hex para dec:
        0x07 = 7
        0xD0 = 208
    
    aplicando a fórmula:
        A = 7
        B  = 208

        (256*7) + 208/4 = 500
        
        500 RPM

#### Fontes
----
https://www.csselectronics.com/pages/obd2-explained-simple-intro</br>
https://pt.wikipedia.org/wiki/OBD</br>
https://en.wikipedia.org/w/index.php?title=OBD-II_PIDs</br>
https://en.wikipedia.org/wiki/On-board_diagnostics</br>
https://www.csselectronics.com/pagescan-bus-simple-intro-tutorial#decode-can-data</br>
https://kvaser.com/about-can/higher-layer-protocols/j1939-introduction/</br>
https://en.wikipedia.org/wiki/On-board_diagnostics#SAE_standards_documents_on_OBD-II</br>
https://en.wikipedia.org/wiki/Unified_Diagnostic_Services</br>
https://www.obdtester.com/obd2_protocols</br>
https://www.sinocastel.com/5-common-obd2-protocols/</br>
https://simmasoftware.com/iso-15765-4-code-the-complete-guide-to-obd-II-over-can/</br>
https://www.obdexperts.com/obd2-protocols-explained-can-iso-j1850-and-more/</br>
https://openecu.com/openecu-calibrator-help-iso15765/</br>
https://obdplanet.com/obd2-protocol/</br>
https://www.obdexperts.com/obd2-protocols-explained-can-iso-j1850-and-more/</br>
