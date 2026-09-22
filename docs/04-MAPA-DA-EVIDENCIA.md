# HERUS — Mapa da evidência

## Veredicto atual

O HERUS tem bastante evidência de **engenharia de contratos host-only**. Ele ainda tem pouca evidência de **valor humano e realidade física**. O mapa abaixo impede que as duas categorias sejam misturadas.

## O que está demonstrado no ambiente local

| Domínio | Demonstração atual | Evidência | Limite |
|---|---|---|---|
| Firmware | Invariantes de núcleo, memória, confiança e interação | `./prove.sh --quiet` | Não é placa física. |
| Simulação | Mundo com distância e adversários | `sim/` e proof | Não mede RF, energia ou ergonomia. |
| Semantic IR | Schema, validação e identidade canônica | `research/semantic_ir.py` | Não é compreensão aberta. |
| Symbiont | Identidade persistente, descoberta e Skill finita | `research/symbiont_v2/` | Não é adaptação geral. |
| Holdout histórico | Contraexemplos de observabilidade e autoridade | `research/holdout_benchmark_v1.json` | Não é holdout externo definitivo. |
| Causal Bridge | Runtime e oracle separados, efeitos ocultos bloqueados | `research/bridge_causal.py` | Campanha pequena e local. |
| HERUS Bridge | Proposta, prévia, cancelamento e parada sem efeito externo | `research/bridge_product.py` | Nenhuma sessão humana ainda. |
| Proveniência | Digests de inputs locais | `research/software_provenance_manifest.json` | Manifesto não assinado externamente. |

## O que não está demonstrado

Ainda não existe evidência suficiente para afirmar simbiose útil, execução segura, benefício humano, acessibilidade, confiança de campo, segurança de rádio, autonomia, alcance físico, desempenho térmico, robustez a hospedeiros arbitrários, compreensão de linguagem aberta ou AGI.

A existência de um teste para um comportamento não prova que o comportamento seja amplo. A existência de um módulo de executor não prova que exista um executor seguro. A existência de um protocolo social não prova que pessoas tenham participado.

## Hierarquia de força

1. **Código e teste local:** mostra que um comportamento delimitado foi exercitado.
2. **Mutação adversarial:** mostra que o teste falha quando uma propriedade é removida ou falsificada.
3. **Processos separados:** reduzem circularidade entre sistema e oracle.
4. **Revisão independente:** testa se outra pessoa entende e reproduz o resultado.
5. **Bancada física:** mede o hospedeiro real.
6. **Sessões humanas:** testam compreensão, custo, erro e valor.
7. **Replicação externa:** sustenta uma afirmação pública forte.

O HERUS está entre os níveis 1 e 3 para seu mecanismo principal. Ainda precisa dos níveis 4–7 para qualquer claim de produto.

## O que os estados significam

`SUPPORTED` é um estado reservado, não um resultado atual. `SAFE_BUT_UNPROVEN` significa que uma proposta foi construída sob contratos locais, mas sem atestação externa. `mechanism_only` exige mecanismo, segurança, privacidade e reprodutibilidade completos; não é sinônimo de utilidade. `useful_symbiosis` exige também evidência humana comparada a baseline congelado. Hoje, a classificação correta do programa é `not_proven`.

## Como uma nova evidência entra

Toda evidência nova precisa declarar pergunta, hipótese, baseline, comando, digest, resultado bruto, resultado negativo e limite. O documento deve aparecer perto do artefato que o produz. O README só deve promover um claim depois que o comando e o teste existirem.

## Referências

[1]: ../research/evidence/final_audit/00-synthesis.md "Síntese da auditoria profunda"
[2]: ../research/evidence/bridge_causal_v1.json "Resultado bruto causal"
[3]: 52-DEFINICAO-SIMBIOSE-UTIL.md "Contrato de simbiose útil"
[4]: 56-RODADA-DECISIVA-HERUS-BRIDGE.md "Rodada decisiva"
