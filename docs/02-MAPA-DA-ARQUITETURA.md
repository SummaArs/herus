# HERUS — Mapa da arquitetura

## A arquitetura em uma visão

O HERUS deve ser lido como uma sequência de fronteiras, não como um bloco inteligente único:

```text
Pessoa
  ↓ intenção, confirmação, cancelamento e parada
Interface local / Bridge
  ↓ proposta revisável
ASA Core / Symbiont
  ↓ identidade persistente + Skill finita + orçamento
Host Model + World Model
  ↓ observações públicas e evidência
Hospedeiro
  ↓ sensores, memória, rádio ou atuadores locais
Mundo externo
```

Cada seta é uma fronteira. Uma fronteira pode transportar dados, uma proposta ou uma evidência. Ela não concede autoridade automaticamente.

## As quatro entidades principais

### Self Model

É a continuidade do HERUS. Contém a identidade persistente e o repertório de Skills verificadas. A identidade não deve carregar automaticamente contexto ou permissão de um hospedeiro anterior.

### Host Model

É o perfil do hospedeiro atual: recursos, ações públicas, chaves de estado, época e digest. Ele descreve o vínculo atual. Não é a identidade do HERUS.

### World Model

É o conjunto de transições observadas no hospedeiro. Ele registra o que uma ação parece fazer em um estado específico. Conflitos devem remover a previsão, não escolher uma versão conveniente.

### Human Model

Não é um módulo cognitivo interno. É o conjunto de contratos que mantém a pessoa no comando: prévia, confirmação, cancelamento, abstenção, parada, consentimento e recuperação.

## O ciclo operacional

1. **Identificar:** o hospedeiro apresenta um perfil observável.
2. **Observar:** o runtime coleta sinais públicos dentro de um orçamento.
3. **Aprender:** transições são associadas a evidências e digests.
4. **Compor:** o runtime busca uma Skill finita que satisfaz uma meta.
5. **Verificar:** a Skill é reavaliada contra as evidências disponíveis.
6. **Propor:** a transferência para outro host produz uma proposta.
7. **Revisar:** uma pessoa vê o que seria feito.
8. **Decidir:** confirmar, cancelar, abster ou parar.
9. **Executar:** somente uma autoridade externa comprovada poderia liberar esta fase; o estado atual do projeto não a fornece.

## Quatro distinções obrigatórias

| Não confundir | Distinção correta |
|---|---|
| Capacidade e autoridade | Saber como fazer não concede permissão para fazer. |
| Proposta e execução | Uma proposta é um plano serializado; execução exige um dispatcher e autoridade independente. |
| Observação e efeito fechado | Observar uma resposta não prova que efeitos ocultos não ocorrerão. |
| Teste verde e evidência causal | Passar fixtures prova apenas os asserts declarados; mutações precisam demonstrar que o teste detecta fraude. |

## Onde entram as grandes ideias

A tese original de comunicador semântico usa o **Lexicon**, HCP, SBC/MAP-B, Weave e Echo. A tese posterior de simbiose usa **Self Model**, Host Model, World Model, Skills, observabilidade e transferência. Elas compartilham identidade, significado finito e limites de autoridade, mas não devem ser tratadas como se fossem a mesma prova.

O firmware implementa contratos físicos e de segurança. O Python explora o mecanismo de transferência e a Semântica IR. O simulador verifica mundos controlados. O HERUS Bridge conecta a pesquisa a uma tarefa humana pequena.

## O que a arquitetura ainda não resolve

Ela ainda não resolve dinâmica contínua, incerteza probabilística robusta, causalidade geral, linguagem aberta, morfologia ampla, execução física segura, benefício humano, privacidade em campo ou adoção. Esses itens ficam no futuro porque exigem evidência diferente, não porque faltam nomes no diagrama.

## Referências

[1]: 48-ARQUITETURA-FINITA-E-LINGUAGEM.md "Arquitetura finita e linguagem"
[2]: 51-API-SIMBIONTE-V2.md "API do Symbiont v2"
[3]: 54-CORRECAO-CONTRATO-OBSERVABILIDADE.md "Contrato de observabilidade"
[4]: 56-RODADA-DECISIVA-HERUS-BRIDGE.md "HERUS Bridge"
