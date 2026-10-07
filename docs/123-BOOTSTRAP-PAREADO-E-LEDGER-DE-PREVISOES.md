# Bootstrap pareado e ledger de previsões

A rodada finalmente arquiva previsões por exemplo no holdout temporal S06 do MIntRec para dois métodos locais:

- Naive Bayes multinomial;
- memória finita de contexto do HERUS.

Cada linha contém o índice, o rótulo, as duas previsões, aceitação e correção. O bootstrap reamostra os mesmos índices para os dois métodos, preservando o pareamento.

## Limite importante

O DistilBERT não entra no bootstrap desta rodada porque sua execução anterior salvou métricas agregadas, não o vetor de previsões. A comparação transformer continua válida como resultado agregado, mas ainda não como teste pareado.

O próximo experimento deve alterar o benchmark transformer para salvar exatamente o mesmo ledger e então comparar:

- Naive Bayes;
- HERUS seletivo;
- DistilBERT multilíngue;
- Tensor-Train.

Isso é um avanço de rigor, não uma alegação automática de SOTA.
