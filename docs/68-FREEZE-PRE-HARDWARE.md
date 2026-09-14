# Freeze pré-hardware do HERUS

## Decisão

A camada host-only do HERUS está congelada para novas alegações de simbiose. A partir deste ponto, novas evidências sobre hospedeiros vivos, energia, latência física, rádio, memória embarcada, ruído e atuadores exigem hardware real.

O congelamento não afirma que o HERUS está pronto para produção. Ele afirma que o conjunto atual de contratos, provas e limites foi executado sem falhas no ambiente host-only disponível.

## Última correção crítica

A prova anterior verificava que três registros possuíam `host_id` diferentes, mas isso não demonstrava uma transição real de hospedeiro. O modelo agora possui `rebind()`.

O ciclo validado é:

```text
attach(host-A)
→ detach(host-A)
→ bind(host-B)
```

O novo vínculo conserva `herus_id`, mas limpa o `WorldModel` e não permite propostas baseadas nas Skills do hospedeiro anterior. O novo hospedeiro inicia com contexto próprio e autoridade `NONE`.

## Estado final host-only

| Propriedade | Estado |
|---|---|
| Identidade persistente entre hospedeiros | Provada |
| Proveniência por artefato real local | Provada |
| Rebind com limpeza de contexto | Provado |
| Deriva de perfil detectável | Provada |
| Execução sem autorização externa | Bloqueada |
| Transferência automática de autoridade | Bloqueada |
| Dados financeiros convertidos em ordem | Bloqueado |
| Energia física | Não medida |
| Latência física | Não medida |
| Rádio e antena reais | Não medidos |
| Memória e temperatura do ESP32-S3 | Não medidas |
| Atuador LRA | Não validado |

## Critério de bloqueio

Não serão adicionados percentuais de autonomia, consumo ou desempenho físico baseados em simulação. Não serão declarados robôs, servidores ou sistemas financeiros como controlados pelo HERUS sem uma integração viva, uma autoridade explícita e uma evidência reproduzível.

O próximo trabalho autorizado depende da chegada e identificação do hardware. A sequência física começa por B1/B2: identidade visual, revisão da placa, variante de rádio, pinagem e boot serial. Até esses gates passarem, a implementação permanece em `pre_hardware`.

## Resultado

O HERUS não está “perfeito” no sentido de produto final ou sistema universal. Ele está **fechado e suficientemente provado para parar a expansão de alegações host-only**. A próxima evolução relevante precisa vir de uma medição física, não de mais complexidade abstrata.
