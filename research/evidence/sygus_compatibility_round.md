# Evidência — Auditoria de compatibilidade SyGuS

## Fonte

Foi usado um clone raso do repositório público `SyGuS-Org/benchmarks`, que declara conter os benchmarks públicos da competição SyGuS. O próprio repositório alerta que podem existir erros e duplicatas; portanto, o HERUS não trata o conjunto como verdade automaticamente.

## Protocolo

O auditor `research/sygus_compatibility.py` não executa benchmarks, não executa texto sintetizado e não atribui sucesso. Ele classifica arquivos `.sl` por lógica declarada, número de funções sintetizadas, presença de restrições e marcadores fora do escopo atual.

O subconjunto operacional inicial exige uma função sintetizada, lógica `LIA`, ao menos uma restrição e ausência de `ite`, arrays, bit-vectors, strings, quantificadores ou recursão. Essa é uma triagem conservadora, não um parser SyGuS completo.

## Resultado

Na amostra determinística de 64 arquivos da família `comp/2017/CLIA_Track`, 45 foram classificados como candidatos compatíveis e 19 foram rejeitados: 15 por uso de `ite` e 4 por múltiplas funções sintetizadas. A classificação não mede taxa de solução e não autoriza comparar o HERUS com os solvers SyGuS.

## Limite científico

O MVP atual verifica e sintetiza máquinas de estado finitas; SyGuS cobre síntese de funções em teorias e gramáticas mais amplas. O adaptador serve para impedir comparações inválidas e preparar uma tradução declarada. Para alegar desempenho, ainda é necessário implementar o tradutor semântico, preservar a especificação, executar um baseline SyGuS confiável e avaliar um split público/oculto com métricas iguais.
