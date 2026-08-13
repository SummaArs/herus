# Cofre de memória seletiva — cartão mínimo cifrado e reversível

> **Estado da evidência:** este passo prova em host o contrato de software do cofre e suas falhas adversariais simuladas. Ele não prova que uma unidade ESP32-S3 física protege a raiz, a NVS, a RAM, eFuses, debug ou o armazenamento contra extração física. Essas propriedades dependem de integração alvo, provisionamento e bancada.

O Passo 4 acrescenta ao Núcleo HERUS um **cofre local de um cartão semântico mínimo**. A finalidade não é gravar uma conversa, uma biografia nem uma suposta verdade sobre alguém. A finalidade é guardar, depois de uma autorização humana explícita, um pequeno registro tipado de uma observação que a política já classificou como apropriada para memória complementar seletiva.

| Limite | Consequência verificável neste passo |
|---|---|
| Sem áudio, texto, transcrição, resumo ou embedding | O formato do cartão e o blob não têm campos para esses conteúdos. |
| Sem identidade, localização, timestamp ou rede | Nem o cartão, nem métricas, nem a porta de armazenamento expõem tais campos. |
| Sem chave pública na API ou no blob | A raiz é recebida somente por uma porta interna durante a operação; chave derivada e raiz são zeroizadas. |
| Sem persistência por candidato ou modelo | `memory_candidate_t`, diálogo, rádio e LLM não aparecem na API de selagem. |
| Sem persistência automática de dados sensíveis ou de terceiros | O cofre só aceita sinal `SELF` e `ORDINARY` que a política reavalia como `AUTO_ELIGIBLE`. |
| Sem “melhor esforço” em inconsistência | Falha de raiz, leitura, escrita, confirmação de geração, autenticação ou erase bloqueia o cofre. |

## 1. Posição na cadeia de memória

O cofre é deliberadamente posterior à política de relevância, à sessão física de captura e à extração tipada. Um resultado `AUTO_ELIGIBLE` não é autorização para escrever. A autorização é uma decisão separada, ligada ao identificador do cartão e a um recibo opaco de revisão, e exige `human_confirmed == 1` exatamente. O módulo não cria essa autorização, não a deduz da fala e não transforma sinal de modelo em consentimento.

| Etapa | Produz | Pode persistir? | Relação com o cofre |
|---|---|---:|---|
| Passo 1 — política | Disposição de relevância | Não | O cofre revalida `AUTO_ELIGIBLE`. |
| Passo 2 — captura | Janela física, limitada e transitória | Não | O cartão deve declarar sessão autorizada. |
| Passo 3 — extração | Candidato tipado e incerto | Não | Não há API de candidato para `seal`. |
| **Passo 4 — cofre** | Cartão mínimo cifrado | **Sim, após confirmação humana** | Uma única unidade selada, reversível por erase. |
| Passo 5 — consolidação | Fluxo humano de revisão/curadoria | Só através do cofre | Emite autorização disciplinada após confirmação física. |

A separação evita converter classificação estatística em uma ordem de retenção. Mesmo que, no futuro, uma LLM local ajude a explicar ou recuperar memória, ela não terá uma chamada direta que escreva no cofre. A confirmação física continua sendo requisito de arquitetura, não um detalhe de interface.

## 2. Contrato público e dados persistidos

A porta `memory_vault_store.h` contém somente o backend opaco de blob e de piso de geração. A raiz não pertence a nenhum tipo público: `memory_vault.c` a obtém apenas por um adaptador privado de plataforma, durante uma operação, e o restante do produto não recebe raiz, chave derivada, nonce ou formato cifrado. O backend armazena um blob fixo de **68 bytes** e um piso monotônico de geração, não estruturas C em claro.

| Campo do cartão | Tipo lógico | Por que existe | O que não revela |
|---|---|---|---|
| `card_id` | Identificador local opaco | Vincula autorização e leitura esperada | Conteúdo da observação. |
| `review_receipt_id` | Recibo local opaco | Vincula persistência a uma revisão humana anterior | Identidade da pessoa ou texto da revisão. |
| `memory_signal_t` | Tipo, escopo, sensibilidade e percentuais | Preserva limites de política para revisão posterior | Áudio, transcrição ou semântica livre. |
| `origin` | Explícita ou inferência controlada | Preserva incerteza de origem | Fonte textual ou histórico da captura. |
| `extract_reasons` | Máscara de razões tipadas | Explica categoria de extração sem conteúdo | A frase que motivou a extração. |

