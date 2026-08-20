# HERUS — Contrato de magia confiável e surpresa útil

**Estado:** contrato de design e segurança para a próxima evolução.  
**Objetivo:** fazer o HERUS parecer mágico porque entende contexto pessoal relevante, nunca porque esconde vigilância ou autoridade.

## 1. Definição operacional

A “mágica” do HERUS não é uma afirmação sobrenatural nem uma promessa de inteligência ilimitada. É a percepção de que o dispositivo conectou, no momento certo, fatos que a pessoa já autorizou e provavelmente teria dificuldade de recuperar sozinha.

Uma surpresa útil precisa ser **relevante, explicável, proporcional, reversível e local**. Se qualquer uma dessas condições falhar, o comportamento correto é permanecer silencioso ou pedir confirmação — não improvisar uma ação.

> O efeito desejado é: “Como ele percebeu isso?” A resposta técnica precisa existir, ser curta e auditável: “Usei estas memórias autorizadas e esta regra local.”

## 2. Classes de antecipação

| Classe | Exemplo de capacidade | Condição mínima |
|---|---|---|
| **Lembrete contextual** | lembrar uma decisão ou compromisso quando uma consulta local coincide com o contexto | memória revisada, evidência não expirada e alta relevância |
| **Conexão pessoal** | associar uma ideia atual a um projeto ou preferência já armazenada | pelo menos duas evidências compatíveis e explicação disponível |
| **Lacuna útil** | dizer que falta uma premissa para concluir algo | prova de ausência local, sem pedir ao Core automaticamente |
| **Contradição protetiva** | alertar que duas memórias autorizadas divergem | conflito explícito; nunca escolher pela recência |
| **Sugestão de próximo passo** | propor uma ação ou pergunta de confirmação | plano bounded, custo conhecido e confirmação física antes de executar |
| **Surpresa emocional ou sensível** | qualquer inferência sobre saúde, terceiros, identidade, localização ou intimidade | não antecipar automaticamente; exigir interação explícita e política específica |

## 3. Invariantes MAGIC-01 a MAGIC-10

| ID | Invariante |
|---|---|
| `MAGIC-01` | Toda antecipação nasce de evidência local autorizada ou de uma regra local verificável. |
| `MAGIC-02` | O HERUS não usa áudio bruto, transcrição, embedding, identidade, localização ou chave como explicação ou log de produto. |
| `MAGIC-03` | Uma surpresa pode ser apresentada, mas nunca executa envio, compra, alteração, rádio ou memória pessoal sem confirmação física. |
| `MAGIC-04` | Memória expirada, supersedida, conflitante ou ambígua não sustenta antecipação afirmativa. |
| `MAGIC-05` | Toda conclusão antecipada possui provenance mínima: cartões, gerações e regras; a pessoa pode pedir “por quê?”. |
| `MAGIC-06` | Se o contexto não for suficientemente discriminante, o HERUS permanece silencioso ou pergunta, em vez de interromper. |
| `MAGIC-07` | O Core não é consultado como efeito colateral de uma surpresa e não define a identidade da evidência. |
| `MAGIC-08` | O orçamento de memória, passos e energia lógica é bounded; atingir o limite produz abstinência explícita. |
| `MAGIC-09` | Surpresas sobre terceiros, dados sensíveis ou inferências íntimas exigem uma política local mais restritiva e não são antecipadas por padrão. |
| `MAGIC-10` | Ausência do Core, do relógio ou de um sensor não converte falta de contexto em certeza inventada. |

## 4. Métrica de sucesso antes do hardware

No host, a próxima implementação pode provar somente comportamento de contrato: relevância determinística em fatos simbólicos, explicação de provenance, abstenção sob ambiguidade, bloqueio de conteúdo sensível, não mutação do reasoner base e ausência do Core.

Não deve afirmar que a experiência será naturalmente mágica, que a antecipação terá acurácia humana ou que o dispositivo compreenderá fala aberta. Essas propriedades só poderão ser avaliadas depois de uma bancada física e de um protocolo humano aprovado.

## 5. Implementação executável

A camada `magic_anticipation` agora funciona como mediador bounded entre evidência semântica, consulta local e proposta de diálogo. Ela retorna uma proposta tipada de antecipação, não uma ação. O diálogo ou o adaptador háptico poderá apresentar a proposta; somente uma confirmação física canônica poderá promover qualquer operação que altere o mundo ou a memória pessoal.

A suíte de antecipação passa **17/17**, a suíte do gatilho temporal passa **12/12** e a ponte read-only de diálogo passa **8/8**. O pipeline global passa com **61 suítes**, **111 invariantes de sistema simulado** e mutação adversarial **7/7**. Os casos cobrem lembrança, conexão, silêncio fora da janela, bloqueio de contexto pessoal proativo, bloqueio de conteúdo sensível e de terceiros, lacuna conhecida, expiração, contradição, ambiguidade, limite de raciocínio, TTL por geração, overflow, orçamento de apresentações e preservação do turno do diálogo.

Esses resultados provam apenas o contrato determinístico no host. Não medem se a pessoa realmente achará a experiência mágica, nem validam voz aberta, sensores, energia, latência ou háptica física.

## Referências internas

[1]: docs/69-HERUS-PONTE-MEMORIA-REASONER-OFFLINE.md "Ponte memória–reasoner offline"

[2]: docs/68-HERUS-MEMORIA-SEMANTICA-TEMPORAL-E-CONFLITOS.md "Memória semântica temporal e conflitos"

[3]: docs/64-HERUS-SOBERANIA-ON-WRIST-E-FRONTEIRA-CORE.md "Soberania on-wrist e fronteira Core"
