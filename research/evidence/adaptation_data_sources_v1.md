# Fontes de dados reais para adaptação multi-domínio

## Robótica

O NIST descreve os Assembly Task Boards como ferramentas de benchmark para tarefas de montagem robótica, incluindo inserção de pinos, rosqueamento de porcas e montagem de conectores. A fonte é adequada para definir tarefas verificáveis de manipulação e contratos de observação, mas não substitui dados de um robô específico nem demonstra transferência automática para qualquer plataforma.

Fonte: [NIST — Benchmarks and Datasets to Tackle Real-World Robotic](https://www.nist.gov/el/intelligent-systems-division-73500/benchmarks-and-datasets-tackle-real-world-robotic).

## Finanças

O Office of Financial Research oferece uma API oficial para séries de fundos de mercado monetário, mercados de repo, taxas de referência, estatísticas de primary dealers e Treasury constant maturity rates. A API permite selecionar dataset, vintage, intervalo, periodicidade, agregação e formato temporal. É uma fonte adequada para tarefas de observação, previsão limitada, detecção de mudança e simulação; não autoriza ordens ou movimentação financeira real.

Fonte: [OFR — API Full Data Set](https://www.financialresearch.gov/short-term-funding-monitor/api-specs/api-full-dataset/).

## Infraestrutura crítica

A CISA apresenta o HIFLD como um repositório de dados geoespaciais de infraestrutura crítica, cobrindo setores como comunicações, energia, finanças, transporte e água. A própria fonte alerta que os dados podem conter erros, são atualizados pelos provedores e precisam ser validados quanto à precisão e adequação antes de uso em decisões que afetem segurança humana ou resultados de missão. Isso é diretamente compatível com o princípio do HERUS de que dado real não equivale automaticamente a evidência suficiente.

Fonte: [CISA — Mapping Your Infrastructure: Datasets for Infrastructure Identification](https://www.cisa.gov/resources-tools/resources/mapping-your-infrastructure-datasets-infrastructure-identification).

## Decisão inicial

O laboratório deve começar com três tarefas de baixo efeito: uma tarefa NIST de montagem em ambiente de simulação, uma série OFR para detecção de regime ou anomalia sem decisão financeira, e um conjunto CISA/HIFLD para classificação de qualidade/proveniência de infraestrutura sem ação operacional. Cada tarefa precisa registrar fonte, licença ou condição de acesso, versão, intervalo temporal, transformação, digest, divisão temporal e limites da alegação.

Nenhuma dessas fontes prova adaptação universal. Elas fornecem ambientes e observações reais para testar se o mesmo núcleo consegue distinguir recurso, incerteza, incompatibilidade e necessidade de abstenção em domínios diferentes.