O cartão aceito precisa ter `session_authorized == 1`, escopo `SELF`, sensibilidade `ORDINARY`, valores canônicos e limitados, origem conhecida e razões não vazias. A política é chamada novamente e precisa retornar `AUTO_ELIGIBLE`; cartões `REVIEW`, sensíveis, de terceiros, ambíguos ou malformados são rejeitados antes que a raiz seja carregada.

| Parte do blob de 68 bytes | Tamanho | Proteção | Finalidade |
|---|---:|---|---|
| Cabeçalho | 28 B | **AAD autenticado** | Versão, contexto do cofre, `card_id`, geração e nonce canônico. |
| Cartão cifrado | 24 B | Confidencialidade e integridade AEAD | Sinal tipado, origem, razões, recibo e reserva canônica. |
| Tag | 16 B | Tag ChaCha20-Poly1305 completa | Rejeita alteração, contexto errado e raiz errada. |

A reserva deve ser zero, e o desempacotamento revalida o cartão. Isso faz com que uma futura mudança de formato seja uma evolução de versão explícita, em vez de interpretação permissiva de bytes desconhecidos.

## 3. Derivação, AEAD e contexto autenticado

O módulo usa os primitivos portáveis que o repositório já testa contra vetores RFC e implementação independente: HKDF-SHA-256 e ChaCha20-Poly1305. A tag é sempre a tag completa de 16 bytes; o encurtamento admitido para quadros de rádio não é usado no cofre local.

| Elemento | Regra do cofre | Objetivo |
|---|---|---|
| Raiz | 32 B, fornecida temporariamente por adaptador privado de plataforma | Não aparecer em tipo público, blob, telemetria ou API de produto. |
| Sal HKDF | `vault_id || generation` (8 B) | Separar chave por contexto e geração. |
| Informação HKDF | `HERUS/MEMORY-VAULT/v1` | Separação de domínio explícita. |
| Chave AEAD | Saída de 32 B da HKDF | Uma chave distinta para cada geração do cofre. |
| Nonce de 12 B | `HMV1 || vault_id || generation` | Nonce canônico e único por par chave-geração. |
| AAD de 28 B | Cabeçalho inteiro | Autentica versão, contexto, cartão, geração e nonce. |
| Tag | 16 B | Autenticação integral do ciphertext e AAD. |

A chave por geração é essencial: apagar um cartão não reduz o piso de geração, e uma escrita posterior usa geração maior, chave diferente e nonce diferente. A raiz e a chave derivada são sobrescritas com `secure_zero` ao fim da operação. Esta zeroização é uma propriedade do código compilado e testado em host; ela não é, sozinha, uma demonstração de ausência de remanência em registradores, cache, RAM externa ou hardware físico.

A decisão segue o princípio de que material de chave e informações criptográficas exigem proteção e gestão de ciclo de vida, em vez de serem tratados como dados comuns [1]. A separação de domínio é um controle de redução de acoplamento interno; ela não transforma a raiz em chave de hardware sem uma implementação de porta adequada.

## 4. Anti-rollback: o que é exigido da porta

Um tag AEAD prova que um blob não foi alterado sem a chave, mas **não** prova sozinho que ele é o mais novo. Um atacante que consiga restaurar um blob antigo e intacto pode reapresentá-lo com tag válida. Por isso, o contrato exige um `generation_floor` durável, separado do blob substituível.

| Operação | Ordem obrigatória | Resultado em falha |
|---|---|---|
| Inicialização | Ler piso durável antes de aceitar registros | Estado `BLOCKED` se a leitura falha. |
| Selagem | Cifrar geração `floor + 1`; armazenar blob; confirmar novo piso | Estado `BLOCKED` se blob ou confirmação do piso falhar. |
| Abertura | Exigir `blob.generation == floor` antes de decifrar | `E_ROLLBACK` e `BLOCKED` se a geração divergir. |
| Apagamento | Remover blob sem diminuir piso | `BLOCKED` se erase falhar. |

O backend tem de rejeitar reduções de piso e tornar a confirmação durável antes de retornar sucesso. O simulador RAM testa esse contrato lógico; ele não fornece persistência resistente a queda de energia nem a um adversário físico. Em produção, esta âncora precisa ser mapeada para um mecanismo revisado do alvo e testada contra power-loss, reflash, rollback de partição e reprovisionamento.

## 5. Ameaças tratadas e não tratadas

O cofre deliberadamente prefere indisponibilidade à aceitação de estado cujo histórico não pode ser demonstrado. O estado `BLOCKED` impede operações posteriores até que um ciclo controlado de inicialização/recuperação seja decidido por software de produto e validado em hardware; este passo não inventa um mecanismo mágico de recuperação.

