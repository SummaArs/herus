# Skill Layer v1 — resultado e limites

## Resultado

A primeira camada de Skills do HERUS foi implementada no laboratório host-only. O sintetizador enumerativo percorreu uma gramática RPN aritmética finita com comprimento máximo de sete instruções. Foram examinados 6.713 candidatos: 6.712 foram rejeitados e um foi aceito após verificação independente.

A habilidade aceita implementa a transformação `2x + 3` com o programa `INPUT → CONST:2 → MUL → CONST:3 → ADD`. Ela passou por três casos visíveis e três casos ocultos, incluindo entradas negativas e valores fora do conjunto de síntese. O resultado foi registrado como `VERIFIED`, com hash de evidência vinculado ao conteúdo da habilidade, custo declarado de cinco passos e nenhum efeito permitido.

O ciclo de confiança observado foi:

```text
CANDIDATE → QUARANTINED → TESTED → VERIFIED
```

Nenhuma transição concede `AUTHORIZED` ou `ACTIVE`. Esses estados não podem ser produzidos pelo sintetizador ou pelo verificador.

## O que foi demonstrado

O experimento demonstra que um núcleo host-only pode propor procedimentos em um espaço fechado de programas, rejeitar candidatos por semântica ou orçamento, verificar um candidato contra casos ocultos e armazenar o resultado como uma habilidade versionada sem conceder autoridade de execução.

O teste de generalização não prova generalização ampla. Ele prova apenas que o candidato aceito não foi rejeitado pelos seis exemplos definidos e que passou por três exemplos ocultos escolhidos para a fixture desta versão.

## O que não foi demonstrado

Este resultado não demonstra raciocínio aberto, compreensão de linguagem natural, aprendizagem estatística, aquisição autônoma de objetivos, segurança contra todos os verificadores defeituosos, execução em microcontrolador, controle de atuadores, comunicação física ou substituição de modelos de linguagem.

A biblioteca atual é experimental e local. Ainda faltam persistência append-only com migração formal, uma segunda implementação independente do verificador, contraexemplos gerados por um oráculo separado, análise de custo em hardware e integração com HIR/Babel sem criar uma ponte de autoridade.

## Comandos de reprodução

A partir de `research/`:

```bash
make generative-lab-skills
make test
```

O teste específico pode ser executado com:

```bash
PYTHONPATH=research python3 -m unittest -v research.test_skills
```

O artefato numérico desta rodada está em [`benchmark.json`](benchmark.json).
