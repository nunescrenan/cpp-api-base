# VendBunny Backend

Backend em C++ usando [Drogon](https://github.com/drogonframework/drogon), com build via CMake e dependencias gerenciadas por `vcpkg`.

## Requisitos

- CMake 3.20+
- compilador com suporte a C++20
- `vcpkg`
- Ninja ou outro generator configurado no ambiente

## Rodando o projeto

```bash
cmake -S . -B build
cmake --build build
./build/vendbunny_backend
```

Para modo watch:

```bash
./start.sh
```

`start.sh` usa `watchexec`, entao ele precisa estar instalado no sistema.

## Configuracao

O backend tenta ler `config.json` na raiz do projeto e depois sobrescreve com variaveis de ambiente quando presentes.

Campos suportados:

- `host`
- `port`
- `threads`
- `logLevel`
- `corsEnabled`

Variaveis de ambiente suportadas:

- `HOST`
- `PORT`
- `THREADS`
- `LOG_LEVEL`

Exemplo de `config.json`:

```json
{
  "host": "127.0.0.1",
  "port": 9001,
  "threads": 1,
  "logLevel": "info",
  "corsEnabled": true
}
```

## Endpoints

- `GET /ping`

Resposta esperada:

```json
{
  "status": "200 OK",
  "message": "pong"
}
```

## Estrutura

```text
src/
  config/
  controllers/
    ping/
  middlewares/
```

## Notas

- CORS esta habilitado quando `corsEnabled` for `true`.
- O path de upload do Drogon foi apontado para o diretorio temporario do sistema, entao o projeto nao cria mais a pasta local `uploads/`.
