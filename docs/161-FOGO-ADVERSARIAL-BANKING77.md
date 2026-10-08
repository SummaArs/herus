# Fogo adversarial — Banking77 real

## Veredito

**FAIL de robustez adversarial; PASS de reprodutibilidade do experimento.**

O HERUS foi executado no test oficial real do Banking77, com 3.080 exemplos, usando 6.966 exemplos para ajuste e 3.037 para calibração. O test não foi usado para escolher a política.

As perturbações são determinísticas e textuais. Sem revisão humana cega, elas são **stress tests**, não ataques certificados como label-preserving.

| Ataque | Acurácia limpa | Acurácia atacada | Cobertura | Mudança de previsão | Wrong-label flip |
|---|---:|---:|---:|---:|---:|
| Normalização de caixa/pontuação | 75,16% | 74,19% | 100% | 5,42% | 2,85% |
| Deleção de caractere | 75,16% | 74,64% | 100% | 11,62% | 5,40% |
| Prefixo irrelevante | 75,16% | **62,73%** | 100% | **26,62%** | **19,40%** |
| Typo determinístico | 75,16% | **68,18%** | 100% | 16,04% | **12,48%** |

## Interpretação

O resultado mais importante não é a queda de acurácia isolada. É a combinação:

> **A entrada fica degradada, mas o HERUS continua aceitando 100% das decisões.**

Isso significa que a política seletiva atual não detecta incerteza suficiente nesses ataques. Ela opera como um classificador total, não como um sistema que sabe quando deve abster-se.

O limiar de segurança proposto pela auditoria é um limite superior unilateral de 95% para wrong-label flip menor ou igual a 1%. Os quatro ataques excedem esse limite pontual; contudo, a certificação estatística final ainda exige bootstrap pareado e adjudicação semântica humana. O experimento atual não deve ser vendido como certificação formal.

## O que este resultado invalida

- robustez adversarial do HERUS;
- claim de fail-closed sob texto degradado;
- claim de que a cobertura seletiva atual representa segurança operacional;
- claim de SOTA geral;
- claim de adaptação segura a entradas desconhecidas.

## Próximo experimento obrigatório

1. adicionar um detector de shift/abstenção que use somente treino/calibração;
2. calibrar o detector sem observar o test atacado;
3. executar uma segunda bateria com revisão humana cega para separar ataques label-preserving de transformações semanticamente ambíguas;
4. medir se o detector reduz wrong-label flip sem cair em abstention total;
5. repetir em CLINC150 OOS e em pelo menos um domínio independente, depois de fixar versão, licença e checksum.

O resultado permanece versionado mesmo sendo negativo. A bateria completa está em `research/evidence/adversarial_banking77_v1.json`.
