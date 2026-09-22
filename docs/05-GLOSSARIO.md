# HERUS — Glossário

## ASA — Adaptive Symbiotic Architecture

Arquitetura de pesquisa que separa identidade persistente, hospedeiro, mundo observado e autoridade humana. ASA não significa inteligência geral nem consciência.

## Core

Núcleo lógico persistente do HERUS. No desenho físico histórico, também nomeia a cápsula eletrônica. Nos documentos ASA, o sentido correto depende do contexto: núcleo lógico não é automaticamente dispositivo físico.

## Host / hospedeiro

Ambiente no qual o HERUS observa recursos, estado e ações públicas. Pode ser um simulador, um firmware, um notebook, um servidor ou futuramente um dispositivo físico.

## Host Model

Perfil finito do hospedeiro atual: identidade do host, recursos, ações seguras, chaves de estado e época. Ele deve ser renovado quando o vínculo muda.

## World Model

Modelo das transições observadas: ação, estado inicial, estado final e efeito. Conflitos retiram uma previsão; não são resolvidos por escolha arbitrária.

## Self Model

Identidade e repertório persistentes do HERUS. A persistência não deve carregar automaticamente autoridade, contexto ou Skills de outro hospedeiro.

## Skill

Capacidade finita verificada. No Symbiont v2, é uma sequência de primitive actions com meta, efeitos e digests de evidência.

## Symbiont

Runtime de pesquisa que descobre transições em um hospedeiro, compõe uma Skill e tenta propor sua transferência para outro host. O runtime atual propõe; não executa o alvo.

## Rebind

Troca de hospedeiro. O `herus_id` pode persistir, mas mundo, contexto e repertório local precisam ser reancorados.

## Semantic IR

Representação intermediária semântica com schema e identidade canônica. Ela organiza intenção; não concede permissão para executar.

## Black-box

Fronteira em que o runtime conhece apenas a interface pública declarada. Ele não deve ler estado privado, fixture, oracle ou implementação do host.

## Holdout

Caso ou conjunto mantido separado do processo que produz a hipótese. Um holdout público, rotulado ou executado no mesmo processo é mais fraco que um holdout causalmente isolado.

## Oracle

Componente que deriva um veredicto a partir de uma verdade de referência. Para ser útil, precisa estar separado do runtime e não copiar labels esperados.

## Proposal / proposta

Plano serializado e revisável. Não é execução, autorização, segurança nem sucesso humano.

## Authority / autoridade

Permissão explícita para produzir um efeito externo. Conexão, descoberta, contrato textual ou proposta não equivalem a autoridade.

## Fail-closed

Diante de incerteza, conflito, digest ausente, orçamento excedido ou autoridade ausente, o sistema bloqueia ou se abstém.

## SAFE_BUT_UNPROVEN

Estado máximo atual para uma proposta local sob contratos de pesquisa. Significa que a proposta foi construída, mas não recebeu atestação externa suficiente.

## HERUS Bridge

Vertical slice local que apresenta uma proposta de remapeamento entre interfaces. Exige prévia e permite confirmar, cancelar, abster ou parar. Não produz efeitos externos.

## `pre_hardware`

Estado em que a engenharia está preparada para iniciar medições físicas. Não significa que autonomia, rádio, energia ou ergonomia foram provados.

## Simbiose útil

Definição congelada que exige mecanismo, segurança, privacidade, reprodutibilidade e benefício humano medido contra baseline. O HERUS ainda não atingiu esse estado.

## Referências

[1]: 02-MAPA-DA-ARQUITETURA.md "Mapa da arquitetura"
[2]: 04-MAPA-DA-EVIDENCIA.md "Mapa da evidência"
[3]: 52-DEFINICAO-SIMBIOSE-UTIL.md "Definição congelada de simbiose útil"
