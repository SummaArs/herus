# HERUS — Simulador pessoal e fronteira de hardware

**Estado:** host-side integrado e publicado; nenhum resultado físico é reivindicado.  
**Objetivo:** substituir, antes da bancada, o máximo possível dos componentes comportamentais do relógio: relógio monotônico, energia disponível, disponibilidade de haptic, contato físico, perda de energia e sequência de eventos semânticos.

## 1. Arquitetura de substituição

O simulador não duplica o firmware. O núcleo exercitado é o `firmware/core/ambient_presence.{h,c}` real. O diretório `sim/` fornece somente adaptadores de mundo: eventos tipados, energia, estado de alimentação e um traço de execução. Quando o hardware existir, esses adaptadores deverão ser trocados por fontes reais de geração monotônica, DRV2605L, botão/gesto, gerenciamento de energia e sensores autorizados.

| Fronteira física futura | Substituto atual | O que a substituição pode testar |
|---|---|---|
| Relógio monotônico | `event.generation` determinística | Ordenação, expiração, cooldown e rejeição de regressão |
| DRV2605L/haptic | `haptic_available` e custo energético | Oferta retida, oferta única, latência lógica e ausência de saída |
| Contato humano | `physical_contact` explícito | Reconhecimento separado de autorização |
| Bateria/PMIC | orçamento em `uJ` | Supressão por energia insuficiente e ausência de iniciativa falsa |
| Brownout/power-cycle | `pps_power_cycle` e evento `power_on=0` | Scrub de estado transitório e não-revivificação |
| Adaptador semântico | `ap_observation_t` | Confiança, relevância, novidade, risco, privacidade e validade |

O traço de execução é deliberadamente pequeno. Ele não contém áudio, texto, identidade, localização, embedding ou conteúdo de terceiro. A latência registrada é diferença entre gerações do cenário; não é latência física do produto.

## 2. Cenário integrado

O cenário `personal` percorre uma sequência determinística que representa uma vida curta ao redor de uma oportunidade semântica. Primeiro, uma observação útil é retida quando o haptic está indisponível. Depois, a disponibilidade aparece e uma única oferta é produzida. Consultas repetidas não repetem a oferta nem destroem a janela de contato. Ausência de contato mantém a oferta não confirmada; contato físico reconhece somente o recebimento.

A sequência continua com nova oportunidade, power-cycle, expiração sem apresentação tardia, energia insuficiente, relógio regressivo e desligamento explícito. O cenário não mede se a observação seria correta para uma pessoa: ele mede se a política preserva os limites quando recebe uma observação já tipada.

| Resultado host-side | Veredito |
|---|---:|
| Cenário pessoal | **13/13** |
| Bancada virtual total | **124 invariantes** |
| Redteam global de contratos existentes | **7/7 mutantes mortos** |
| Regressão global | **78 suítes** |
| Proveniência | **válida; 1 gate local ativo e 3 pendentes** |

## 3. Descoberta durante a integração

A primeira execução falhou em dois pontos do próprio cenário. O teste tentava confirmar uma oferta depois de sua expiração, e o simulador retornava imediatamente após expirar sem processar uma nova observação no mesmo evento. A sequência foi corrigida para distinguir contato no limite válido de contato tardio e para manter a semântica de expiração explícita.

A falha mais instrutiva ocorreu na mutação global: a campanha existente não incluía os novos arquivos no comando de compilação e, por isso, os mutantes sequer compilavam. Isso não era um mutante morto; era uma lacuna de cobertura do harness. O harness foi corrigido para compilar o binário completo com `ambient_presence.c`, `personal_sim.c` e `personal_scenario.c`. Depois da correção, os sete mutantes existentes voltaram a ser mortos.

> Um mutante que não compila por uma fonte ausente não é uma vitória adversarial. É uma campanha incompleta.

## 4. O que agora pode ser ajustado sem o hardware

A arquitetura já permite experimentar, no host, o comportamento de presença: quando manter uma sugestão latente, quando esperar por haptic, como medir uma janela lógica, quando expirar, como esquecer após power-cycle, como evitar insistência e como separar recebimento de autorização. Também permite inserir sequências mais longas de vida sem depender de sensores físicos.

Isso cobre uma parte importante da inteligência pessoal, mas não a percepção bruta. O simulador recebe confiança, relevância, novidade e risco como entradas; ele não demonstra que um microfone, VAD, reconhecimento de voz ou sensor produzirá esses valores corretamente. Essa interface é intencional: a camada de percepção futura terá de provar seus próprios erros sem contaminar a política de presença.

## 5. Limites restantes

Ainda dependem de hardware e de pessoas a energia real do ESP32-S3, a durabilidade de flash/NVS sob brownout, o timing do DRV2605L, a inteligibilidade de padrões hápticos, a latência elétrica, o conforto de uso, a compreensão de contato e a qualidade de qualquer percepção de voz ou sensor. Também não há resultado de WER, acurácia, autonomia de bateria, alcance, consumo físico ou aceitação humana.

O ganho real desta etapa é arquitetural: quando a bancada existir, os primeiros testes não precisarão decidir o que significa “ser sutil” ou “não criar autoridade”. Esses contratos já têm uma implementação e uma sequência de falhas reproduzível. A bancada deverá verificar se os adaptadores físicos conseguem fornecer os eventos e se pessoas consideram o comportamento útil, compreensível e digno de confiança.

## Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
(cd sim && make -s personal)
./prove.sh --quiet
```
