# HERUS — Leia isto primeiro

## Em uma frase

**HERUS é uma pesquisa de arquitetura simbiótica verificável.** Ela investiga se um núcleo persistente pode observar um hospedeiro, aprender uma rotina finita, remapear essa rotina para outro hospedeiro e propor uma ação sem confundir capacidade com autoridade.

Hoje, o HERUS é um **protótipo host-only em estado `not_proven`**. Ele contém mecanismos interessantes e uma cadeia ampla de contratos, mas ainda não provou simbiose útil, benefício humano, execução segura, segurança física ou generalidade.

> A regra de leitura é simples: **primeiro entenda o que foi provado; depois leia o que foi imaginado.**

## A ordem recomendada

1. [Linha do tempo](01-LINHA-DO-TEMPO.md): mostra como a ideia cresceu, mudou e foi reduzida por auditorias.
2. [Mapa da arquitetura](02-MAPA-DA-ARQUITETURA.md): explica os conceitos sem exigir que o leitor conheça o código.
3. [Mapa do código](03-MAPA-DO-CODIGO.md): conecta cada conceito aos diretórios e entradas executáveis.
4. [Mapa da evidência](04-MAPA-DA-EVIDENCIA.md): separa demonstração, hipótese, teste negativo e trabalho pendente.
5. [Glossário](05-GLOSSARIO.md): fixa o significado dos termos que mais facilmente geram confusão.
6. [Roadmap e gates](06-ROADMAP-E-GATES.md): mostra a ordem correta para continuar sem saltar etapas.
7. [Rodada decisiva](56-RODADA-DECISIVA-HERUS-BRIDGE.md): descreve a última correção epistemológica e o HERUS Bridge.
8. [Direção soberana](57-DIRECAO-SOBERANA-E-SIMBIONTE.md): define o novo objetivo do simbionte e a soberania de dados.

## As três perguntas que governam o projeto

### O que o HERUS foi?

Ele começou como um comunicador semântico de baixa largura de banda. Depois ganhou um núcleo pessoal, voz, háptica, memória seletiva, confiança, recuperação e uma visão de hardware wearable. Em seguida, a pesquisa passou a investigar uma tese mais ambiciosa: a **Adaptive Symbiotic Architecture**, ou ASA.

### O que o HERUS é?

É um repositório com quatro corpos de trabalho: um firmware C com contratos e invariantes; um simulador C; protótipos Python de Semantic IR, raciocínio e Symbiont; e um conjunto de documentos e evidências. O núcleo de pesquisa mais defensável hoje é uma **transferência finita entre hospedeiros que produz propostas, não execução**. A direção oficial agora é transformar esse núcleo em um simbionte autônomo e soberano, sem promover essa capacidade antes dos gates correspondentes.

### O que o HERUS será?

Ele deverá avançar por soberania local, separação entre Core, gateway e hospedeiro, transferência de uma Skill entre dois sistemas, integração com o pulso e só então sessões humanas, valor comparativo e hardware. Hardware, rádio, voz, LLM local e execução externa não podem substituir a prova de que os dados permanecem sob controle do usuário.

## Como interpretar este repositório

| Sinal | Significado |
|---|---|
| `PASS` no firmware ou na pesquisa | Um contrato específico passou no ambiente e nos fixtures declarados. |
| `not_proven` | Há uma hipótese ou mecanismo, mas faltam dimensões obrigatórias para uma conclusão forte. |
| `ABSTAIN` ou `BLOCKED` | O sistema recusou avançar diante de ambiguidade, falta de evidência ou autoridade. |
| `pre_hardware` | O trabalho físico ainda depende dos gates de identificação e boot. |
| Documento histórico | Preserva uma ideia ou resultado anterior; não necessariamente representa o estado atual. |
| Manifesto de proveniência | Lista os artefatos locais protegidos por digest; não é uma assinatura externa. |

## Verificação mínima

```bash
./prove.sh --quiet
make -C research test
PYTHONPATH=research python3 -m research.bridge_causal
```

Um resultado verde confirma regressões internas e invariantes controladas. Não confirma impacto social, segurança de campo, autonomia, alcance de rádio ou simbiose geral.

## Referências

[1]: ../README.md "Estado público atual do HERUS"
[2]: 01-LINHA-DO-TEMPO.md "Linha do tempo didática do HERUS"
[3]: 04-MAPA-DA-EVIDENCIA.md "Mapa de evidências e limites"
[4]: 56-RODADA-DECISIVA-HERUS-BRIDGE.md "Rodada decisiva e HERUS Bridge"
