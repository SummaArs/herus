# H1 — Embedded equivalence

H1 é a implementação do HERUS no ESP32-S3. O objetivo não é reproduzir o custo do computador; é preservar as decisões protegidas sob restrições reais.

A primeira equivalência usa os vetores dourados H0. Para cada entrada, H1 deve preservar a classe, a representação ou a abstention, o motivo de bloqueio quando aplicável, a autoridade e a impossibilidade de execução. Pequenas diferenças numéricas de confiança podem ser toleradas apenas quando não cruzam um limiar de decisão.

A ponte C11 existente já executa inferência inteira, escolhe representação por orçamento e mantém execução em zero. A prova atual ainda é host-side: ela demonstra compilação e invariantes do C11, não desempenho ou energia do ESP32-S3. O próximo passo físico será executar os mesmos vetores na placa e registrar RAM, ciclos, latência, temperatura e reinicializações.

A hierarquia de prova é:

```text
H0: decisão canônica no computador
H1: mesma decisão sob C11 e orçamento embarcado
H2: mesma decisão com sensores, rádio, LRA e bateria
H3: mesma identidade e contratos após migração de hospedeiro
```

Em nenhum nível uma descoberta do hospedeiro concede autoridade. Se H0 retorna `ABSTAIN`, H1 não pode retornar `PROPOSE` ou `EXECUTE` para o mesmo caso protegido.
