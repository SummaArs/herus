# Auditoria crítica: erros cometidos e correções permanentes

Esta auditoria não é publicidade. Ela registra onde o HERUS poderia ter produzido uma conclusão mais forte do que os dados sustentavam.

## 1. Confundir implementação com capacidade

**Erro:** tratar módulos, testes e nomes arquiteturais como evidência de uma capacidade geral.

**Correção:** cada afirmação deve apontar para um dataset, protocolo, holdout, métrica e artefato reproduzível.

## 2. Chamar proxy de paradigma completo

**Erro:** comparar um bandit textual com aprendizado por reforço geral.

**Correção:** usar o rótulo `proxy` sempre que não houver estados, ações causais, transições e recompensas ambientais.

## 3. Falar em vitória contra transformers sem executar transformer

**Erro:** sugerir superioridade teórica sem comparação direta.

**Correção:** nenhuma afirmação sobre transformers será permitida sem modelo, tokenizer, parâmetros, seed, orçamento, dados e holdout fixados.

## 4. Usar um único domínio real

**Erro:** extrapolar de MIntRec para simbiose geral.

**Correção:** MIntRec é apenas um domínio textual. A próxima conclusão geral exigirá múltiplos datasets reais e um holdout de domínio novo.

## 5. Calibrar no holdout

**Erro evitado:** a calibração atual usa S05 e avalia S06. Esta separação deve permanecer obrigatória.

**Correção:** o holdout final nunca escolhe limiar, hiperparâmetro, representação ou regra.

## 6. Otimizar cobertura ignorando falsos aceites

**Erro:** aumentar respostas simplesmente reduzindo o limiar.

**Correção:** toda melhoria deve mostrar a curva cobertura–precisão e o custo do erro. Cobertura sem segurança não é progresso.

## 7. Medir apenas acurácia

**Erro:** uma métrica única pode esconder abstenção, desequilíbrio e deriva.

**Correção:** reportar acurácia, macro-F1, cobertura, precisão seletiva, matriz por classe, custo, latência e memória quando disponíveis.

## 8. Confundir dataset real com problema HERUS

**Erro:** tratar rótulos MIntRec como eventos HERUS.

**Correção:** nenhum mapeamento sem anotação independente. Resultados MIntRec medem a tarefa original, não o núcleo de autoridade do HERUS.

## 9. Recalcular trabalho caro dentro de loops

**Erro de engenharia:** a primeira calibração reconstruía o modelo para cada limiar.

**Correção:** materializar escores uma vez, medir custo e testar limites de orçamento.

## 10. Proveniência como etapa posterior

**Erro de processo:** alterar artefatos e corrigir hashes somente depois do gate.

**Correção:** o gate de proveniência deve rodar junto da validação, antes da publicação.

## 11. Meta de 100% mal definida

**Erro:** tratar 100% como conclusão absoluta de um algoritmo universal.

**Correção:** 100% significa cumprir um contrato de versão específico, com escopo, datasets, baselines, segurança e limites declarados. Não significa vencer todo ML ou resolver AGI.

## Regras permanentes

1. Sem dado, sem alegação.
2. Sem baseline equivalente, sem vitória.
3. Sem holdout independente, sem generalização.
4. Sem custo, sem alegação de eficiência.
5. Sem abstenção testada, sem alegação de segurança.
6. Sem transformer executado, sem alegação contra transformer.
7. Sem domínio novo, sem alegação de simbiose geral.
