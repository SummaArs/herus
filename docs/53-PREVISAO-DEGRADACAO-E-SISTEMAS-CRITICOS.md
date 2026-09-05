# Previsão de degradação e verificação de sistemas críticos

## Veredito executivo

A pesquisa anexada contém uma direção útil para o HERUS, mas não deve ser incorporada como teoria universal de “captura de entropia”. O núcleo aproveitável é **prognóstico de degradação sob modelo, sensores, limites e incerteza explícitos**. Isso se aproxima de prognostics and health management: dados temporais podem apoiar detecção de falha, diagnóstico, avaliação de saúde e estimativa de vida útil remanescente. Repositórios públicos da NASA, por exemplo, descrevem conjuntos de dados de prognóstico como séries temporais que percorrem estado nominal até falha [4].

A termodinâmica fornece restrições físicas, não um relógio universal de ruptura. A primeira lei trata da conservação de energia dentro do domínio definido; a segunda lei trata da direção de processos irreversíveis e da entropia total do sistema apropriado [2] [3]. Uma variável chamada “entropia do sistema” não pode ser usada sem especificar fronteira, estado, equilíbrio, fluxo de calor, geração interna e escala temporal. O NIST observa que a termodinâmica define entropia em equilíbrio e que, para um sistema isolado em processo irreversível, a entropia aumenta; isso não implica que uma medida de não equilíbrio cresça monotonamente em todos os instantes [1].

## Classificação das afirmações

| Afirmação da pesquisa | Classificação | Tratamento no HERUS |
|---|---|---|
| Sistemas físicos e digitais podem degradar e falhar | Válida em sentido geral | Traduzir em variáveis observáveis e limites de operação |
| A conservação de energia não elimina a entropia | Válida, com domínio e convenções explícitos | Manter como restrição física, não como preditor isolado |
| Todo sistema possui uma trajetória entrópica mensurável | Forte demais | Substituir por hipótese testável para um sistema específico |
| `ΔS ≥ 0` funciona como indicador direto de ruptura | Incorreta como regra universal | Exigir modelo de estado, fronteira e validação experimental |
| `dσ/dt = -k[σ-σcrit]` prevê ruptura | Insuficiente | Definir σ, `k`, `σcrit`, unidades, condições iniciais e critério de falha |
| Calor, vibração e ruído podem ser capturados como energia ou dados | Parcialmente válida | Separar recuperação de energia, medição e dissipação entrópica |
| O sistema pode intervir antes da falha | Condicional | Somente com margem, confiança calibrada e autoridade definida |
| Aprendizado atualiza parâmetros | Possível, mas arriscado | Permitir apenas adaptação auditada, com rollback e sem alterar invariantes |

## Correções técnicas necessárias

### Entropia não é sinônimo de desgaste

A entropia termodinâmica não deve ser usada como sinônimo de ruído, deterioração, envelhecimento, complexidade de software ou risco de falha. Em engenharia, um indicador de saúde pode correlacionar-se com degradação sem ser a entropia termodinâmica. O HERUS deve nomear o indicador pelo que ele mede: temperatura, vibração, impedância, erro de sensor, deriva, ciclos de carga, margem de energia ou outro sinal definido.

### A segunda lei não fornece monotonicidade simples de qualquer sinal

A afirmação operacional correta é mais limitada: a entropia total do sistema isolado apropriado não diminui em um processo irreversível. Um subsistema aberto pode trocar energia e matéria com o ambiente; uma variável observada pode subir, descer ou oscilar. Portanto, o HERUS não deve inferir “ruptura iminente” apenas de uma série crescente chamada entropia.

### A equação de tensão precisa de semântica

A equação apresentada, `dσ/dt = -k[σ-σcrit]`, tem solução que converge para `σcrit` sob `k > 0`; ela não define por si só fadiga, dano acumulado, falha ou tempo até ruptura. Para virar um modelo verificável, será necessário especificar estado, unidade, parâmetros identificáveis, ruído de medição, entradas externas, condição de falha e margem de segurança. Sem isso, a equação é uma ilustração, não uma previsão.

### “Captura de entropia” deve ser dividida em três conceitos

