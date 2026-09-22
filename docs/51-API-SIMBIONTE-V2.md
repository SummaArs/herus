# API do Symbiont v2 — Etapa 1 da prova ASA

**Status:** pesquisa host-only consolidada; não é firmware, controlador físico nem prova de inteligência geral.

## Propósito

Esta API é o menor núcleo executável para testar se uma identidade persistente pode ocupar hospedeiros diferentes, descobrir capacidades locais por evidência, compor uma competência abstrata e reancorá-la em outro hospedeiro sem depender dos nomes das ações.

A etapa 1 existe antes do wearable para responder uma pergunta mais importante que a escolha do microcontrolador:

> **Há uma propriedade útil de simbiose que sobreviva à troca de corpo, contexto e vocabulário operacional?**

A utilidade social é um requisito de seleção. Uma demonstração só será considerada relevante se reduzir esforço, erro, risco ou dependência de um dispositivo específico para uma pessoa real. Transferir uma Skill entre dois brinquedos é evidência de mecanismo, não ainda evidência de produto social.

## Fronteira pública

O pacote `research.symbiont_v2` expõe os seguintes conceitos:

| Conceito | Função | Não permite |
|---|---|---|
| `SymbiontRuntime` | Mantém a identidade e coordena descoberta, síntese, verificação e transferência | Executar código baixado ou conceder autoridade física |
| `HostAdapter` | Fronteira mínima entre núcleo e hospedeiro | Expor memória interna ou um perfil completo obrigatório |
| `Observation` | Estado observado com digest determinístico | Provar autenticidade de um sensor físico |
| `Evidence` | Registro de ação, estado anterior, estado posterior e efeito | Converter observação em permissão |
| `AbstractSkill` | Contrato de competência independente dos nomes locais | Ser promovida sem verificação fresca |
| `PersistentMemory` | Guardar somente Skills verificadas e a identidade | Guardar automaticamente o contexto do hospedeiro anterior |
| `DiscoveryBudget` | Limitar sondas e candidatos | Tornar exploração ilimitada |

O ciclo normativo é:

```text
bind → observe → probe → record evidence → learn model
     → synthesize → verify → promote → rebind → transfer
```

## Invariantes da etapa 1

1. A identidade `herus_id` sobrevive ao `rebind`.
2. O contexto, o modelo do mundo e a evidência do hospedeiro anterior não são reutilizados como observação do novo hospedeiro.
3. Uma Skill candidata não é promovida sem verificação.
4. A transferência usa contratos de efeito observados, não nomes de ações.
5. Evidência conflitante torna a previsão indisponível.
6. Limites de descoberta são explícitos.
7. O pacote não possui rede, geração de código, memória durável de produto ou autoridade física.
8. Uma falha de transferência resulta em `False`/abstenção, nunca em execução especulativa.

## O que a etapa prova

O benchmark atual demonstra uma hipótese estreita: em dois hospedeiros discretos com efeitos observáveis e equivalentes, uma competência verificada pode ser reancorada quando os identificadores concretos das ações mudam.

Isso permite afirmar apenas **grounding simbiótico limitado e reproduzível**. Não permite afirmar compreensão geral, aprendizado aberto, robustez robótica, segurança física, benefício humano ou AGI.

## Critério de utilidade

Antes de conectar esta API a um wearable, a etapa seguinte deve substituir os `ToyHost` por hospedeiros black-box e medir uma tarefa que tenha valor além do benchmark. Exemplos possíveis incluem preservar uma configuração segura entre dispositivos de acessibilidade, transferir uma rotina de comunicação essencial entre interfaces diferentes ou manter uma preferência humana sem carregar dados privados indevidos.

A tarefa será aceita como socialmente relevante somente se houver:

- usuário ou cenário de necessidade identificável;
- redução mensurável de erro, tempo, custo ou dependência;
- benefício que não dependa de um nome artificial de ação;
- abstenção segura quando a transferência não for sustentada;
- comparação com uma solução convencional;
- avaliação de privacidade, acessibilidade e falhas.

## Reprodução

```bash
PYTHONPATH=. python3 -m unittest research.symbiont_v2.test_symbiont -v
PYTHONPATH=. python3 -m research.symbiont_v2.benchmark
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
```

Os resultados desta etapa permanecem separados do firmware.

## Etapa 2 — protocolo black-box

A fronteira agora é exercitada com `BlackBoxHost`, que expõe somente `host_id`,
recursos, espaço público de ações, observação, execução e reset. O runtime não
consulta o mapa interno de efeitos. Ele infere os efeitos exclusivamente pela
diferença entre observações públicas antes e depois de uma sondagem.

`propose_transfer()` substitui a antiga interpretação ambígua de transferência:
produz um `TransferProposal` ou abstém-se. Construir a proposta não chama o
executor do hospedeiro. `transfer()` permanece apenas como wrapper booleano de
compatibilidade e também não executa ações. Correspondências de efeito com mais
de uma ação são ambíguas e falham fechado.

Este é um avanço metodológico, não uma prova de generalidade. O próximo gate
continua sendo o holdout: hospedeiros desconhecidos, evidência contraditória,
mudança temporal, efeitos parcialmente observáveis e ações perigosas. Nenhum
resultado desta API autoriza um atuador físico.
