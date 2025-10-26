# can-collector

Guia para levantar o Docker do Thingsboard:
```sh
# Cria o esquema de banco de dados para o Thingsboard
docker compose run --rm -e INSTALL_TB=true -e LOAD_DEMO=true thingsboard-ce

#Inciando o Thingsboard com desmonstração de logs
docker compose up -d && docker compose logs -f thingsboard-ce
```