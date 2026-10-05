# Host black-box e rotação de interface

**Status:** evidência host-only de harness; não é operação em hardware real.

## O que mudou

A campanha anterior entregava episódios prontos ao learner. Este harness adiciona um hospedeiro com:

- estado interno privado;
- estado público parcial;
- ações com nomes locais;
- sondagem por interface pública;
- ação equivalente com outro nome em outro hospedeiro;
- rotação da interface durante a tarefa;
- estado oculto que não entra na representação do learner.

O learner não recebe o mapa privado de ações nem o estado interno do hospedeiro.

## Resultado

```text
host de origem: source_a
host alvo: target_x
após rotação: target_new
estado privado exposto: não
espaço de ações alterado: sim
evidência fresca: obrigatória
autoridade concedida pelo learner: não
```

Evidência: [`research/evidence/blackbox_symbiotic_v1.json`](../research/evidence/blackbox_symbiotic_v1.json).

## Interpretação

O resultado demonstra a sequência computacional:

```text
efeito em um host
→ sondagem pública no host seguinte
→ ação com outro nome
→ nova sondagem após rotação
→ nova proposta
```

O learner não copia `source_a`, não usa `target_x` como identidade universal e não reutiliza a ação antiga depois da rotação. Ele depende das observações atuais.

## Limites

Este é um simulador black-box, não um dispositivo externo real. O harness ainda controla o ambiente, a ação e o oracle. Faltam:

- latência e falhas reais;
- ruído de sensores;
- host implementado por terceiro;
- observação parcial mais severa;
- execução física;
- custo energético.

A próxima etapa deve separar o host em um processo independente ou serviço local com protocolo público, para que o learner não compartilhe memória nem classes internas com o hospedeiro.
