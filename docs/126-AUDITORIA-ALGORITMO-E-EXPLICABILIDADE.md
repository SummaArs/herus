# Auditoria do Symbiotic Learning v2

**Data:** 2026-10-07  
**Escopo:** núcleo incremental, fachada `Herus`, contratos de segurança, explicabilidade e validade experimental.

## 1. Mapa mínimo do algoritmo

1. `Episode` representa uma observação `(estado antes, ação, estado depois)` e metadados de custo, risco, contexto, tempo, proveniência e resultado.
2. O efeito é o delta determinístico entre os estados.
3. `observe()` aceita somente observações finitas dentro dos orçamentos e sem risco acima da autoridade local.
4. `induce()` agrupa episódios por `(contexto, efeito)` e só cria uma hipótese quando a ação é identificável.
5. Deriva temporal ou alias de ação produz `ABSTAIN`.
6. `propose()` procura correspondência exata de efeito/contexto dentro de custo, risco e janela temporal; nunca executa.
7. `Herus` expõe a biblioteca em uma fachada única, com `data()`, `program()`, `observe()`, `propose()` e `inspect()`.

## 2. Achados críticos

### Corrigidos nesta rodada

- **Explicabilidade incompleta:** a proposta informava apenas `reason` e `evidence_count`; agora expõe `evidence_ids` e uma `explanation` determinística.
- **Hipótese sem trilha de auditoria:** `SkillHypothesis` agora expõe os IDs dos episódios que formaram cada hipótese.
- **Entrada vazia:** ações vazias agora são rejeitadas fail-closed.
- **Risco negativo:** valores de risco negativos agora são rejeitados, evitando que um metadado inválido pareça mais seguro que risco zero.

Os IDs são derivados de uma representação determinística do episódio e permitem ligar uma decisão ao conjunto exato de observações sem conceder autoridade de execução.

### Mantidos como limites explícitos

- A confiança em milésimos ainda é uma **escala heurística**, não uma probabilidade calibrada.
- A indução usa correspondência exata; não há generalização semântica comprovada fora do espaço observado.
- `provenance` é registrado, mas ainda não é uma assinatura criptográfica nem uma cadeia de custódia externa.
- O algoritmo é proposal-only: não há execução, interação com rede ou controle de host.
- A evidência de benchmark não autoriza claims de SOTA geral, superioridade a transformers, RL geral, simbiose geral ou AGI.

## 3. Critério de progresso

Esta rodada melhora **auditabilidade e validade operacional**, não afirma uma vitória de acurácia. O próximo aumento científico exige medir, em dados reais e holdouts intocados:

- precisão seletiva versus cobertura;
- tamanho de conjunto conformal;
- custo de CPU, memória e latência;
- estabilidade em múltiplas sementes;
- transferência para MInDS-14 e um terceiro domínio independente;
- comparação pareada com baselines clássicos e transformer sob o mesmo protocolo.

## 4. Resultado verificável

A nova suíte adiciona invariantes para:

- presença e cardinalidade da trilha de evidência;
- explicação legível da proposta;
- rejeição de ações vazias;
- rejeição de risco negativo.

A melhoria é deliberadamente pequena e local: preserva compatibilidade dos construtores existentes, mantém a fronteira sem autoridade e não transforma uma hipótese em uma alegação de inteligência geral.
