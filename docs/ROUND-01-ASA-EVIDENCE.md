# Rodada 1 — Evidência inicial para o ASA

## Conclusão

A primeira rodada produziu evidência positiva, porém estreita, para uma propriedade específica do **Adaptive Symbiotic Architecture (ASA)**: uma proposta tipada pode ser transferida entre dois hospedeiros sintéticos com nomes de ações diferentes quando a transferência usa contratos de efeito observados, e não nomes previamente conhecidos.

O experimento usou 13 casos do corpus local `research/evidence/semantic_ir_real_corpus.json`. Esse corpus é uma coleção versionada de regressão de linguagem e contrato; não é telemetria de produção. Os hospedeiros foram sintéticos e deliberadamente ocultaram a correspondência entre nomes de ações.

O ASA obteve equivalência semântica em **13 de 13 casos**. O baseline que não possui transferência por contrato obteve **8 de 13**, porque só consegue reproduzir os casos que exigem abstention. Nos oito casos ambíguos, desconhecidos ou inválidos, o ASA se absteve corretamente. Não houve decisão não segura. O experimento não prova inteligência geral, compreensão aberta de linguagem ou desempenho físico.

## Motivação científica

O desenho combina três ideias externas com os contratos existentes do HERUS. O Jev, da TypeSafe AI, propõe um decisor que recebe estado e produz valores tipados acompanhados de probabilidades, em vez de gerar texto livre. Essa é uma alegação técnica da própria empresa e não deve ser tratada como resultado acadêmico independente [1].

O SHAPER formula a adaptação de agentes incorporados como evolução de Skills e do harness externo, mantendo os pesos do modelo congelados [2]. O OpenSkill acrescenta a necessidade de construir sinais de verificação independentes e evitar vazamento de respostas-alvo durante a evolução [3]. O benchmark Embodied Agent Interface reforça que avaliação de agentes deve separar interpretação do objetivo, decomposição, sequência de ações e modelagem de transições [4].

A adaptação escolhida para o HERUS é deliberadamente menor: o parser finito continua responsável pela entrada; o ASA transfere apenas a relação entre uma intenção tipada e um efeito observável do hospedeiro; a saída permanece uma proposta sem autoridade de execução.

## Hipótese falsificável

> **H1:** Dado um vocabulário finito de intenções, dois hospedeiros com ações primitivas de nomes e ordem diferentes, e evidência observável suficiente, a transferência por contrato de efeito do ASA preserva a proposta semântica e mantém abstention nos casos que não podem ser determinados.

A hipótese falha se qualquer uma das seguintes condições ocorrer: o ASA escolher uma ação incorreta em um hospedeiro equivalente; deixar de se abster diante de conflito, negação ou entrada fora do domínio; depender dos nomes das ações; ou produzir uma decisão externa em vez de uma proposta tipada.

## Protocolo

O hospedeiro de origem expõe três ações com nomes artificiais: `a1`, `a2` e `a3`. O hospedeiro-alvo expõe as mesmas affordances semânticas com nomes diferentes e em ordem diferente: `route_delta`, `haptic_blue` e `abort_local`. O ASA observa os efeitos, sintetiza Skills limitadas, verifica-as e monta o mapeamento no hospedeiro-alvo por assinatura de efeito.

A entrada de linguagem é processada por um adaptador finito baseado no texto ou no comando tipado recebido. O adaptador não consulta o campo `expected` do corpus durante a inferência. Esse campo serve somente à avaliação posterior.

A avaliação mede quatro propriedades. **Equivalência semântica** verifica se o evento proposto coincide com o evento esperado. **Abstention segura** verifica se casos sem evento esperado não recebem evento. **Decisão não segura** conta casos em que o ASA produz evento quando deveria abster-se. **Equivalência contratual exata** verifica também o status textual, permitindo distinguir uma rejeição textual específica de um `ABSTAIN` semanticamente seguro.

## Resultado

| Sistema | Casos | Equivalência semântica | Abstention segura | Decisões não seguras |
|---|---:|---:|---:|---:|
| ASA por contrato de efeito | 13 | 13/13 | 8/8 | 0 |
| Baseline sem transferência | 13 | 8/13 | 8/8 | 0 |

O ASA teve 13/13 de equivalência semântica porque descobriu o contrato dos efeitos no hospedeiro-alvo. O baseline teve 8/13 porque corretamente se absteve dos cinco casos sem proposta, mas não conseguiu transferir os três eventos válidos para as ações de nomes novos nem reconhecer o comando tipado válido.

A equivalência contratual exata do ASA foi 5/13. Essa métrica menor é esperada: o ASA usa `ABSTAIN` como resultado seguro para entradas inválidas, enquanto o corpus diferencia `UNKNOWN` e `REJECTED`. A diferença é registrada como dívida de contrato, não ocultada. Uma rodada posterior deve definir uma taxonomia comum de abstention antes de comparar sistemas.

## Relação com dados reais

Os 13 casos são provenientes de um artefato local versionado e rastreável que contém texto, comandos tipados e expectativas de contrato. Eles representam dados reais de linguagem e especificação de interface, mas não representam distribuição de uso de pessoas reais. A evidência desta rodada é, portanto, **real-local + sintética**, não uma avaliação de campo.

## Limitações

O adaptador de linguagem possui vocabulário fechado e foi construído para o contrato atual de `ARRIVE`, `HELP` e `CANCEL`. Ele não demonstra NLU aberta. Os hospedeiros têm estados discretos e efeitos escalares. Não há ruído de sensor, observabilidade parcial, custo energético, latência física ou acionamento de atuadores. O conjunto real é pequeno e não permite inferência estatística sobre uma população.

A transferência por efeito também depende de que os efeitos observados sejam suficientemente distintos. Efeitos colidentes, estados ocultos ou mudanças dinâmicas do hospedeiro ainda precisam ser avaliados. A rodada seguinte deve introduzir incerteza, conflitos e famílias maiores de hospedeiros, além de comparar uma política de efeito exato com uma política probabilística calibrada.

## Reprodução

```bash
PYTHONPATH=. python3 -m research.asa_round1
PYTHONPATH=. python3 -m unittest research.test_asa_round1
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
```

A nova suíte local contém **120 testes**, com um teste previamente ignorado no conjunto original. O benchmark da rodada 1 não concede autoridade externa e não executa ações fora dos hospedeiros sintéticos.

## Referências

[1]: https://typesafe.ai/blog/introducing-system-one-models-and-jev "Introducing System One Models & Jev — TypeSafe AI"
[2]: https://arxiv.org/html/2608.11350v1 "Self-Evolving Embodied Agents via Skill-Harness Evolution"
[3]: https://arxiv.org/html/2606.06741v1 "OpenSkill: Open-World Self-Evolution for LLM Agents"
[4]: https://proceedings.neurips.cc/paper_files/paper/2024/file/b631da756d1573c24c9ba9c702fde5a9-Paper-Datasets_and_Benchmarks_Track.pdf "Benchmarking LLMs for Embodied Decision Making"
