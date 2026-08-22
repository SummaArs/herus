# HERUS — Vida semântica contínua

**Estado:** host-side integrado; não é uma prova de percepção humana nem de hardware.  
**Objetivo:** deixar de testar apenas eventos isolados e exercitar uma sequência de vida semântica em que preferências, objetivos, contexto, memória, conflito, expiração e reboot se relacionam ao longo de gerações.

## 1. A composição implementada

O cenário `semantic-life` combina três camadas já existentes ou explicitamente adaptadas. O `personal_sim` fornece relógio monotônico, energia, alimentação, haptic e contato. O `memory_semantic_evidence` real fornece fatos tipados, proveniência opaca, supersession, conflito, expiração, floor e consulta limitada. O `semantic_life` apenas compõe essas camadas em uma sequência determinística; ele não adiciona transcrição, identidade, localização, embedding ou texto.

| Camada | Responsabilidade | Autoridade |
|---|---|---|
| Evento tipado | Representar uma observação semântica já classificada | Nenhuma |
| Presença ambiente | Decidir silêncio, espera, oferta e contato | Apenas microoferta local |
| Candidato de memória | Representar uma possibilidade transitória | Nenhuma até confirmação |
| Índice semântico | Reter fato revisado com card e receipt opacos | Leitura local limitada |
| Reboot | Limpar evidência ativa e importar floor | Quarentena até sucessor válido |
| Consulta | Recuperar match, ambiguidade ou contradição | Nunca transmite nem age |

## 2. Sequência exercitada

A sequência começa com uma preferência observada sem autoridade física; ela é descartada e não entra no modelo. A mesma classe de preferência, depois de contato e confirmação explícitos, entra no índice local e pode ser consultada com sua proveniência. Uma segunda preferência compatível permanece representada e produz ambiguidade, em vez de ser eliminada por uma escolha arbitrária.

Em seguida, dois objetivos funcionais incompatíveis entram em conflito. A consulta retorna `MSE_QUERY_CONTRADICTED` e não seleciona card algum. Um contexto com validade curta expira quando a vida avança. O reboot limpa o índice, importa apenas o floor semântico e impede que preferências pré-reboot reapareçam. Um sucessor acima do floor pode ser reindexado; um floor recuperado inferior ao floor durável é recusado sem mutação.

| Propriedade | Resultado |
|---|---:|
| Cenário contínuo `semantic-life` | **16/16** |
| Bancada virtual completa | **140 invariantes** |
| Regressão global | **78 suítes** |
| Mutação global | **7/7 mutantes mortos** |
| Proveniência | **válida; simulador agora protegido pelo manifesto** |

## 3. O que esta etapa realmente demonstra

A etapa demonstra que o HERUS pode manter um modelo semântico seletivo em host sem confundir relevância com autoridade. Ela demonstra também que memória pessoal não precisa ser uma lista de acontecimentos: pode conter alternativas, conflitos e validade temporal, e a resposta correta pode ser não escolher.

A continuidade ainda é uma continuidade de **símbolos e estados tipados**, não de uma vida humana observada. O simulador não sabe se uma preferência era verdadeira, se um objetivo era importante ou se o contexto foi compreendido. Ele recebe essas propriedades como entradas controladas e verifica se, uma vez recebidas, o sistema se comporta de modo conservador e explicável.

## 4. Descobertas de engenharia

A composição encontrou uma fronteira correta entre percepção e memória. O adaptador semântico pode produzir um candidato, mas não pode promover o candidato por conta própria. A memória precisa de contato e confirmação explícitos, e o índice mantém a proveniência do card e do receipt sem armazenar conteúdo humano.

A composição também confirmou que reboot não é apenas uma limpeza de RAM. O índice começa vazio, recebe somente um floor autorizado e permanece incapaz de responder com conhecimento pré-reboot até receber uma evidência sucessora acima da fronteira. Divergência de floor é falha, não degradação silenciosa.

> Uma inteligência pessoal confiável não é a que sempre responde; é a que mantém continuidade sem transformar hipótese, memória antiga ou conflito em certeza atual.

## 5. Limites honestos

Ainda não foi demonstrada a qualidade da percepção de voz, a relevância real de uma memória, a aprendizagem de preferências a partir da vida cotidiana, a interpretação de objetivos ou a compreensão humana dos sinais hápticos. Também continuam pendentes flash/NVS, brownout, energia real, latência elétrica, sensores, conforto e contato humano.

O próximo avanço deve atacar essa camada com cenários adversariais: memória falsa com confirmação ambígua, mudança silenciosa de preferência, objetivo contraditório após reboot, reindexação abaixo do floor, insistência após ausência de contato e tentativa de converter conflito em escolha. O resultado esperado não é uma porcentagem de acerto humano; é uma política que se recusa a inventar continuidade quando os dados não sustentam essa conclusão.

## Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
(cd sim && make -s semantic-life)
./prove.sh --quiet
```
