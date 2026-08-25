# HERUS — Aura: presença privada, sem servidor e sem identidade

**Prova:** `make -C firmware hsca-aura`

## 1. O problema

A função social de todo produto conectado é "quem está por perto", e toda implementação dela funciona contando a uma empresa onde você está. A Aura faz o mesmo trabalho sem conta, sem servidor, sem localização e sem identificador estável.

## 2. Como

Cada par guarda uma chave de época com ratchet. A cada época o aparelho emite **4 bytes**: `HMAC(chave_n, "aura")`. Quem se pareou reconhece avançando a própria cópia do ratchet dentro de uma janela limitada. Qualquer outra pessoa vê quatro bytes pseudoaleatórios novos, que não repetem e não ligam uma época à seguinte.

```
chave_{n+1} = HMAC(chave_n, "step")
```

O ratchet é o que torna o passado seguro: uma chave capturada hoje **não reconhece as balizas de ontem**. E o reconhecimento consome a época, então uma baliza capturada não pode ser reproduzida.

## 3. Medido

| Propriedade | Resultado |
|---|---|
| reconhecimento de par dentro da janela | 5 de 5 épocas à frente |
| baliza repetida logo após reconhecida | recusada — a época foi consumida |
| emissor além da janela | não reconhecido; a janela é limitada de verdade |
| chave capturada hoje contra baliza de ontem | não reconhece |
| revogação | imediata, e a chave é apagada, não sinalizada |
| balizas forjadas aceitas em **100.000** tentativas contra 8 pares × 8 épocas | **0** (esperado por acaso: 0,0019) |
| balizas repetidas em 1.024 épocas consecutivas | 0 |
| densidade de bits das balizas | 0,5022 |
| registro de par | chave, contador de época e um flag — sem nome, sem lugar |

## 4. Limites honestos

- Quatro bytes dão 2⁻³² de falso positivo por candidato por tentativa. A suíte **reporta a contagem medida** contra um número declarado de tentativas; não alega zero por construção.
- "Não ligável" aqui é a propriedade observável de que as balizas são distintas e imprevisíveis sem a chave. Não é prova criptográfica de indistinguibilidade.
- Nada disso protege contra impressão digital de rádio, que é questão de hardware.
- **Custo real:** reconhecer uma baliza custa até 8 pares × 8 épocas = 64 HMAC-SHA256. Em ESP32-S3 isso precisa ser medido; se doer, a janela encolhe.

## 5. O que isto não é

- Não é localização, não é rastreamento e não emite posição.
- Não é presença como serviço: não existe servidor, diretório ou lista.
- Não é autorização. Reconhecer alguém por perto não autoriza enviar, guardar ou agir.
