# Object Lock v1 — política de decisão

## Regra central

HERUS não executa ações. Ele retorna uma proposta verificável ou `ABSTAIN`.

Uma proposta só é elegível quando:

1. o contexto requerido é compatível com o contexto atual;
2. o estado atual e as precondições são compatíveis;
3. existe efeito-alvo proveniente de fonte independente;
4. há evidência positiva e negativa contabilizada;
5. não há conflito não resolvido entre ações;
6. a evidência está dentro da janela temporal;
7. risco, custo e atraso estão dentro do orçamento;
8. a proveniência e o verificador estão presentes.

## Abstenção obrigatória

Abster-se quando houver:

- contexto vazio sem semântica explícita;
- ação ambígua;
- falha negativa não compensada;
- evidência antiga;
- estado incompatível;
- target derivado do rótulo do holdout;
- qualquer divergência de identidade;
- ausência de verificador.

## Semântica de suporte

Até existir validação futura independente, `confidence` não será usado. O campo deve ser interpretado como suporte/estabilidade da evidência, nunca como probabilidade de sucesso futuro.

## Claims permitidos

O primeiro experimento pode testar apenas precisão entre propostas aceitas, cobertura e risco no domínio congelado. Nenhum resultado pode ser promovido automaticamente para causalidade, transferência, segurança de produção, SOTA ou AGI.
