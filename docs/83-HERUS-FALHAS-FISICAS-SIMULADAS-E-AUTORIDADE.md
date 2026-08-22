# HERUS — Falhas físicas simuladas e autoridade

**Estado:** host-side integrado e publicado; nenhuma propriedade elétrica ou humana é reivindicada.  
**Objetivo:** verificar que perda de adaptadores e interrupções não promovem estados parciais a memória, oferta confirmada ou ação autorizada.

## 1. Fronteiras simuladas

O adaptador pessoal agora aceita falhas explícitas de relógio, semântica, haptic, contato e energia. Cada falha tem um significado diferente e não pode ser comprimida em um simples “erro”: perder o relógio impede avanço temporal; perder o haptic retém uma oportunidade; perder contato impede confirmação; perder energia durante contato preserva a oferta sem confirmar; perder energia global faz scrub; perder o adaptador semântico impede a criação de um novo candidato.

| Falha | Resposta esperada | Autoridade criada |
|---|---|---|
| Relógio ausente | Rejeita evento sem avançar geração | Nenhuma |
| Haptic ausente | Mantém candidato em `HOLD` | Nenhuma |
| Contato ausente | Mantém `OFFER` não confirmado | Nenhuma |
| Energia insuficiente no contato | Mantém `OFFER` e não incrementa acknowledge | Nenhuma |
| Energia perdida | Faz power-cycle e apaga candidato | Nenhuma |
| Semântica indisponível | Não observa nem cria candidato | Nenhuma |
| Adaptador restaurado | Permite continuar apenas com estado ainda válido | Somente a autoridade já prevista |

## 2. Resultado do cenário

O cenário `physical-faults` executa uma oferta retida, restauração de haptic, perda e restauração de contato, perda de clock, power-loss, restauração de energia e perda do adaptador semântico. O contato restaurado só confirma a oferta se ela ainda estiver viva e se houver energia suficiente.

| Resultado host-side | Veredito |
|---|---:|
| Cenário de falhas físicas | **14/14** |
| Vida semântica contínua | **16/16** |
| Presença pessoal | **13/13** |
| Bancada virtual completa | **154 invariantes** |
| Regressão global | **78 suítes** |
| Mutação global | **7/7 mutantes mortos** |

## 3. Invariante central

A fronteira de hardware não pode ser tratada como um canal confiável que eventualmente “entrega” o evento. Cada adaptador tem de fornecer uma condição verificável. Se essa condição não está disponível, o HERUS não deve adivinhar o evento, completar a transação com estado antigo ou transformar a ausência em confirmação.

A consequência é importante para a arquitetura futura: o DRV2605L, o botão, o PMIC, o relógio e qualquer adaptador de percepção não serão autorizadores independentes. Eles fornecerão evidência tipada para um núcleo que continua responsável por validade, ordem, expiração, contato e autoridade.

> Interrupção não é uma versão lenta do sucesso. É um estado próprio, que precisa terminar em espera, scrub, expiração ou rejeição.

## 4. Limites honestos

Este resultado ainda não mede tempo elétrico, energia real, brownout, debounce, contato humano, vibração percebida ou comportamento do usuário durante interrupções. O `uJ` no cenário é um orçamento determinístico, não consumo do ESP32-S3. A geração é um relógio lógico, não oscilador físico. A disponibilidade de haptic é um evento, não prova de que a pessoa sentiu ou entendeu o sinal.

O que foi demonstrado é mais restrito e útil: **dadas as falhas tipadas, o sistema não fabrica autoridade para preencher a lacuna**. A bancada deverá testar se os adaptadores reais conseguem produzir os eventos sem falsos positivos, perdas silenciosas ou temporização fora do contrato.

## Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
(cd sim && make -s physical-faults)
./prove.sh --quiet
```
