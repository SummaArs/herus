# HERUS

**Comunicação essencial, privada e offline — com significado antes de mensagem.**

O HERUS é um sistema pessoal de comunicação para momentos em que telefone, rede móvel e interfaces densas não são a ferramenta certa. Em vez de tentar transmitir áudio ou longas mensagens, ele permite que uma pessoa expresse, confirme e envie **significados essenciais** de forma curta, privada e local.

O produto foi pensado para trilhas, áreas rurais, deslocamentos, equipes de apoio, eventos externos e qualquer situação em que coordenação clara importa mais que uma conversa longa.

> O HERUS não tenta substituir o telefone. Ele existe para preservar comunicação humana essencial quando conectividade, atenção ou privacidade falham.

## A experiência HERUS

A pessoa usa um vestível simples. Ela inicia uma interação por um gesto físico, expressa uma intenção curta e recebe confirmação clara por voz local, interface mínima ou vibração. Nada é transmitido sem confirmação física.

| Princípio | O que significa na prática |
|---|---|
| **Significado primeiro** | O rádio carrega uma intenção estruturada, não áudio ou uma transcrição literal. |
| **Privado por padrão** | O produto não depende de conta, nuvem, identidade transmitida, localização ou histórico de conversa para operar. |
| **Pessoa no comando** | Inteligência pode sugerir, resumir ou perguntar; ela nunca envia, publica, compra ou altera algo por conta própria. |
| **Offline de verdade** | O funcionamento essencial é local e continua útil sem sinal de celular ou internet. |
| **Falhar fechado** | Ambiguidade, confiança insuficiente, replay, expiração ou vínculo revogado bloqueiam a ação em vez de “tentar mesmo assim”. |

## O sistema: vestível + Núcleo

O HERUS é composto por duas presenças complementares.

| Elemento | Papel |
|---|---|
| **HERUS vestível** | A interface pessoal e imediata: gesto, fala local, confirmação, vibração e comunicação essencial de baixo consumo. |
| **Núcleo** | Um dispositivo circular de bolso que amplia bateria, antena e capacidade de computação local. Ele atua como estação-base pessoal para o vestível. |

O Núcleo é também a base para a próxima evolução do HERUS: um **complemento seletivo de memória pessoal**. A visão não é gravar toda a vida da pessoa. É identificar, com consentimento, ideias, decisões, compromissos e contexto que valem recuperar no futuro, descartando o restante.

Uma futura LLM local poderá ajudar o Núcleo a organizar e explicar essa memória. Ela será uma camada de raciocínio e recuperação, não uma fonte de autonomia: qualquer uso de comunicação, armazenamento sensível ou ação externa continuará sob controle da pessoa.

## Para quem é

| Cenário | Valor do HERUS |
|---|---|
| **Trilhas, campo e estrada** | Coordenação curta entre pessoas sem depender de cobertura móvel. |
| **Equipes de apoio** | Estados e intenções rápidos, sem a fricção de rádio de voz contínuo. |
| **Família e grupos pequenos** | Confirmação de chegada, espera, encontro, mudança de plano ou ajuda. |
| **Rotinas com baixa atenção visual** | Interação por gesto, fala curta e háptica em vez de telas e menus. |
| **Memória pessoal futura** | Recuperação privada de ideias e decisões que a pessoa autorizou lembrar. |

## Estado atual

O HERUS está em **release candidate pré-hardware**. A arquitetura, os contratos de privacidade, a confirmação física, a inteligência local limitada, o vínculo entre vestível e Núcleo, as barreiras de modelo, o cofre cifrado de cartão mínimo, a consolidação humana limitada, a recuperação tipada controlada, sua apresentação simbólica de status e a composição Grand Finale da cadeia de memória foram implementadas e verificadas em host.

Ainda não há resultados de campo. Alcance, consumo, ergonomia, reconhecimento de fala, comportamento háptico, integração BLE, armazenamento protegido e desempenho de uma LLM local precisam ser medidos no hardware real antes de se tornarem alegações de produto.

A próxima etapa física é a Fase 0: dois devkits, bancada curta, medição RF, energia e interação, com critérios de interrupção definidos antes da coleta.

## Documentação principal

| Documento | Para quê serve |
|---|---|
| [Visão do produto](docs/04-PRODUCT.md) | Propósito, proposta de valor e direção de produto. |
| [Núcleo](docs/06-NUCLEO.md) | Papel do dispositivo circular de bolso, privacidade e caminho de inteligência local. |
| [Memória seletiva](docs/17-MEMORIA-SELETIVA.md) | Política inicial para lembrar ideias, decisões e contexto útil sem gravar a vida inteira. |
| [Captura consentida](docs/18-SESSAO-CAPTURA-MEMORIA.md) | Sessão física, limitada e transitória que antecede qualquer memória pessoal. |
| [Extração de candidatos](docs/19-EXTRACAO-CANDIDATOS.md) | Interpretação local e conservadora que cria sinais tipados sem guardar a fala. |
| [Cofre de memória](docs/20-COFRE-MEMORIA.md) | Cartão mínimo cifrado, autorização humana separada, geração anti-rollback e apagamento fail-closed. |
| [Consolidação humana](docs/21-CONSOLIDACAO-HUMANA.md) | Revisão física limitada, expiração sem retenção, conflito não automático, recuperação por identificador e remoção controlada. |
| [Recuperação controlada](docs/22-RECUPERACAO-SEMANTICA.md) | Matching local de cartões tipados com limiar, razões e ambiguidade explícita; sem busca livre, escrita ou autoridade de modelo. |
| [Interface de recuperação](docs/23-INTERFACE-RECUPERACAO-HUMANA.md) | Status simbólico one-shot para correspondência, ausência e ambiguidade; sem conteúdo livre, desempate, escrita, envio ou ação. |
| [Grand Finale de memória](docs/24-GRAND-FINALE-MEMORIA.md) | Prova composta da cadeia de captura ao status humano, com conflito/modelo bloqueantes e gates explícitos para hardware e avaliação. |
| [Especificação do sistema](docs/00-HERUS-MASTER.md) | Arquitetura geral, protocolo, segurança, energia e limites conhecidos. |
| [Guia de construção](docs/03-BUILD-GUIDE.md) | Próximos passos de hardware e critérios para interromper ou prosseguir. |
| [Segurança](SECURITY.md) | O que a criptografia protege hoje e o que ainda depende de integração física. |

## Estado de engenharia

A versão consolidada pode ser verificada localmente com:

```bash
./prove.sh --quiet
```

O comando executa as verificações portáveis e o simulador. Um resultado positivo confirma contratos de software e autoriza somente o início controlado da bancada; ele **não** constitui evidência de alcance, energia, UX ou desempenho físico.

A história detalhada de experimentação, provas e decisões de implementação é preservada no ramo [`internal/engineering-archive`](https://github.com/SummaArs/herus/tree/internal/engineering-archive). Ela existe para rastreabilidade de engenharia, sem ocupar a apresentação principal do produto.

## Licença

Proprietary. Copyright © 2026 Gustavo Gonçalves. Todos os direitos reservados — veja [LICENSE](LICENSE).
