# Refinamento de políticas entre modelo e execução

## Problema

Provar uma máquina abstrata e provar que a implementação concreta possui transições compatíveis não basta quando a política também muda de representação. Uma ação abstrata pode possuir outro nome, um evento concreto pode ser uma especialização de um evento abstrato e várias representações concretas podem apontar para o mesmo símbolo do modelo.

## Contrato implementado

`research/critical_policy_refinement.py` verifica, em domínio finito, que uma política concreta total preserva a decisão da política abstrata sob mapas explícitos de estados, entradas e ações.

Para cada par `(estado concreto, entrada concreta)`, o verificador calcula o par abstrato mapeado e exige que:

```text
ação_concreta mapeada = ação_abstrata esperada
```

O verificador rejeita mapa incompleto, símbolo mapeado fora do modelo abstrato e política não total. Uma divergência retorna `COUNTEREXAMPLE` com a regra concreta e a regra abstrata conflitantes.

## Exemplo

| Representação concreta | Representação abstrata |
|---|---|
| `c_safe` | `safe` |
| `packet_loss` | `loss` |
| `safe_retain` | `safe_hold` |

A correspondência só é aceita quando todas as regras concretas preservam a política abstrata sob esses mapas. O verificador não adivinha mapas e não trata nomes semelhantes como equivalência.

## Limites

A garantia é relativa às duas máquinas, às duas políticas e aos mapas fornecidos. Ela não prova que o mapa foi obtido corretamente do firmware, que a política abstrata é segura fora do modelo, que a implementação cobre todo o mundo físico ou que uma chamada real alcança a regra correspondente. Essas propriedades continuam pertencendo ao refinamento de máquinas, ao auditor de caminhos e à validação do sistema.

O resultado é, portanto, uma camada de ligação formal entre síntese e implementação, não uma certificação de segurança completa.
