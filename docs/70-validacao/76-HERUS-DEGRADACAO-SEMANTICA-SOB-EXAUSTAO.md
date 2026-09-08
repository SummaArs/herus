# HERUS — Degradação semântica sob exaustão e contexto hostil

**Estado:** host-side verificado; comportamento físico ainda pendente.  
**Objetivo:** provar que memória cheia, conflito, ambiguidade, consentimento revogado e ausência do Core degradam para estados explícitos, nunca para sucesso inventado.

## Resultado executável

A nova etapa foi implementada sobre `memory_semantic_evidence`, `memory_reasoning_bridge` e `magic_anticipation`, sem criar uma camada paralela. A campanha usa o mesmo C11 estrito do firmware e um redteam que remove os gates diretamente dos arquivos de produção.

| Controle | Resultado |
|---|---:|
| Matriz de degradação semântica | **6/6** |
| Redteam de capacidade e contexto | **5/5 mutantes mortos** |
| Regressão global | **72 suítes** |
| Invariantes da simulação | **111/111** |
| Proveniência | **válida; 1 entrada ativa, 3 pendentes, sem atestação externa** |

## Semântica observada

Quando o índice semântico atinge `MSE_MAX_EVIDENCE`, a próxima inserção retorna `MSE_E_FULL` e o contador permanece inalterado. O teste preenche os 16 slots, tenta inserir uma nova evidência e compara a capacidade antes e depois. Assim, exaustão não pode sobrescrever uma evidência antiga nem fazer uma nova evidência parecer persistida.

Uma consulta contextual ativa sobre memória cheia, mas sem correspondência, produz `MAGIC_KNOWN_GAP` com explicação disponível, não uma lembrança fabricada. A mesma consulta com `proactive_consent = 0` produz `MAGIC_SILENT`, mesmo quando a janela de atenção está ativa. A proposta não ganha autoridade pelo fato de o contexto parecer relevante.

Duas evidências funcionais incompatíveis produzem `MAGIC_CONTRADICTION` e `requires_confirmation = 1`. O sistema não escolhe o fato mais novo como verdade. Duas alternativas compatíveis produzem `MAGIC_ABSTAIN`; a resposta permanece sem candidato selecionado. Essa diferença é importante: contradição exige resolução humana explícita, enquanto ambiguidade apenas interrompe a seleção automática.

A ausência do Core é representada no teste por ausência de evidência local. O resultado é `MAGIC_KNOWN_GAP`, sem confirmação implícita e sem fallback de consulta remota. Essa modelagem mantém o Core como alimentador autorizado de conhecimento, não como fonte de autoridade ou executor.

## Mutantes adversariais

A frente sabotadora removeu o gate de capacidade, o gate de consentimento contextual, a conversão de conflito para confirmação, a abstention de ambiguidade e a marcação de fatos funcionais conflitantes. Cada mutante fez a matriz falhar e foi morto.

| Mutante | Controle removido | Falha que o oráculo detecta |
|---|---|---|
| `semantic-capacity-gate` | Rejeição de inserção além de 16 evidências | Sobrescrita ou alteração de `evidence_count` |
| `magic-consent-gate` | Silêncio sem consentimento proativo | Proposta contextual com consentimento revogado |
| `magic-conflict-abstention-gate` | Confirmação obrigatória em conflito | Conflito tratado como recall |
| `magic-ambiguity-abstention-gate` | Abstention de alternativas | Escolha de um favorito sob ambiguidade |
| `semantic-conflict-marking-gate` | Marcação dos dois fatos como conflitantes | Contradição deixa de chegar à proposta |

## O que esta etapa não prova

Os resultados não medem qualidade de memória humana, utilidade subjetiva da “magia”, acurácia de uma LLM, energia, latência, persistência elétrica, desgaste de flash, comportamento de brownout, ruído de sensores ou qualidade de áudio. Também não provam que o Core real entregará uma resposta, nem que qualquer resposta externa será correta.

A afirmação sustentada é mais estreita: dentro dos contratos tipados atuais, a falta de capacidade e a incerteza semântica não podem ser convertidas em certeza silenciosa, sobrescrita, apresentação proativa sem consentimento ou autoridade de ação.

## Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/test_degradation_redteam.py
cd firmware
make degradation-matrix degradation-redteam
cd ..
./prove.sh --quiet
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
```

O resultado global `ALL INVARIANTS HOLD` significa contratos host-side verificados. Os gates físicos continuam bloqueados até que uma bancada reproduza os cenários com mídia, alimentação e interfaces reais.
