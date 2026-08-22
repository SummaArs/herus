# HERUS — Migração collision-aware e contrato executável do Core feed

**Estado:** host-only, C11, sem alegação de validação física.  
**Branch:** `feat/herus-semantic-compiler`  
**Regra de produto:** o Core alimenta propostas; o cérebro continua no pulso.

## 1. O avanço implementado

A identidade simbólica deixou de ser implicitamente limitada a 16 bits nas estruturas do reasoner. O novo alias `sr_symbol_t` usa `srreg_handle_t` de 32 bits, preservando namespace, versão e slot. `sr_fact_t`, `sr_pattern_t` e os termos do reasoner agora carregam o handle completo; `SR_SYMBOL_LEGACY` permanece somente como projeção explícita para fixtures e compatibilidade.

O planner, o diálogo e a abdução recebem esses mesmos tipos através da API compartilhada. Não foi criada uma segunda lógica paralela: a migração preserva o reasoner já testado e adiciona provas em cada fronteira pública para evitar que uma cópia intermediária volte a truncar a identidade.

| Fronteira | Evidência host observada |
|---|---:|
| Reasoner, prova, contradição, ausência, ambiguidade e abdução | 27/27 |
| Planner bounded, custo, ciclo, no-plan e confirmação | 10/10 |
| Diálogo, autoridade, consulta e abdução read-only | 21/21 |
| Compilador semântico e integração | 54/54 |
| Pacote Core feed | 14/14 |

## 2. Feed executável

`firmware/core/knowledge_feed.{h,c}` implementa um envelope bounded de fatos e regras tipados. O digest canônico é calculado localmente com SHA-256 sobre campos sem padding implícito. O pacote exige versão do schema, versão do registry, sequência monotônica, namespace permitido, TTL limitado, contagem bounded, digest correto e uma função de verificação de autenticação fornecida pela fronteira de enlace/secure element.

A função de aplicação é transacional. Sem confirmação local, o retorno é `KF_PROPOSED`, o diálogo não é alterado e o cursor anti-rollback não avança. Com confirmação local, todos os registros são aplicados em uma cópia temporária; somente se a cópia completa for aceita o estado é promovido e a sequência é registrada.

| Ataque ou condição | Resultado provado |
|---|---|
| Payload adulterado depois do digest | `KF_REJECTED_DIGEST` antes da inserção |
| Sequência repetida | `KF_REJECTED_VERSION` sem duplicação |
| Envelope personal com handles factory | `KF_REJECTED_NAMESPACE` |
| Excesso de registros | `KF_REJECTED_LIMIT` |
| Autenticação ausente ou inválida | `KF_REJECTED_AUTHORITY` |
| Feed válido sem confirmação | `KF_PROPOSED`, sem mutação |
| Feed válido com confirmação física | `KF_ACCEPTED`, commit integral |
| Core sem conhecimento novo | reasoner local continua consultável |
| Core ausente no transporte | `KF_CORE_UNAVAILABLE`, sem erro cognitivo local |

## 3. O que foi realmente provado

O pipeline `./prove.sh --quiet` terminou com **todas as invariantes host passando**, incluindo **111 invariantes de sistema simulado**, a campanha de mutação global **7/7**, a campanha F4 de controles críticos e a suíte de feed **14/14**. O resultado significa que os contratos executados na árvore local resistem aos casos enumerados.

> Esse resultado não prova assinatura de produção, armazenamento seguro de trust anchors, resistência física, alcance de rádio, consumo de energia, desempenho de voz, qualidade conversacional ou operação no ESP32-S3.

A verificação de assinatura continua uma fronteira deliberada: `kf_auth_verify_fn` recebe o pacote e o digest, mas a chave não entra no feed nem no log. A suíte host usa um verificador de enlace controlado para exercitar o gate; isso **não** é evidência de um secure element conectado nem de uma cadeia criptográfica física concluída.

## 4. Estado de soberania

O Core não possui função de execução no contrato. Ele não pode criar memória pessoal sem confirmação local, alterar trust anchors, mudar a versão do registry, enviar rádio ou bypassar abstenção. O conhecimento aceito passa a ser evidência ordinária do reasoner local, e uma consulta posterior não depende da disponibilidade do Core.

A ausência do Core não é tratada como falha cognitiva. O feed novo fica indisponível, mas fatos, regras, provas, abdução bounded e planner local continuam sujeitos às mesmas políticas. A parte ainda não conhecida deve produzir abstenção, não uma resposta inventada.

## 5. Próximo gate honesto

Antes de qualquer alegação de produto, é necessário integrar o verificador de autenticação com a fronteira real de transporte e o armazenamento seguro de trust anchors, definir política de expiração no runtime, testar reinicialização com cursor persistente e executar a bancada elétrica com `BOARD_HAS_HAPTIC_I2C=0` até o pin map ser verificado. Esses são próximos gates, não resultados atuais.
