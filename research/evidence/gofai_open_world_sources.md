# Fontes para o critério de superação do GOFAI

## Fonte principal

Lorang et al., *Breaking Task Impasses Quickly: Adaptive Neuro-Symbolic Learning for Open-World Robotics*, arXiv:2601.16985v1, 2026. URL: https://arxiv.org/html/2601.16985v1

A fonte descreve três limitações relevantes para o benchmark do HERUS: o planejamento simbólico oferece raciocínio estruturado, mas tem dificuldade de adaptação a variações imprevisíveis; abordagens híbridas enfrentam ineficiência de amostras, adaptação lenta e falhas de recuperação em ambientes dinâmicos; e a ligação entre representação simbólica e aprendizagem é um desafio central para exploração e generalização.

O artigo avalia adaptação por novidades, aprendizagem de Skills/controles e transferência entre embodied contexts. Esses pontos são úteis para o HERUS, mas não devem ser copiados como prova de superação: o ASA precisa acrescentar descoberta do hospedeiro, orçamento de autoridade, abstinência verificável, proveniência e separação entre proposta e execução.

## Consequência para o benchmark HERUS

A superação não será declarada por usar mais camadas ou por produzir uma sequência simbólica. O HERUS precisará vencer um baseline GOFAI equivalente em pelo menos quatro dimensões mensuráveis: adaptação a uma novidade não presente na síntese, transferência para um hospedeiro não previsto, recuperação após falha sem reprogramação manual e abstinência correta quando evidência ou autoridade forem insuficientes.

A comparação deve registrar custo de sondagem, tempo até uma Skill válida, taxa de falsos efeitos, cobertura de casos ocultos, taxa de recusa correta e preservação da autoridade. O hardware físico continua obrigatório para afirmar propriedades de latência, energia, rádio, memória e interação humana.
