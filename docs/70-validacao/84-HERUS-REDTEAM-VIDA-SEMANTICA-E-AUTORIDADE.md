# HERUS — Redteam da vida semântica e da autoridade pessoal

**Estado:** host-side integrado e publicado; resultado sem alegações de hardware ou compreensão humana.  
**Objetivo:** remover deliberadamente controles centrais do modelo pessoal e exigir que a sequência contínua detecte a sabotagem.

## 1. Campanha adversarial

A campanha `tools/test_semantic_life_redteam.py` recompila o mesmo binário da bancada virtual com uma mutação por vez. Um mutante só é considerado morto quando o cenário `semantic-life` termina com falha. Mutante que não compila é falha do harness, não vitória do redteam.

| Mutação removida | Controle atacado |
|---|---|
| `memory-physical-authority-bypass` | Candidato sem contato e confirmação torna-se memória |
| `functional-conflict-bypass` | Objetivos incompatíveis deixam de ser funcionais |
| `reboot-semantic-scrub-bypass` | Evidência antiga sobrevive ao reboot |
| `reboot-floor-import-bypass` | Floor recuperado não é instalado |
| `divergent-floor-gate-bypass` | Floor inferior substitui silenciosamente o durável |
| `context-expiry-bypass` | Contexto expirado continua parecendo atual |
| `reboot-quarantine-bypass` | Reboot sai da quarentena sem sucessor |
| `conflict-disposition-bypass` | Conflito é apresentado como memória retida |

## 2. Resultado

O redteam matou **8/8 mutantes críticos**. A campanha foi integrada ao `prove.sh` depois da mutação global existente, de modo que a regressão termina em falha caso a campanha semântica seja removida, compile um binário incompleto ou permita uma sabotagem.

A regressão integrada confirmou:

| Camada | Resultado |
|---|---:|
| Bancada virtual | **154 invariantes** |
| Vida semântica contínua | **16/16** |
| Falhas físicas simuladas | **14/14** |
| Presença ambiente | **13/13** |
| Mutação geral | **7/7** |
| Redteam semântico | **8/8** |
| Regressão global | **78 suítes** |

## 3. O que a campanha torna difícil

A campanha torna difícil que o HERUS invente uma biografia coerente a partir de estados parciais. Sem contato e confirmação, um candidato não vira memória. Sem classificação funcional, objetivos conflitantes podem parecer alternativas e o sistema deixa de se abster. Sem scrub, reboot pode reviver fatos antigos; sem floor, uma geração inferior pode reaparecer como autoridade. Sem expiração, contexto vencido pode contaminar a situação atual. Sem quarentena, a recuperação pode parecer uma continuação válida antes de reindexação.

Esses ataques não provam que as memórias verdadeiras serão extraídas da vida real. Eles provam uma propriedade mais estreita: **quando a entrada não sustenta continuidade, o núcleo não deve fabricá-la para parecer inteligente**.

> Uma memória pessoal falsa que soa fluente é mais perigosa que uma resposta vazia. O comportamento correto, em caso de dúvida, é preservar a lacuna.

## 4. Descoberta sobre o próprio redteam

A primeira execução da campanha revelou que o compilador temporário não incluía o diretório `sim/`; todos os mutantes falhavam antes de chegar ao cenário. Isso foi corrigido com um include path explícito. Depois da correção, os oito mutantes passaram a compilar e foram mortos pelo cenário. A distinção foi preservada no relatório para que “não compilou” nunca apareça como controle provado.

## 5. Limites honestos

A campanha não mede relevância humana, aprendizagem real de preferências, qualidade de reconhecimento de voz, conforto do haptic ou aceitação social. Também não prova persistência física sob brownout, energia, NVS, sensores ou comportamento do usuário. Ela prova a resistência lógica da fronteira entre candidato, memória, conflito, reboot e autoridade no host.

O próximo avanço deve atacar combinações de falhas: confirmação ambígua durante power-loss, mudança de preferência na mesma geração, reindexação enquanto há oferta ambiente, conflito após sucessor de reboot e recuperação com adaptador semântico ausente. O critério continuará sendo a recusa de autoridade falsa, não a produção de respostas a qualquer custo.

## Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
python3 tools/test_semantic_life_redteam.py
./prove.sh --quiet
```
