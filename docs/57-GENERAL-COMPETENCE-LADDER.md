# HERUS — Escada de competência geral

A expressão “aproximar-se da AGI” só será útil se for convertida em propriedades observáveis. O HERUS não receberá esse rótulo por produzir respostas convincentes, por passar em um único benchmark ou por compor Skills dentro de um DSL pequeno.

| Nível | Propriedade exigida | Evidência mínima |
|---|---|---|
| G0 | Execução verificável | Uma Skill tipada passa casos visíveis e ocultos |
| G1 | Reutilização | Uma Skill verificada é recuperada em outro problema compatível |
| G2 | Composição | Skills existentes formam uma capacidade nova com prova de tipos e limites |
| G3 | Adaptação | O sistema muda representação e repertório diante de restrições do hospedeiro |
| G4 | Transferência | A mesma abstração funciona em outro domínio ou hospedeiro sem copiar a solução |
| G5 | Aprendizado contínuo | Experiências alteram o repertório com proveniência, sem corromper invariantes |
| G6 | Descoberta de lacunas | O sistema identifica o que não sabe e formula uma busca ou experimento verificável |
| G7 | Competência aberta controlada | Resolve tarefas novas com linguagem, planejamento e consequências limitadas, mantendo abstinência quando necessário |
| G8 | Competência geral demonstrada | Evidência independente de transferência ampla, robustez, eficiência e segurança em ambientes diferentes |

## O que não conta como prova

Uma resposta textual plausível não prova entendimento. Uma previsão correta não prova causalidade. Uma Skill que funciona em seus próprios casos de síntese não prova generalização. Uma execução em simulador não prova segurança física. Uma demonstração com autoridade pré-configurada não prova autonomia segura. Um modelo maior ou um número maior de testes também não prova competência geral se o conjunto puder ser memorizado.

## Separações obrigatórias

Competência não é autoridade. Generalização não é consciência. Autonomia não é ausência de supervisão. Aprendizado não pode alterar silenciosamente a política de segurança. Um componente linguístico pode traduzir linguagem para HIR, mas não pode conceder efeitos. Um hospedeiro pode oferecer recursos, mas não pode declarar sozinho que uma ação é autorizada.

## Critério de declaração

O HERUS só poderá dizer que alcançou um nível quando houver artefato versionado, conjunto de testes ocultos, contraexemplos, custo medido, proveniência, resultados negativos e uma implementação ou ambiente independente do gerador original. Se uma propriedade depender do hardware, ela permanece pendente até ser medida no hardware.

A meta “aproximar-se da AGI” é, portanto, um programa de pesquisa, não uma alegação atual. O resultado mais importante será localizar a fronteira: demonstrar quais formas de competência podem ser construídas por síntese, adaptação e memória verificáveis, e identificar exatamente onde a arquitetura ainda falha em aberto.
