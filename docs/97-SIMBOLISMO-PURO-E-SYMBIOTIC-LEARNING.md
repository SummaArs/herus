# Simbolismo puro e Symbiotic Learning

## Resposta direta

**100% simbólico é possível para o núcleo constitucional do HERUS.** Não é uma aposta segura para todo o sistema de percepção, representação e generalização aberta.

Essas duas afirmações não são contraditórias.

O núcleo pode ser totalmente simbólico porque seu domínio é finito e deliberado: autoridade, tipos, contratos, estados, leases, evidência, abstenção, rollback e execução autorizada. Nessa camada, previsibilidade e verificabilidade valem mais que flexibilidade.

A camada que interpreta linguagem aberta, sinais ruidosos, novos hospedeiros e regularidades não previstas enfrenta outro problema. Ela precisa adquirir representações. A literatura recente não mostra que um sistema puramente simbólico consiga fazer isso de modo competitivo em domínio aberto sem algum mecanismo de aprendizado ou uma ontologia previamente construída.

## O que os papers mudam

A revisão de Colelough e Regli mostra que a pesquisa neuro-simbólica se concentra em aprendizado/inferência, lógica/raciocínio e representação, enquanto confiança, explicabilidade e metacognição recebem menos atenção [1]. Isso valida a direção do HERUS, mas não autoriza dizer que Symbiotic Learning já é um novo campo estabelecido.

O rsbench, de Bortolotti et al., mostra que modelos neurais e neuro-simbólicos podem alcançar bons resultados usando atalhos e sem aprender os conceitos corretos [2]. Portanto, acurácia não basta. O HERUS precisa medir se a representação aprendida corresponde ao contexto e se continua válida sob mudança de hospedeiro.

LaSR, de Grayeli et al., combina evolução simbólica com uma biblioteca de conceitos induzida por LLM e relata ganhos em regressão simbólica [3]. O ponto importante não é copiar LLMs. É reconhecer que aprendizado pode propor abstrações e que o simbólico pode verificar, compor e selecionar hipóteses. Isolar totalmente os dois lados provavelmente reduz a capacidade de descoberta.

A literatura de benchmarking neuro-simbólico também separa famílias model-theoretic, proof-theoretic fuzzy e proof-theoretic probabilistic [4]. Isso mostra que “ser simbólico” não é uma métrica única. O HERUS precisa declarar qual propriedade está testando em cada experimento.

## Decisão arquitetural

O HERUS não deve perseguir uma pureza única. Deve perseguir uma **separação constitucional**:

```text
PERCEPÇÃO E REPRESENTAÇÃO
aprendizado permitido, limitado e mensurado
              ↓
HIR / CARTÕES DE CONTEXTO
representação canônica finita
              ↓
NÚCLEO SIMBÓLICO CONSTITUCIONAL
100% simbólico, verificável e fail-closed
              ↓
PROPOSTA
pode ser aprendida ou sintetizada
              ↓
VERIFICADOR INDEPENDENTE
100% simbólico para autoridade e contrato
              ↓
EXECUÇÃO
somente com autorização explícita
```

A regra central é:

> **Aprendizado pode sugerir significado; somente o núcleo simbólico pode conceder autoridade.**

Assim, uma rede neural não pode transformar sua confiança em permissão. Uma hipótese aprendida precisa ser traduzida para o vocabulário finito, passar por verificação independente, carregar proveniência e poder ser rejeitada.

## Definição mais rigorosa de Symbiotic Learning

Neste estágio, Symbiotic Learning não deve ser anunciado como um novo paradigma de ML já estabelecido. É uma hipótese de pesquisa com quatro propriedades combinadas:

1. o algoritmo aprende uma adaptação condicionada ao hospedeiro;
2. o aprendizado permanece separado da autoridade;
3. experiências são transferidas apenas como evidência verificável;
4. o sistema pode abster-se, reverter e reconstruir contexto quando o hospedeiro muda.

Essa combinação pode se tornar uma contribuição original. A originalidade não está em usar símbolos ou neurônios isoladamente. Está no contrato de adaptação soberana sob restrição de autoridade.

## Como chegar a 100%

Para este projeto, 100% não significa vencer todo ML. Significa cumprir um contrato científico versionado:

- desempenho medido contra baselines reais;
- transferência para pelo menos um domínio novo;
- representação avaliada contra atalhos;
- confiança calibrada;
- custo e memória medidos;
- autoridade nunca derivada de predição;
- falhas reproduzíveis e documentadas;
- biblioteca importável e comportamento determinístico sob seed;
- escopo de generalização explicitamente limitado.

A porcentagem atual permanece **64,5%**. A comparação transformer foi concluída, mas não houve vitória robusta. A literatura aumenta a qualidade da hipótese e do protocolo, não a porcentagem automaticamente.

## Próxima pesquisa implementável

A próxima rodada deve adicionar três testes reais:

1. **Shortcut test:** perturbar palavras superficiais preservando o evento e medir se a decisão permanece estável.
2. **Domain-shift test:** treinar em MIntRec e avaliar em um segundo corpus real com vocabulário e distribuição diferentes.
3. **Authority-separation test:** alterar a predição aprendida sem alterar a evidência e provar que a autoridade não muda.

Só depois disso vale comparar uma arquitetura híbrida contra o transformer em mais de uma seed e mais de um dataset.

## Referências

[1]: https://arxiv.org/html/2501.05435v1 "Neuro-Symbolic AI in 2024: A Systematic Review"
[2]: https://proceedings.neurips.cc/paper_files/paper/2024/hash/d1d11bf8299334d354949ba8738e8301-Abstract-Datasets_and_Benchmarks_Track.html "A Neuro-Symbolic Benchmark Suite for Concept Quality and Reasoning Shortcuts"
[3]: https://proceedings.neurips.cc/paper_files/paper/2024/hash/4ec3ddc465c6d650c9c419fb91f1c00a-Abstract-Conference.html "Symbolic Regression with a Learned Concept Library"
[4]: https://lamarr-institute.org/de/publication/benchmarking-in-neuro-symbolic-ai/ "Benchmarking in Neuro-Symbolic AI"
