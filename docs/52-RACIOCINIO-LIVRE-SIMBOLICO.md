# HERUS — Raciocínio Livre Simbólico v2

**Status:** protótipo de pesquisa host-only. Não altera a autoridade do firmware.

## Tese

O HERUS já possuía composição tipada, saturação de equivalências e um laboratório generativo finito. Esta etapa avança a hipótese em direção a um mecanismo que **gera hipóteses e pequenos programas simbólicos**, em vez de apenas selecionar entre estados pré-definidos.

A tese mensurável é:

> Dado um vocabulário algébrico explícito, o sistema pode inventar expressões novas, descobrir identidades, sintetizar pequenas funções a partir de exemplos e produzir um certificado exato ou uma negativa controlada.

Isto não é uma alegação de inteligência geral. Grounding, objetivos, semântica aberta e interação física continuam sendo problemas separados.

## 1. Kernel algébrico exato

`research/free_reasoner/free_reasoner.py` implementa um fragmento de anel com termos imutáveis, inteiros/racionais exatos, soma, subtração, multiplicação, negação, parser determinístico e normalização polinomial esparsa.

Duas expressões são consideradas algebraicamente equivalentes apenas quando o kernel produz exatamente o mesmo polinômio canônico. Não há limiar de similaridade nem confiança estatística no gate de prova.

## 2. Busca e prova

O `Prover` combina duas camadas:

1. normalização exata, que funciona como núcleo semântico confiável;
2. busca limitada sobre reescritas associativas, comutativas, distributivas e identidades elementares, produzindo uma trilha legível quando encontrada.

Se a busca não encontrar uma trilha curta, uma igualdade ainda pode ser aceita **somente porque o kernel algébrico a provou**. Um timeout de busca nunca vira `TRUE` por aproximação.

## 3. Descoberta de conjecturas

`discover_conjectures()` gera expressões e as agrupa pelo significado algébrico exato. Quando duas árvores sintaticamente diferentes ocupam a mesma classe semântica, surge uma conjectura nova.

Exemplo de forma descoberta:

```text
((a * b) * c) == ((a * c) * b)
```

O sistema não precisou armazenar essa permutação específica como resposta. Ela emerge da combinação da gramática de geração com a semântica comutativa do produto.

## 4. Síntese generativa

`synthesize_linear_or_polynomial()` trata uma função desconhecida como espaço de programas. Ele enumera expressões pequenas usando `+`, `-` e `*`, mede a assinatura exata de cada candidato sobre os exemplos e elimina candidatos semanticamente duplicados.

Para:

```text
-3 -> 9
-2 -> 4
-1 -> 1
 0 -> 0
 1 -> 1
 2 -> 4
 3 -> 9
```

um candidato mínimo possível é:

```text
x*x
```

Esse resultado é **síntese que ajusta os exemplos**, não prova de que a intenção humana original era necessariamente `x²`. A distinção fica explícita na API.

## 5. Arquitetura de pesquisa

```text
                 objetivo / exemplos
                         |
                         v
              gerador simbólico limitado
                         |
              +----------+----------+
              |                     |
              v                     v
        semantic signatures     rewrite search
              |                     |
              +----------+----------+
                         v
                exact algebra kernel
                         |
             +-----------+-----------+
             |                       |
             v                       v
          PROVED                 COUNTEREXAMPLE
             |
             v
       candidate / lemma
```

A direção para o HERUS é integrar esta camada com a Semantic IR existente, sem permitir que um candidato generativo pule os gates de autoridade.

## 6. Por que a abordagem é relevante

E-graphs/equality saturation já são uma técnica estabelecida para representar simultaneamente expressões equivalentes e foram usadas em sistemas de síntese e otimização. citeturn747789search0turn747789academia59

HDC/VSA também é uma área ativa de representação composicional e simbólica distribuída, com forte base algébrica e aplicações cognitivas. citeturn747789search3turn747789academia58

Portanto, a proposta do HERUS não é alegar que descobriu individualmente esses componentes. A aposta experimental está na composição específica:

```text
representação composicional
        +
geração de programas/hipóteses
        +
normalização algébrica exata
        +
contraexemplos
        +
prova recuperável
        +
limites determinísticos
```

## 7. O que ainda falta para “raciocínio livre” forte

O laboratório ainda não resolve:

- grounding do mundo real;
- indução de novos símbolos e novos tipos sem intervenção;
- escolha autônoma de objetivos/valores;
- busca universal eficiente;
- indução estrutural geral e descoberta de invariantes arbitrários;
- causalidade e física do mundo;
- linguagem natural aberta;
- memória episódica multimodal;
- planejamento longo-horizonte.

Esses limites são parte da especificação, não detalhes escondidos.

## 8. Próximo experimento decisivo

O próximo benchmark deve separar combinações vistas de combinações ocultas:

1. entregar axiomas básicos e exemplos parciais;
2. esconder a combinação final;
3. pedir ao sistema para gerar o termo/algoritmo;
4. avaliar em entradas nunca vistas;
5. exigir contraexemplo quando a hipótese estiver errada;
6. medir custo de busca, memória e taxa de abstenção;
7. comparar contra enumerador simples, e-graph tradicional e baseline neural declarado.

O resultado interessante não é apenas acerto. É **generalização combinatória com prova e custo controlado**.

## Veredito científico

Esta etapa move o HERUS de “composição simbólica finita” para um protótipo de **geração simbólica verificável**. É uma direção de pesquisa concreta e implementável.

Ainda não há base para dizer que o problema de raciocínio livre universal foi resolvido. A reivindicação adequada neste estágio é muito mais forte e testável:

> **O HERUS agora possui um substrato experimental em que novas estruturas podem ser geradas e somente então promovidas após verificação simbólica exata.**