| Evento adversarial | Resultado provado pela suíte host | Limite da prova |
|---|---|---|
| Autorização ausente ou não canônica | Escrita rejeitada antes de persistir. | Não prova que a UX física produz recibos corretos. |
| Sinal sensível ou de terceiro | Escrita rejeitada mesmo se há autorização. | Não classifica fala real; recebe sinal tipado já produzido antes. |
| Alteração de tag | AEAD falha, saída é zeroizada e cofre bloqueia. | Não mede ataques físicos de canal lateral. |
| Alteração de AAD/cabeçalho | Contexto inválido falha antes de devolver cartão. | Não protege um backend que ignore a porta. |
| Raiz diferente | Tag não autentica e nenhum cartão é devolvido. | Não prova proteção de raiz em silício. |
| Blob antigo válido | Divergência com piso gera `E_ROLLBACK`. | Depende de piso realmente durável e não redutível. |
| Falha de store ou erase | Cofre bloqueia; não afirma sucesso. | Não prova atomicidade de flash sob falta de energia. |

O código não tenta mitigar comprometimento total de firmware, extração física sem Secure Boot/Flash Encryption, depuração habilitada, ataques de canal lateral, corrupção do backend que também reduza o piso, perda da raiz, deduplicação de muitos cartões ou busca semântica. Esses riscos serão tratados por integração de plataforma, consolidação e avaliação de hardware; eles não são resolvidos por uma suíte host.

## 6. Caminho ESP32-S3, sem alegação antecipada

A documentação do ESP-IDF descreve NVS Encryption para ESP32-S3, incluindo um esquema que deriva chaves de armazenamento a partir de uma chave HMAC em eFuse, sem armazenar chaves XTS em flash [2]. A mesma documentação de segurança recomenda Secure Boot, Flash Encryption, chave única por dispositivo, restrições de debug e armazenamento NVS cifrado para dados confidenciais [3]. Esses recursos são candidatos de backend para as portas do cofre, **não** uma característica já integrada ou medida pelo HERUS.

| Gate antes de alegar “cofre protegido em hardware” | Evidência necessária |
|---|---|
| Backend ESP-IDF | Implementação revisada de blob e piso, com tratamento de falha/atomicidade. |
| Proteção de raiz | Decisão e registro de provisão: eFuse HMAC, secure element ou mecanismo equivalente. |
| Inicialização confiável | Secure Boot, configuração de Flash/NVS Encryption e interface de debug conforme política de produção. |
| Anti-rollback real | Testes em dispositivo para power-loss, rollback de partição e reflash. |
| Apagamento real | Procedimento que documente o que é removido e os limites de remanência da mídia. |
| Medição | Relatório de bancada separado, com versão de hardware/ESP-IDF e critérios de aceitação. |

## 7. Prova executável

O alvo `make memory-vault` compila em C11 estrito e executa uma porta RAM de teste. A suíte é incorporada ao `./prove.sh --quiet` como a **10ª de 21 suítes**. Ela cobre autorização ausente, candidato sensível, round-trip válido, adulteração de tag, adulteração de cabeçalho/AAD, raiz errada, rollback contra piso independente, chave por geração, falha de erase e falha de store.

```bash
cd firmware
make memory-vault
cd ..
./prove.sh --quiet
```

Um resultado positivo demonstra que o código host cumpre esses contratos sob os cenários injetados. Ele não mede alcance, consumo, WER, latência de modelo, resistência a extração física ou segurança de hardware.

## 8. Continuidade: consolidação humana implementada em host

O Passo 5 agora implementa a [consolidação humana](21-CONSOLIDACAO-HUMANA.md): proposta transitória, revisão física limitada, expiração sem retenção, conflito sem resolução automática, emissão disciplinada de recibo, recuperação por identificador e remoção controlada. O módulo continua sem texto livre, busca semântica, rede, LLM ou autoridade autônoma.

O próximo passo poderá investigar recuperação semântica controlada. Mesmo nessa etapa futura, uma LLM permanece uma fonte de sugestão; confirmação humana, consolidação e cofre continuam sendo os únicos caminhos de retenção.

## Referências

[1] National Institute of Standards and Technology, *Recommendation for Key Management: Part 1 — General*, SP 800-57 Part 1 Rev. 5, maio de 2020. [Página oficial](https://csrc.nist.gov/pubs/sp/800/57/pt1/r5/final) e [DOI](https://doi.org/10.6028/NIST.SP.800-57pt1r5).

[2] Espressif Systems, *NVS Encryption — ESP32-S3, ESP-IDF Programming Guide*. [Documentação oficial](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/storage/nvs_encryption.html).

[3] Espressif Systems, *Security Overview — ESP32-S3, ESP-IDF Programming Guide*. [Documentação oficial](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/security/security.html).
