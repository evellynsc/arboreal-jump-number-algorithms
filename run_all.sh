#!/bin/bash

# Caminho para o diretório de configuração
CONFIG_DIR=config

# Caminho para o executável do programa
PROGRAM_PATH=./build/ajns

# Verifica se o diretório de configuração existe
if [ ! -d "$CONFIG_DIR" ]; then
    echo "[ERRO] O diretório de configuração \"$CONFIG_DIR\" não existe."
    exit 1
fi

# Itera sobre cada arquivo JSON no diretório de configuração
for f in "$CONFIG_DIR"/*.json; do
    echo "[INFO] Executando o programa para o arquivo de configuração $f"
    $PROGRAM_PATH 1 "$f"
    if [ $? -ne 0 ]; then
        echo "[ERRO] Ocorreu um erro ao processar o arquivo $f"
    else
        echo "[INFO] Arquivo $f processado com sucesso"
    fi
done

echo "[INFO] Todos os arquivos de configuração foram processados."