O HERUS deve rejeitar a expressão como primitiva sem definição. Há pelo menos três fenômenos diferentes: **recuperação de energia residual**, como calor convertido em trabalho dentro de limites de eficiência; **aquisição de dados**, como sensores que transformam vibração em informação; e **redução de risco**, como uma intervenção que desloca o sistema para uma região segura. Nenhum deles captura entropia como se fosse uma substância armazenável.

## Tradução para o HERUS

A vertente adequada é um **verificador e sintetizador de políticas de degradação**, não um oráculo termodinâmico. O modelo deve ser finito e explícito:

```text
observações → estado de saúde → transições permitidas → política segura
             ↓                 ↓                  ↓
        incerteza         invariantes          confirmação humana
```

O núcleo pode verificar propriedades como “um sensor fora da faixa leva ao estado seguro em até N passos”, “nenhuma ação excede o orçamento de energia”, “a política não atua sem confirmação” e “a previsão não é promovida quando a validação está fora do domínio observado”. Quando o modelo não sustentar a conclusão, o resultado deve ser `abstain` com contraexemplo ou dados faltantes.

| Elemento | Estado desejado |
|---|---|
| Variável de degradação | Definida por unidade, sensor e faixa válida |
| Modelo | Máquina de estados, equações ou transições com parâmetros rastreáveis |
| Previsão | Intervalo ou classe de risco, nunca certeza textual |
| Síntese | Política limitada que reduz risco dentro do modelo |
| Verificação | Invariantes, limites, overflow, timeout e caminhos de falha |
| Aprendizado | Atualização local de parâmetros com validação, rollback e registro |
| Autoridade | Sem atuação externa automática; confirmação física quando exigida |

## Primeiro experimento recomendado

O experimento inicial deve usar um sistema finito e seguro: uma máquina de estados de saúde para um módulo off-grid. Os estados podem ser `NOMINAL`, `DEGRADING`, `MARGIN_LOW`, `SAFE_HOLD` e `UNKNOWN`. As entradas são leituras quantizadas de temperatura, tensão, vibração, erro de comunicação e contagem de ciclos. A política deve ser sintetizada dentro de uma gramática restrita e verificada contra invariantes.

A previsão não será “a falha ocorrerá em T”. Será uma afirmação condicionada: “dado este modelo, este histórico, este domínio de validade e este orçamento, o caminho permitido alcança `SAFE_HOLD` antes de violar o limite”. A estimativa de tempo até falha pode ser adicionada depois, com dados de prognóstico reais e separação temporal entre treino, validação e teste. A NASA mantém repositórios públicos adequados para investigar essa classe de problema, mas também declara que os dados são usados por conta e risco dos usuários e não substituem qualificação do sistema [4].

## Limites de segurança

O HERUS poderá gerar evidência de verificação para um modelo e uma implementação delimitados. Isso não equivale a certificar um sistema crítico completo. Certificação exige requisitos rastreáveis, revisão independente, ensaios, hardware qualificado, análise de segurança, controle de configuração e responsabilidade institucional.

Nenhum componente desta vertente deve comandar atuadores, alterar parâmetros de segurança em produção, suprimir alarmes, assinar certificados ou promover um modelo aprendido sem revisão humana. O comportamento padrão diante de dados ausentes, sensores conflitantes, deriva não modelada, extrapolação ou prova incompleta é **abstenção e estado seguro**.

## Referências

[1]: https://www.nist.gov/publications/remarks-irreversible-processes-and-entropy-increase "NIST — Remarks on Irreversible Processes and Entropy Increase"
[2]: https://www1.grc.nasa.gov/beginners-guide-to-aeronautics/first-law-conservation-of-energy/ "NASA Glenn — First Law: Conservation of Energy"
[3]: https://www1.grc.nasa.gov/beginners-guide-to-aeronautics/second-law-entropy/ "NASA Glenn — Second Law: Entropy"
[4]: https://www.nasa.gov/intelligent-systems-division/discovery-and-systems-health/pcoe/pcoe-data-set-repository/ "NASA — Prognostics Center of Excellence Data Set Repository"
