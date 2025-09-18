#!/bin/bash

# Caminho para o diretório de configuração
INPUT_DIR=config/feasibility

# Caminho para o executável do programa
PROGRAM_PATH=./build/ajns

# Verifica se o diretório de configuração existe
if [ ! -d "$INPUT_DIR" ]; then
    echo "[ERRO] O diretório de configuração \"$INPUT_DIR\" não existe."
    exit 1
fi

# Itera sobre cada arquivo JSON no diretório de configuração
for f in "$INPUT_DIR"/*.json; do
    filename=$(basename "$f")
    output_file="${INPUT_DIR}/output/${filename}.output"
    echo "[INFO] Executando o programa para o arquivo de configuração $f"
    $PROGRAM_PATH 1 "$f" > "$output_file" 2>&1 &
    if [ $? -ne 0 ]; then
        echo "[ERRO] Ocorreu um erro ao processar o arquivo $f"
    else
        echo "[INFO] Arquivo $f processado com sucesso"
    fi
done

echo "[INFO] Todos os arquivos de configuração foram processados."

# avaliar os modelos
# csv
# gerar um arquivo da forma esperada
# tempo de execução
# número de jumps
# instância (nome,num de violadores)
# quantidade de nós que foram explorados
# quantidade de cortes (de cada tipo) do branch and cut (olhar como fazer isso)
# verificar se dá pra pegar o valor da relaxação na raiz
# limite primal
# limite dual
# 


# avaliar a relaxação linear
# valor da relaxação linear 

