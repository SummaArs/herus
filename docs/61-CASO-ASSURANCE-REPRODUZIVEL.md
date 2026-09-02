# Caso de assurance reproduzível

O HERUS recebeu uma fixture declarativa em `research/evidence/reference_assurance_case.json` e um runner independente em `research/run_assurance_case.py`. O objetivo é tornar a demonstração da cadeia de assurance executável sem depender da construção de objetos dentro dos testes.

## Resultado de referência

```json
{
  "verdict": "ASSURED",
  "reason": "finite_chain_assured",
  "abstract_verification": "VERIFIED",
  "concrete_verification": "VERIFIED",
  "machine_refinement": "REFINED",
  "policy_refinement": "REFINED",
  "call_path": "COVERED"
}
```

O resultado foi gerado pelo alvo `make -C research assurance-case`. O certificado contém digest canônico de modelos, políticas, mapas e caminhos críticos. Esse digest identifica o conteúdo lógico da evidência da rodada; não constitui assinatura, atestação de compilador ou prova sobre o mundo físico.

## Mutações

A mutação do caminho crítico de `COVERED` para `UNCOVERED` produz `BLOCKED`. A mutação da política concreta, quando mantida executável, produz `COUNTEREXAMPLE`. Um schema inválido é rejeitado antes da composição. Assim, o caso demonstra três propriedades necessárias: sucesso composto, bloqueio por ausência de caminho e falha por divergência semântica.

## Limites

O caso é finito e propositalmente pequeno. Ele não representa um ativo operacional real, não substitui integração com sensores ou atuadores e não certifica o firmware inteiro. Seu valor é reprodutibilidade: a cadeia completa agora pode ser revisada por uma entrada declarativa, um comando, um resultado serializado e mutações adversariais previsíveis.
