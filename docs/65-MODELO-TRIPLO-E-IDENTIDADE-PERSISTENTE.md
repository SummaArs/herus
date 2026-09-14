# HERUS: modelo triplo e identidade persistente

## Decisão

O anexo propõe que o HERUS mantenha três modelos distintos: **World Model**, **Host Model** e **Self Model**. A proposta é válida e foi incorporada em uma forma finita e testável no módulo `research/symbiotic_models.py`.

A separação não é apenas terminológica:

| Modelo | Pergunta | Evidência mínima |
|---|---|---|
| World Model | O que foi observado fora do HERUS? | Observação com sujeito, predicado, valor e proveniência |
| Host Model | Em qual hospedeiro estou e quais recursos ele declarou? | `HostProfile` validado e digest canônico |
| Self Model | O que este HERUS pode propor neste hospedeiro? | Identidade, digest do host, Skills comprovadas e limites explícitos |

Misturar os modelos permitiria que uma observação sobre o mundo se tornasse uma permissão, ou que uma capacidade declarada pelo hospedeiro fosse confundida com uma capacidade verificada do HERUS. O contrato impede essas duas confusões.

## Identidade e hospedeiro

`PersistentIdentity` representa a continuidade do HERUS. A identidade possui um identificador estável e registra eventos de vinculação e desvinculação. O `host_id` e o digest do hospedeiro são vínculos substituíveis, não a identidade do HERUS.

Assim, a mudança de hospedeiro é representada como:

```text
HERUS-001 + host-A
       ↓ unbind/bind
HERUS-001 + host-B
```

A continuidade é preservada apenas se o novo `HostProfile` passar pela validação existente. Um digest alterado invalida o snapshot simbiótico; não produz adaptação silenciosa.

## Regras fail-closed

O contrato implementado aplica as seguintes regras:

1. Uma observação do mundo precisa de proveniência e não pode ser marcada como `CONFIRMED` sem confiança `PROVEN`.
2. Um host só pode ser vinculado se seu perfil finito for válido.
3. A descoberta de uma Skill permite no máximo uma proposta (`PROPOSE`).
4. `execute()` sempre retorna `ABSTAIN` neste estágio; autorização de execução pertence a um contrato externo e explícito.
5. Deriva do digest do host torna o snapshot inconsistente e bloqueia a operação.
6. Desvincular o host remove o `Self Model` operacional; a identidade permanece, mas não há capacidade ativa.

## O que foi deliberadamente rejeitado

O anexo contém ideias úteis como personalidade persistente, sons semânticos, autonomia geral e a frase “AGI sem humano”. Elas não foram implementadas como capacidades porque ainda não possuem critérios operacionais suficientes ou poderiam ampliar autoridade sem evidência.

Também não foi adicionada uma rede neural, uma LLM local ou uma ontologia aberta. A arquitetura atual continua finita, auditável e compatível com o princípio de que componentes aprendidos podem gerar hipóteses, mas não autoridade.

## Evidência produzida

A suíte `research/test_symbiotic_models.py` cobre continuidade de identidade, consistência dos três modelos, deriva de host, desvinculação, observação não comprovada e rejeição de autoridade desconhecida. Ela é executada pelo `prove.sh` como o gate **40/40**.

O resultado prova somente invariantes do modelo em software. Não prova adaptação a qualquer host, generalização aberta, consciência, AGI, operação física, segurança de atuadores ou compreensão humana de feedback háptico. Essas propriedades continuam pendentes de experimentos próprios.

## Próximo experimento válido

O próximo passo científico não é adicionar mais metáforas ou módulos. É executar o **Unknown Host Challenge** em dois hosts simulados com perfis parcialmente ocultos: exigir descoberta, modelagem, proposta e abstention sob deriva. O experimento deve medir taxa de descoberta correta, falsos positivos, tempo, orçamento e ausência de escalada de autoridade. A etapa física só pode substituir a simulação depois dos gates B1/B2.
