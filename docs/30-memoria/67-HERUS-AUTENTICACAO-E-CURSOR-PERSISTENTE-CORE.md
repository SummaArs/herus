# HERUS — Autenticação do Core feed e cursor anti-rollback persistente

**Estado:** host-only, C11, publicado em PR incremental.  
**Princípio:** autenticar a alimentação não transforma o Core em autoridade.

## 1. Motivação

O `knowledge_feed` anterior já rejeitava digest adulterado, namespace errado, sequência antiga e ausência de confirmação local. Havia, entretanto, duas fronteiras que ainda precisavam de prova mais forte: a autenticação concreta de um pacote vindo de um enlace emparelhado e a sobrevivência do cursor anti-rollback a um reboot ou interrupção de escrita.

Esta PR fecha essas duas lacunas no host sem fingir que o secure element físico já está conectado.

## 2. HMAC sobre o digest canônico

O pacote agora possui `auth_tag` de 16 bytes, produzido por HMAC-SHA256 sobre o `payload_digest` de 32 bytes. A chave é emprestada somente durante a chamada de verificação; não é copiada para o pacote, não aparece nos registros do produto e não entra no caminho cognitivo.

A API `kf_hmac_tag()` permite ao adaptador de enlace produzir a tag, enquanto `kf_hmac_verify()` verifica o status `KF_AUTH_VERIFIED_SIGNATURE` e compara a tag com `ct_eq()`. A primitive HMAC já existente no repositório é reutilizada; não foi criada uma criptografia paralela. Uma tag adulterada retorna `KF_REJECTED_AUTHORITY` antes de qualquer proposta ou inserção.

| Caso | Resultado host |
|---|---|
| Digest e HMAC válidos | pacote passa os gates |
| Tag adulterada | rejeição de autoridade |
| Digest adulterado | rejeição de digest |
| Pacote sem autenticação | rejeição de autoridade |
| Tag válida sem confirmação local | permanece proposta |
| Tag válida com confirmação local | conhecimento pode ser promovido pela política local |

HMAC fornece autenticação simétrica de um enlace emparelhado; ele não é uma assinatura pública de proveniência. A proveniência do produtor e a política de trust anchor continuam sendo responsabilidades do verificador externo. Não há alegação de secure element ou de assinatura assimétrica implementada nesta PR.

## 3. Cursor persistente em dois slots

`knowledge_feed_cursor.{h,c}` serializa o cursor em um registro canônico sem padding de C. Cada registro contém formato, versão de registry, sequência, digest do payload, tag HMAC e bytes reservados que devem permanecer zero. A persistência usa dois slots alternados:

| Operação | Gate |
|---|---|
| Carregar dois slots ausentes | estado vazio legítimo |
| Carregar registro presente | formato, versão, HMAC e sequência válidos |
| Dois registros válidos | escolhe a maior sequência |
| Mesma sequência com digest diferente | bloqueia como rollback/ambiguidade |
| Registro presente corrompido | bloqueia; não cai silenciosamente para slot velho |
| Commit | escreve slot alternado, lê de volta e só então promove RAM |
| Falha de escrita | floor em RAM permanece inalterado |
| Readback adulterado | promoção é recusada |
| Sequência igual ou menor | rollback rejeitado |

A escolha de bloquear diante de um slot presente corrompido é deliberada. Recuperar automaticamente um slot antigo pode reabrir uma janela de replay; a recuperação deve entrar em quarentena e exigir tratamento explícito do sistema de armazenamento.

## 4. Evidência de execução

A suíte `knowledge-feed` passa **16/16** e a suíte `knowledge-feed-cursor` passa **11/11**. O pipeline global `./prove.sh --quiet` termina com todas as invariantes host, as **111 invariantes simuladas** e a mutação global **7/7** passando.

> Esses números provam os contratos executados no host. Eles não provam durabilidade de flash, brownout, wear leveling, atomicidade elétrica, disponibilidade do secure element, energia ou comportamento do Core em hardware real.

## 5. Próximo gate físico e de integração

O próximo passo responsável é adaptar `kfc_storage_t` a um backend persistente real com teste de interrupção elétrica controlada, mantendo o mesmo formato canônico. A chave deve vir da fronteira protegida de pareamento, nunca de um arquivo de configuração ou de uma constante de firmware. Antes do flash, é necessário validar o mapa de placa, manter o haptic target desabilitado por padrão e demonstrar zeroização e recuperação no alvo.
