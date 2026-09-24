# HERUS — Roadmap e gates

## Regra de continuidade

O HERUS não deve avançar por acúmulo de módulos. Ele deve avançar somente quando uma incerteza relevante for reduzida. Cada fase possui uma pergunta, uma entrega, um gate e uma condição de parada.

## Direção oficial: simbionte soberano

O objetivo futuro do HERUS é tornar-se um **simbionte computacional autônomo, transferível e soberano**. Ele deverá habitar sistemas autorizados, compreender as capacidades de cada hospedeiro, transferir Skills entre softwares e usar uma superfície física para confirmação, cancelamento e parada.

Soberania significa que dados privados, memória, traces e modelos permanecem no domínio controlado pelo usuário por padrão. Nenhum egress externo deve ocorrer por padrão. Qualquer comunicação indispensável para uma ação autorizada deverá ser mínima, limitada, registrada e revogável. Essa é uma meta arquitetural ainda não demonstrada pelo protótipo atual.

O documento normativo dessa direção é [57 — Direção soberana e simbionte](57-DIRECAO-SOBERANA-E-SIMBIONTE.md).

## Fase A — Reprodutibilidade e leitura

**Pergunta:** outra pessoa consegue entender o estado atual e reproduzir os gates locais?

**Entrega:** documentação organizada, comandos estáveis, links válidos, manifesto consistente e distinção entre histórico e estado ativo.

**Gate:** `./prove.sh --quiet`, `make -C research test`, auditoria de proveniência e revisão independente dos mapas.

**Parar se:** números divergirem, o README prometer mais que os artefatos ou o proof estiver verde por falha de shell.

## Fase B — Mecanismo causal

**Pergunta:** o Symbiont detecta uma transferência impossível quando o efeito oculto não é observável?

**Entrega:** runtime, oracle e casos em processos separados; família `probe–commit gap`; trace e digests rederiváveis.

**Gate:** mutação de labels não muda o oracle; probe não muta estado privado; orçamento zero significa zero chamadas; efeito oculto bloqueia.

**Parar se:** o runtime recebe a fixture, o oracle copia o label, ou a proposta vira `SUPPORTED` sem atestação externa.

## Fase C — HERUS Bridge sintético

**Pergunta:** uma pessoa consegue distinguir proposta, prévia, confirmação, cancelamento, abstenção e parada?

**Entrega:** três braços locais: baseline convencional, condição sem Symbiont e HERUS checked. Nenhum braço transmite ou executa efeito externo.

**Gate:** eventos válidos, prévia obrigatória, cancelamento reversível, parada independente, zero commits externos e rastreabilidade de eventos.

**Parar se:** a pessoa não sabe o que aconteceria, o sistema faz algo sem confirmação ou a abstenção é contada como sucesso.

## Fase D — Sessões formativas

**Pergunta:** o vocabulário de controle é compreendido sem treinamento excessivo?

**Entrega:** cinco ou seis sessões formativas, consentimento, acessibilidade, retenção mínima e resultados brutos.

**Gate:** compreensão de ação e não ação, cancelamento e parada. Esta fase não calcula ainda “benefício social”.

**Parar se:** a pessoa atribui autoridade ao HERUS, não consegue cancelar ou precisa de intervenção frequente.

## Fase E — Piloto pareado

**Pergunta:** a proposta reduz erro, tempo ou dependência sem introduzir custo ou risco maior?

**Entrega:** protocolo congelado com baseline convencional, condição sem Symbiont e condição HERUS. Métricas e análise precisam ser definidas antes dos resultados.

**Gate:** benefício mensurável, falhas reportadas, controles de segurança, privacidade e acessibilidade completos.

**Parar se:** o efeito existir apenas em uma tarefa artificial, desaparecer contra o baseline ou exigir autoridade ambígua.

## Fase F — Hardware

**Pergunta:** o mecanismo e o produto continuam compreensíveis em um hospedeiro físico?

**Entrega:** identificação de placa, boot, memória, latência, rádio, háptica, energia e interação, nesta ordem.

**Gate:** B1 e B2 antes de qualquer claim físico; depois B3–B10 conforme os manifestos de bancada.

**Parar se:** medição não reproduzir a hipótese, o consumo impedir o uso ou a ergonomia destruir a compreensão.

## Fase G — Release ou encerramento

Um release só pode ocorrer com atestação de proveniência, documentação de limites, evidência externa e uma tarefa que justifique o produto. Se os gates humanos ou causais falharem, o resultado correto é redesenhar ou encerrar a linha, não adicionar mais complexidade.

## Linha de decisão

```text
mapear → reproduzir → provar soberania local → isolar causalidade
      → transferir uma Skill entre hosts → integrar pulso e revogação
      → testar compreensão → medir valor → medir hardware → replicar
```

Nunca inverter esta ordem para usar hardware, voz, LLM ou nuvem como substitutos de uma hipótese não demonstrada ou da soberania dos dados.

## Referências

[1]: 04-MAPA-DA-EVIDENCIA.md "Mapa de evidências"
[2]: 56-RODADA-DECISIVA-HERUS-BRIDGE.md "Rodada decisiva"
[3]: ../research/evidence/hardware_readiness_manifest.json "Manifesto de prontidão física"
[4]: 52-DEFINICAO-SIMBIOSE-UTIL.md "Contrato de utilidade"
