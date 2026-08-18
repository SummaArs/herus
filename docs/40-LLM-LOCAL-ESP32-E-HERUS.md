# LLM local em ESP32 e o HERUS

**Status:** análise técnica host-only; não é resultado de hardware do HERUS.
**Revisão:** llm-01
**Data:** 17 de agosto de 2026

## 1. Veredito curto

A demonstração compartilhada é tecnicamente legítima e interessante, mas não deve ser interpretada como “ChatGPT rodando em qualquer ESP32”. O repositório original `slvDev/esp32-ai` demonstra um modelo de 28,9 milhões de parâmetros, com 14.912.332 bytes no artefato final, em um ESP32-S3 da classe **N16R8** — 16 MB de flash e 8 MB de PSRAM — e reporta aproximadamente 9,5–9,88 tokens/s em seu próprio workload [1] [2].

O mesmo repositório declara que o modelo TinyStories escreve histórias simples, não responde perguntas gerais, não segue instruções, não escreve código e não conhece fatos [1]. O número de parâmetros é uma descrição de residência na hierarquia de memória, não uma medida de capacidade cognitiva.

A placa provisória do HERUS é a LilyGO T3-S3 V1.3, cuja página oficial informa 4 MB de flash e 2 MB de PSRAM [3]. Portanto, o artefato de 14,9 MB não cabe na flash onboard. Mesmo em uma placa N16R8, a demonstração ainda não prova fala em português, recuperação de memória, qualidade conversacional, coexistência com LoRa, energia, temperatura ou segurança do HERUS.

## 2. O que a técnica realmente faz

A técnica usa uma divisão por padrão de acesso. Um núcleo denso menor e as ativações ficam em memória rápida; a cabeça de saída é processada em PSRAM; e uma grande tabela de embeddings fica em flash, sendo consultada parcialmente por token. Quantização de 4 bits reduz o tamanho do artefato, e o runtime `llama2.c` evita a sobrecarga de frameworks grandes.

Essa estratégia é uma demonstração de **engenharia de hierarquia de memória**. Ela não transforma a flash em RAM rápida nem cria capacidade que não esteja no núcleo treinado. A fonte original publica ablações, verificação host, medições de bandwidth e medições on-chip para sua própria configuração [2]. Essas medições são evidência da fonte, não do HERUS.

A documentação oficial da Espressif confirma que o ESP32-S3 tem dois núcleos Xtensa LX7, até 240 MHz, 512 KB de SRAM interna, suporte a PSRAM/flash externa e instruções vetoriais úteis para redes neurais [4]. A documentação do ESP-IDF também registra que o espaço de endereços de PSRAM compartilha regiões com flash, que há restrições de cache e DMA e que stacks normalmente ficam em memória interna [5]. Assim, “2 MB ou 8 MB de PSRAM” não deve ser lido como memória livre para modelo, rádio, segurança, pilhas, buffers e armazenamento simultaneamente.

## 3. Comparação de hardware

| Configuração | Flash declarada | PSRAM declarada | SRAM interna | Conclusão para o artefato de 14.912.332 bytes |
|---|---:|---:|---:|---|
| ESP32-S3 N16R8 da demonstração | 16 MB | 8 MB | 512 KB | Cabe no armazenamento declarado; runtime medido somente pela fonte original |
| LilyGO T3-S3 V1.3 do HERUS | 4 MB | 2 MB | 512 KB | **Não cabe na flash onboard**; precisa outro artefato, TF/flash externo ou placa diferente |
| RP2350/Pico 2 | 4 MB no Pico 2 | 0 MB PSRAM no modelo citado | 520 KB | Não é alvo natural para essa técnica; é mais interessante para controle determinístico e segurança |

O verificador host `make -C firmware llm-budget-check` formaliza apenas esse sizing grosseiro. Ele não declara que um modelo cabe em runtime, não mede tokens por segundo e não mede energia, térmica, rádio, BLE, watchdog ou qualidade.

## 4. Workload correto para o HERUS

O HERUS não deve começar com um chat aberto no wearable. A primeira inteligência local útil é menor e mais controlável:

| Workload | Saída permitida | Candidato de execução |
|---|---|---|
| Classificar intenção curta | enum tipado + confiança + abstention | wearable ou Dock |
| Reconhecer comando de voz delimitado | comando fechado, sem transcrição persistida | ESP32-S3 com pipeline de voz dedicado |
| Detectar candidato de memória | tipo, relevância conservadora e motivo tipado | Dock ou telefone |
| Desambiguar uma consulta | match, ausência ou ambiguidade | Dock/telefone, nunca autoridade de envio |
| Recuperar cartão de memória | identificador local e proveniência mínima | Dock/telefone |
| Gerar explicação longa | texto para UX local | Dock ou telefone; opcional |

