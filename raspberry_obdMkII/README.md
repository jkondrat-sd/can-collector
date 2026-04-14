#### Raspberry Pi com Freematics OBD-II Emulator MK2

#### Propósito do Projeto    
<!-- Realizar a leitura da rede CAN do emuladador OBD2 MK2 utilizando o Raspberry pi -->
O projeto tem por objtivo a leitura da rede <a href="">CANbus</a> do emulador MK2(<a href="https://freematics.com/pages/products/freematics-obd-emulator-mk2/">Freematics OBD-II Emulator MK2</a>) utilizando o <a href="">Raspberry Pi</a>. 
<!-- utilinzando o módulo <a href="https://copperhilltech.com/pican-2-can-bus-interface-for-raspberry-pi/">Pican2</a>. -->

#### Como Funciona?

O Raspberry Pi realiza a leitura da rede CANbus através do módulo Pican2, reponsável por realizar a comunicação com o emulador MK2, ele torna possível o envio e o recebimento de informações na rede. A comunição é relizada com o envio de uma mensagem em hex(<a href="https://pt.wikipedia.org/wiki/Sistema_de_numera%C3%A7%C3%A3o_hexadecimal">hexadecimal</a>), contendo um ID de identificação da rede CAN(CAN ID), o tamnho da mensagem, o <a href="https://en.wikipedia.org/wiki/OBD-II_PIDs#Services_/_Modes">serviço/modo</a> e o pid requisitado pertencente a esse mesmo serviço/modo. O emulador MK2 retorna uma resposta contendo o valor atual do pid requisitado.
<!-- requisições específicas, contendo informações sobre o pid requisitado, o Raspberry Pi envia uma requisição contendo o tamnho da mensagem, serviço de comunicação e o pid -->

#### Configuração da rede CANbus no Raspberry Pi

```sh
    # Requisitos para o sistema operacional
    sudo apt-get update
    sudo apt-get upgrade
    sudo apt install can-utils

    #requisitos python3
    can 0.0.0
    python-can 4.6.1

    # Abra o arquivo de configuração da rede CANbus:
    sudo nano boot/config.txt

    # Adicione essas três linhas ao arquivo, e depois salve as modificações e resete o dispositivo
    dtparam=spi=on
    dtoverlay=mcp2515-can0-overlay,oscillator=16000000,interrupt=25
    dtoverlay=spi-bcm2835-overlay

    # Subindo a interface:
    sudo /sbin/ip link set can0 up type can bitrate 500000

    # Teste a insterface com o comando abaixo:
    cansend can0 7DF#02010C0000000000

    # Após seguir todas as etapas execute o código main.py
    python3 main.py
```

#### Detalhes da comunicação

> ISO 15765-4 CAN (11 bit ID,500 Kbaud)

formato da mensagem da CAN OBDII, segundo a ISO 15765-4 com a configuração de velocidade 500 Kbaud e tamhano da mensagem de 11 bit ID.

> \<CAN ID>#\<tamanho da mensagem>.<serviço/modo>.\<PID>.00.00.00.00.00

exemplo:

envio de uma requisição para RPM(0x0C):
> cansend 7DF#02.01.0C.00.00.00.00.00

Abra um segundo terminal e execute:
```sh
candump can0
```
Esse comando permite ver as respostas da rede CAN mesmo que utilize o código python ou digite os comandos diretemente pelo terminal.

resposta do emulador MK2:
> 7E8# 04 41 0C 07 D0 00 00 00

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

#### fontes

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
