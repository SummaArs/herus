# HERUS — Contrato do Core como alimentação, não como cérebro

**Versão:** `knowledge_feed-v1`  
**Princípio:** o HERUS vestível permanece inteligente quando o Core é desligado.

## 1. Decisão arquitetural

O Core externo não é uma segunda instância obrigatória da mente do HERUS. Ele é uma estação de suporte que pode fornecer energia, transporte, antena, armazenamento intermediário e propostas de conhecimento. O reasoner, a memória, o compilador, o planner, a abdução, a abstenção e a apresentação háptica devem estar no pulso.

Essa separação não é apenas uma decisão de produto. Ela reduz dependência operacional e torna testável a pergunta mais importante: **o usuário continua conseguindo consultar, lembrar, raciocinar e planejar localmente sem o Core?** Se a resposta for não, o Core deixou de ser suporte e virou cérebro remoto.

## 2. O feed não é uma atualização de firmware

O `knowledge_feed` é um pacote tipado de **propostas de conhecimento**. Ele não contém comandos de execução, rádio, chaves, texto bruto, áudio, transcript, embedding, identidade ou localização. O Core não pode enviar uma frase que o pulso interprete como uma ordem.

A decisão foi inspirada por propriedades de manifestos para dispositivos restritos: um receptor deve verificar integridade, autenticidade e aplicabilidade antes de processar um payload [1] [2]. Também aproveitamos a separação de papéis de Uptane — quem produz, delega, autoriza e verifica não precisa ser o mesmo ator [3] — e o princípio de proveniência verificável do in-toto [4]. O HERUS, entretanto, não está implementando SUIT ou Uptane completos nesta etapa.

## 3. Fluxo de aceitação no pulso

O fluxo obrigatório é:

| Etapa | Ação do HERUS | Efeito permitido |
|---|---|---|
| `RECEIVED` | recebe bytes ou registros do Core | nenhum efeito cognitivo |
| `REJECTED_*` | valida schema, limites, versão, namespace, digest e autenticação | descarta ou mantém somente diagnóstico numérico |
| `VERIFIED_PROPOSAL` | valida estrutura e compatibilidade local | proposta permanece fora da memória |
| `PRESENTED_FOR_CONFIRMATION` | mostra estado e evidência resumida | usuário decide se aplicável |
| `ACCEPTED_FACTORY` | política local aceita regra/conhecimento de fábrica | pode entrar no namespace factory |
| `ACCEPTED_PERSONAL` | confirmação física explícita aceita item pessoal | pode entrar no namespace personal |
| `EXPIRED` | TTL ou versão deixa de ser válida | nunca reativa silenciosamente |

A ordem é importante. O Core não pode ser tratado como confiável apenas porque está fisicamente acoplado ou porque o pacote possui uma assinatura. Proveniência, compatibilidade semântica, conflito e política de incorporação continuam sendo decisões no pulso.

## 4. Identidade collision-aware

Cada handle cognitivo interno deve ser um handle de registry versionado de 32 bits, com namespace, versão e slot. O HCP de 16 bits pode continuar existindo como compatibilidade de transporte, mas a projeção nunca será a identidade primária de um fato, regra, prova ou cartão.

Um pacote que referencia versão incompatível, namespace inexistente, slot não pertencente ao registry local ou projeção ambígua deve retornar `REJECTED_VERSION` ou `REJECTED_NAMESPACE`. Não haverá migração implícita durante a consulta: uma migração será uma operação explícita, versionada e testada.

## 5. Autoridade

O Core pode carregar e alimentar; não pode autorizar. Nenhum registro de feed pode possuir `executor`, `radio_command`, `confirm_memory` ou equivalente. Uma regra aceita do Core ainda é apenas conhecimento local; um plano derivado dessa regra continua sendo proposta; uma vibração háptica continua sendo apresentação; e qualquer ação exige confirmação física no pulso.

O namespace `PERSONAL` exige confirmação explícita do usuário. O namespace `FACTORY` pode ser aceito por política local somente quando a proveniência, a compatibilidade e o digest passarem os gates definidos. Mesmo conhecimento de fábrica não concede autoridade para enviar, comprar, abrir, apagar ou modificar qualquer coisa.

## 6. Fallback offline

Quando o Core não estiver presente, o resultado do canal externo será `CORE_UNAVAILABLE`. Isso não altera o estado do reasoner, não apaga memória pessoal, não bloqueia consultas locais, não revoga o registry atual e não transforma uma resposta local em erro de rede.

A única capacidade que pode faltar é o conhecimento novo que ainda não foi incorporado. O HERUS deve continuar capaz de responder com conhecimento já aceito, produzir prova, abster-se quando faltar evidência, consultar memória local e formular plano bounded. Um feed pendente não pode ser assumido como disponível apenas para evitar uma abstenção.

## 7. Estado de implementação e limites

O contrato agora possui uma implementação host-side em `firmware/core/knowledge_feed.{h,c}`. Ela calcula digest canônico SHA-256, verifica schema, versão, sequência monotônica, namespace, limites, formato e um verificador de autenticação fornecido externamente. A aplicação é transacional e só promove fatos e regras depois de confirmação local. A migração collision-aware dos consumidores internos passa por provas do reasoner, planner, diálogo e abdução.

Ainda não há, nesta etapa, armazenamento seguro de trust anchors, verificador de assinatura ligado ao secure element real, cursor anti-rollback persistido após reboot ou ensaio físico Core↔pulso. Portanto, `authn_status=VERIFIED_SIGNATURE` é uma capacidade do contrato, não evidência de uma cadeia criptográfica física concluída. A suíte host usa um verificador de enlace controlado para testar o gate, sem expor chaves.

Os resultados e limites estão registrados em `docs/66-HERUS-MIGRACAO-HANDLES-E-KNOWLEDGE-FEED.md`. O feed continua sendo um caminho de proposta de produto, não uma autorização para elevar o Core a cérebro remoto.

## Referências

[1]: https://www.rfc-editor.org/rfc/rfc9019 "RFC 9019: A Firmware Update Architecture for Internet of Things"

[2]: https://datatracker.ietf.org/doc/html/draft-ietf-suit-manifest-37 "IETF SUIT Manifest draft-37"

[3]: https://uptane.org/docs/latest/standard/uptane-standard "Uptane Standard 2.1.0"

[4]: https://github.com/in-toto/attestation "in-toto Attestation Framework"
