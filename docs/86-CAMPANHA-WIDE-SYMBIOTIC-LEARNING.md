# Campanha ampla do Symbiotic Learning

**Status:** evidência host-only sintética, reproduzível; não é validação externa nem prova de produto.

## Desenho

A campanha gera 100 casos determinísticos:

| Categoria | Quantidade | Propriedade |
|---|---:|---|
| `safe` | 20 | efeito único, contexto compatível |
| `ambiguous` | 20 | duas ações com o mesmo efeito |
| `context_mismatch` | 20 | efeito correto fora do contexto |
| `stale` | 20 | evidência antiga além da janela temporal |
| `risky` | 20 | risco acima do limite |

Todos os métodos recebem os mesmos candidatos.

## Métodos comparados

- `fixed_name`: usa a primeira ação sem aprender efeito;
- `effect_only`: procura delta igual, ignorando contexto, deriva e risco;
- `meta_action`: Symbiotic Learning v2 com histórico meta-simbiótico, contexto, risco e deriva.

## Resultado reproduzido

| Método | Acerto seguro | Falso aceite inseguro |
|---|---:|---:|
| `fixed_name` | 0/20 (0%) | 80/80 (100%) |
| `effect_only` | 20/20 (100%) | 60/80 (75%) |
| `meta_action` | 20/20 (100%) | 0/80 (0%) |

O resultado mostra uma vantagem de segurança neste fixture: a correspondência simples por efeito acerta os positivos, mas aceita efeitos fora de contexto, obsoletos ou arriscados. O meta-simbionte preserva o acerto e abstém-se nos negativos.

## Interpretação correta

Isto é uma **regressão ampla de mecanismo**, não uma prova de state of the art. Os casos foram gerados pelo próprio projeto, são finitos e não representam hospedeiros físicos, usuários ou distribuições externas. O resultado pode ser um artefato do contrato implementado.

A campanha aumenta a confiança de que o algoritmo respeita as invariantes codificadas. Ela não prova que o algoritmo generaliza para uma distribuição desconhecida.

## Reprodução

```bash
PYTHONPATH=research python3 -m wide_symbiotic_benchmark
PYTHONPATH=research python3 -m unittest research.test_wide_symbiotic_benchmark -v
```

Evidência bruta: [`research/evidence/wide_symbiotic_learning_v1.json`](../research/evidence/wide_symbiotic_learning_v1.json).

## Próximo holdout

A próxima versão deve congelar os geradores antes de executar, ocultar os casos negativos do método, adicionar hospedeiros black-box e comparar custo/latência. Sem isso, a classificação permanece **mechanism evidence**, não generalização.
