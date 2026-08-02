# Experimento EKOM — Registry de NVS

## Objetivo

Verificar se a EKOM consegue fornecer contexto suficiente para que um humano
ou agente compreenda e evolua o Registry de NVS com menos investigação, maior
qualidade e maior confiança.

## Pergunta central

Qual é o menor conjunto de contexto necessário para alterar o Registry de NVS
de forma correta e segura?

## Escopo

- bootstrap da aplicação;
- criação e inicialização do Registry;
- registro das definições NVS;
- consumo das definições pelas Capabilities;
- validações e falhas de inicialização;
- critérios de aceite da especificação.

## Fora do escopo

- documentar toda a IoTSmartSysCore;
- copiar detalhes encontrados diretamente no código;
- revisar todas as ADRs existentes;
- definir a estrutura definitiva da EKOM.

## Resultado esperado

Uma pessoa ou agente sem memória das conversas anteriores deve conseguir:

1. formar rapidamente um modelo mental do fluxo;
2. localizar a implementação relevante;
3. compreender as decisões e restrições;
4. propor uma alteração coerente;
5. definir testes e critérios de aceite verificáveis.

## Critério de sucesso

O material será considerado útil se reduzir a necessidade de varredura do
código sem ocultar detalhes importantes ou criar mais trabalho de manutenção
do que benefício.

O primeiro artefato de navegação do piloto é a
[árvore do conhecimento](KNOWLEDGE-MAP.md).