A própria Espressif oferece `esp-nn` para kernels otimizados de redes neurais e `ESP-DL` para inferência NN, e o `ESP-Skainet` suporta wake word e reconhecimento offline de comandos [6] [7] [8]. Esses caminhos são mais próximos da classificação e do reconhecimento de intenção do HERUS do que uma LLM TinyStories.

## 5. Divisão recomendada de responsabilidades

```text
HERUS One
  botão + mute + háptica + rádio/BLE + autoridade física
             │
             ├── telefone: interface rica, modelo maior e sincronização consentida
             │
             └── HERUS Dock: bateria + LoRa + storage + inferência local opcional
```

O One não precisa conter uma LLM para ser inteligente. Ele precisa impedir persistência e envio sem confirmação. O Dock é o melhor lugar para experimentar um modelo local porque pode receber mais memória, alimentação e dissipação, sem tornar o pulso pesado. O telefone pode fornecer uma proposta ou uma resposta longa, mas não pode contornar o gate físico.

A interface existente do HERUS já trata o modelo como adaptador não confiável: ele recebe uma entrada limitada, pode produzir apenas resposta de UX, não cria uma observação de intenção, não cria HCP e não transmite [9]. Essa fronteira deve permanecer mesmo que o modelo seja local e tecnicamente poderoso.

## 6. Critérios de aceitação

Uma implementação de modelo somente será candidata ao produto se demonstrar, no workload escolhido, orçamento de flash/PSRAM/SRAM após todos os periféricos, latência p95, energia por turno, estabilidade térmica, comportamento após reset, abstention em ambiguidade, rejeição de entradas adversariais e zero tentativas de autoridade. O artefato, tokenizer, configuração e digest precisam ser identificados; a licença precisa ser compatível; e o resultado deve ser reproduzível.

A medição física deve distinguir pelo menos três condições: inferência isolada, inferência com o sistema HERUS ativo e inferência durante rádio/armazenamento/segurança. Uma medição do modelo isolado não é uma medição do produto.

## 7. Experimento recomendado

A primeira experiência no hardware não deve tentar reproduzir 28,9 milhões de parâmetros na T3-S3 V1.3. Deve comparar, na mesma entrada sintética e sem dados pessoais, três adaptadores: uma gramática C11 determinística, um classificador NN compacto e uma LLM quantizada em um alvo com memória suficiente. Cada adaptador retorna a mesma saída tipada ao runtime do HERUS.

O host deve verificar golden outputs, tamanho dos assets, tokenizer, casos funcionais, casos ambíguos, casos adversariais e ausência de autoridade. Depois, a bancada deve medir RAM livre, PSRAM livre, latência, energia, temperatura, reset e coexistência com o rádio. Se a gramática alcançar a utilidade necessária com menos risco, ela vence a LLM; usar um modelo maior apenas por marketing seria uma regressão de engenharia.

## 8. Decisão

A matéria compartilhada muda a fronteira de possibilidade, mas não muda o papel correto da inteligência no HERUS. Ela sugere que um ESP32-S3 N16R8 pode hospedar uma demonstração gerativa pequena e local. Não justifica colocar um modelo TinyStories no wearable nem migrar a placa de bancada sem necessidade.

A decisão recomendada é:

1. manter a LilyGO T3-S3 V1.3 para bring-up de rádio, interação e contratos;
2. tratar o Dock como alvo de inferência local, caso um modelo seja necessário;
3. usar no wearable uma gramática, wake word ou classificador compacto antes de uma LLM;
4. usar o telefone como interface/modelo maior durante a prototipagem, sem dar autoridade a ele;
5. considerar uma placa N16R8 separada apenas para reproduzir a demonstração, não como prova do produto HERUS.

O ganho real para o HERUS não é dizer “tem 28,9 milhões de parâmetros”. É conseguir operar com inteligência local suficiente para **abster-se, classificar, recuperar e pedir confirmação**, mesmo quando a rede falha, sem transformar o modelo em autoridade.

## Referências

[1] [slvDev/esp32-ai — README](https://github.com/slvDev/esp32-ai).

[2] [slvDev/esp32-ai — RESULTS.md](https://github.com/slvDev/esp32-ai/blob/main/RESULTS.md).

[3] [LILYGO — T3 S3 V1.3](https://lilygo.cc/en-us/products/t3-s3-v1-3).

[4] [Espressif — ESP32-S3](https://www.espressif.com/en/products/socs/esp32-s3).

[5] [ESP-IDF — Support for External RAM, ESP32-S3](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/external-ram.html).

[6] [Espressif — esp-nn](https://github.com/espressif/esp-nn).

[7] [Espressif — ESP-DL User Guide](https://docs.espressif.com/projects/esp-dl/en/release-v1.1/esp32s3/introduction.html).

[8] [Espressif — ESP-Skainet](https://github.com/espressif/esp-skainet).

[9] [HERUS — Dialogue contract](../firmware/core/dialogue.h).
