# can-collector

<!-- ### Resumo do projeto -->

Esse projeto tem por objetivo a coleta e o armazenamento de dados veículares através de sistemas embarcados. O projeto utilza uma plataforma IoT para o gerêciamento de dispositivos, conhecida como ThingsBoard, que permite o monitoramento, controle e armazenamento de informações. Para realizar o registro de dados cada dispositivo recebe um token da plataforma, no qual vai permitir a comunicação e o armazenamento. A comunicação é realizada através dos protocolos HTTP, HTTPS, MQTT, dentre outros. 

<!--
1. Como funciona?
2. Qual o propósito do projeto?

3. Quais versões do Freematics o código foi testado?
4. Como que o distpositivo está enviando dado pro ThingBoard?(HTTP, WebSocket, ...)
5. Qual conectividade o Freemtaics está uando?(WiFi, 5G, Bluetooth...)
6. Qual o nível de segurança do projeto?(Usa SSL/TSL, criptografa os dados enviados, assina os dados enviados)
7. Se não tem mecanismo de segurança é possível abilitar?
8. Qual arquitetura está sendo utilizada?(client/servidor, pub/sub)
-->

#### Dispositivos
- <a href="./freematics/">Freematics OBD</a>

###  ThingsBoard
O <a href="https://thingsboard.io/">ThingsBoard</a> é uma plataforma IoT para o armazenamento e gerenciamento de dados e dispositivos. Os dispositivos são cadastrados e credenciados nessa plataforma recebendo um token no qual vai permitir o armzenamento e o monitoramento. A comunicação com a plataforma pode ser realizada com protocólos HTTP, HTTPS, MQTT e etc. O ThingsBoard foi escolhido por ser simples, gratuito e funcional, possuindo também um longo periodo de existência no mercado.

<!-- resumo do thingsboard
como funciona no projeto em detalhes? -->

#### informações

informações sobre as versões de biblioteca e imagem Docker

Versão da biblioteca utiliza nos dispositivos IoT:
- Thingsboard@0.15.0

versão da imagem do Docker
- thingsboard/tb-node:4.0.1.1


#### container
<!-- Por que a pessoa deve rodar esse container? -->
<!-- O uso de um container permite que o administrador tenha a sua própria plataforma localmente sem  -->
<!-- Em qual situação devo usar o container? -->
<!-- Por que usar específicamente o Thingsboard e não outra solução? -->

Esse é um guia para a utilização do Thingsboard localmente. Caso o usuário precise realizar testes individuais ou esteja em um local com uma conxão instável com o servidor, uma alternativa é levantar um container Docker no qual pode criar e utilzar as suas próprias regras no Thingsboard, e utilizar todos os seus recursos livremente sem limitações ou interrupções.

O guia a seguir explica os passos para iniciar um container Docker para o Thingsboard:

```sh
# Cria o esquema de banco de dados para o Thingsboard
docker compose run --rm -e INSTALL_TB=true -e LOAD_DEMO=true thingsboard-ce

#Inciando o Thingsboard com desmonstração de logs
docker compose up -d && docker compose logs -f thingsboard-ce
```
