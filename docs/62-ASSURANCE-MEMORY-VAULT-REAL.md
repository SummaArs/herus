# Assurance do memory vault real

## Marco

O caso `research/evidence/memory_vault_assurance_case.json` é a primeira fixture de assurance composta derivada de um subsistema real do firmware HERUS: o `memory_vault`.

A seleção foi feita porque o contrato público declara explicitamente quatro estados (`UNINITIALIZED`, `READY`, `SEALED`, `BLOCKED`), erros de inicialização, autorização, cartão, raiz, armazenamento, autenticidade, rollback e erase, além de uma regra fail-closed para persistência.

## Cadeia verificada

```text
memory_vault.h / memory_vault.c
        ↓
modelo finito de lifecycle
        ↓
política de retenção, sealing e bloqueio
        ↓
mapas de símbolos C11 para símbolos abstratos
        ↓
refinamento da máquina
        ↓
refinamento da política
        ↓
confirmação do sink memory_vault_seal
        ↓
ASSURED
```

O resultado serializado é gerado com:

```text
make -C research memory-vault-assurance
```

O resultado atual é `ASSURED`, com ambas as verificações `VERIFIED`, ambas as relações `REFINED` e o caminho de persistência marcado como `COVERED` pelo perfil HCAE.

## O que foi realmente demonstrado

Foi demonstrado que o modelo finito declarado, a política declarada, os mapas declarados e o caminho crítico declarado são mutuamente consistentes sob o verificador atual. A mutação de remover cobertura do sink produz `BLOCKED`. A mutação de trocar a ação de bloqueio por retenção, mantendo a transição executável, produz `COUNTEREXAMPLE`.

## O que não foi demonstrado

A extração do modelo ainda é manual e baseada no contrato público do header e no corpo revisado do módulo. Não existe ainda um extrator automático que prove correspondência entre cada ramo C11 e a tabela finita. `COVERED` continua significando cobertura estrutural local, não dominância completa de todos os caminhos nem ausência de comportamento indefinido.

A fixture também não prova que a raiz criptográfica, o backend de armazenamento, o contador de geração ou os sensores físicos se comportam corretamente em hardware. O resultado não é certificação de segurança nem autorização de produção.

## Próximo teste de falsificação

O próximo backend deve comparar a tabela declarada com uma extração estrutural automática dos retornos e atribuições de `memory_vault.c`. Qualquer ramo não representável, chamada indireta ou correspondência ambígua deve gerar `UNKNOWN` ou `BLOCKED`, nunca ampliar silenciosamente a tabela.
