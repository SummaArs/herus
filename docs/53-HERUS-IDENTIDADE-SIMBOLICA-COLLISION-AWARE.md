# HERUS — Identidade simbólica collision-aware

**Status:** especificação host-first; nenhuma alteração de ABI ou firmware é declarada por este documento.

## Problema

O compilador semântico atual usa FNV-1a dobrado para um ID de 16 bits. Isso é rápido e determinístico, mas não é uma identidade universal: a auditoria encontrou `gh` e `ne` com o mesmo ID `14346`. A barreira intra-utterance já impede que essa colisão produza IR na mesma frase, porém ela não resolve colisões entre utterances nem oferece um vocabulário aberto seguro.

> Um hash pode ser um bom índice. Não é, sozinho, uma prova de que duas chaves são a mesma entidade.

O contrato do novo domínio deve permanecer separado do texto de produto: o reasoner recebe somente handles tipados; texto transitório pode ser usado para resolução, mas não entra em fatos, provas, logs ou telemetria.

## Alternativas

| Arquitetura | Propriedade forte | Limitação | Decisão preliminar |
|---|---|---|---|
| Hash rápido de 16/32 bits | Baixo custo e determinismo | Colisões inevitáveis no domínio aberto; aumentar largura não prova unicidade | Rejeitada como identidade única |
| Hash criptográfico de 64/256 bits | Colisão acidental extremamente improvável; pode oferecer fingerprint | Continua sendo uma garantia computacional, não unicidade matemática; exige política de keying/versionamento | Útil como fingerprint de verificação, não como identidade isolada |
| MPHF para vocabulário de fábrica | Mapeia conjunto estático conhecido sem colisões e gera IDs densos | Não aceita chaves pessoais novas; chave desconhecida pode apontar para um slot se não houver verificação de pertencimento | Aceita para namespace estático, com verificação de membership |
| Interning dinâmico exato | Chaves distintas são comparadas antes de receber handle; colisão de índice é resolvida, não escondida | Exige capacidade, tabela de resolução, versão e política de reboot/migração | Aceita para namespace pessoal controlado |
| Handle opaco versionado + namespaces | Separa identidade da representação numérica e permite migrar ABI | Exige mudança de contrato entre compilador, reasoner e planner | Arquitetura escolhida |

A motivação para a MPHF é específica: a documentação do CMPH define perfect hashing como o mapeamento de um conjunto estático para inteiros sem colisões e cita vocabulários e compiladores como aplicações [1]. A motivação para não tratar um hash como prova é a distinção do NIST entre resistência a colisões e unicidade: resistência a colisões significa dificuldade computacional de encontrar entradas diferentes com o mesmo valor, não impossibilidade matemática [2].

## Arquitetura escolhida

A identidade será um **handle opaco versionado**, com dois namespaces:

| Namespace | Conteúdo | Resolução | Persistência |
|---|---|---|---|
| `FACTORY` | Vocabulário imutável de relações, ações, lugares e entidades de fábrica | MPHF estático + verificação de membership | Código/asset versionado |
| `PERSONAL` | Entidades introduzidas pela pessoa ou por um dispositivo autorizado | Interning exato, limitado e collision-aware | Apenas registro autorizado; nunca log bruto |

O handle não deve carregar a semântica textual. Uma proposta inicial host-only é um identificador de 32 bits com campos de versão, namespace e slot; a largura final deve ser escolhida depois de medir migração, memória e ABI. O campo numérico é um endereço interno, não uma prova criptográfica nem uma autorização.

### Namespace `FACTORY`

A ferramenta de build recebe uma lista estática canônica. Ela gera uma MPHF, uma tabela de membership e um mapa de handle. Uma entrada desconhecida que apenas cai em um slot existente é rejeitada pela verificação de membership. A versão do léxico faz parte do handle e da tabela; versão desconhecida produz abstention.

A lista de fábrica pode existir como asset técnico imutável porque não é memória pessoal. Mesmo assim, ela não deve ser emitida em logs de produto nem confundida com conhecimento adquirido.

### Namespace `PERSONAL`

A resolução pessoal não pode converter automaticamente qualquer hash em identidade. O interner mantém uma tabela limitada de entradas canônicas durante a resolução. Para cada entrada:

1. compara a chave canônica transitória com as chaves já internadas;
2. reutiliza o handle somente em igualdade exata;
3. aloca um novo slot somente após confirmação de autoridade;
4. retorna `COLLISION`, `FULL`, `VERSION_MISMATCH` ou `ABSTAIN` quando a prova não é suficiente;
5. nunca coloca a chave bruta em fatos, provas, logs ou telemetria.

Se o sistema não puder manter uma associação privada autorizada entre a pessoa e o handle após reboot, ele deve tratar a entidade como desconhecida e pedir confirmação novamente. Não deve reconstruir identidade por um hash curto.

## Invariantes obrigatórios

| ID | Propriedade | Falha aceitável |
|---|---|---|
| `ID-01` | Mesma chave canônica e mesma versão retornam o mesmo handle | Nenhuma; erro interno é falha |
| `ID-02` | Duas chaves diferentes aceitas no mesmo namespace nunca compartilham handle | `COLLISION`/abstention |
| `ID-03` | Chave desconhecida não é aceita apenas por cair em um slot | `UNKNOWN` |
| `ID-04` | Namespace e versão não podem ser confundidos | `VERSION_MISMATCH` |
| `ID-05` | Capacidade cheia nunca vira reutilização silenciosa | `FULL` |
| `ID-06` | Migração preserva a relação entre handle antigo e novo ou abstém | `MIGRATION_REQUIRED` |
| `ID-07` | Reasoner, planner e diálogo recebem handles, nunca texto bruto | rejeição de ABI/ponte |
| `ID-08` | Resolução sem confirmação não cria identidade pessoal durável | `AUTH`/proposta transitória |
| `ID-09` | Um handle não concede autoridade, transmissão ou execução | nenhuma mutação automática |
| `ID-10` | A versão do léxico é observável para auditoria, mas não expõe a chave pessoal | metadado mínimo |

## Critério de decisão para a próxima fase

A arquitetura só será implementada se um modelo host-only demonstrar `ID-01` a `ID-10` sob colisões conhecidas, overflow, versões incompatíveis, reboot/migração, repetição, entradas sensíveis e tentativas de inserir texto bruto. Se a MPHF não puder ser integrada sem verificação de membership, ela será rejeitada para evitar falsos positivos.

O próximo passo é construir esse modelo mínimo fora do firmware e medir o custo de tabela, número de entradas, largura de handle e migração. Nenhum ABI atual deve ser alterado antes desse resultado.

## Referências

[1]: https://cmph.sourceforge.net/ "CMPH — C Minimal Perfect Hashing Library"

[2]: https://csrc.nist.gov/projects/hash-functions "NIST — Hash Functions"
