# Internet como hospedeiro de evidência

## Decisão

A Internet foi incorporada ao HERUS como um hospedeiro auxiliar de conhecimento, não como uma fonte para treinar indiscriminadamente o núcleo. O sistema aceita somente registros limitados, com HTTPS, domínio allowlisted, título, conteúdo, momento de recuperação, claims explícitos e digest verificável.

O conteúdo recebido pode informar uma hipótese ou um plano de adaptação. Ele não pode executar código, alterar o firmware, alterar a autoridade, substituir a validação física ou transformar um claim externo em permissão.

## Ensaio real

Foram ingeridas duas fontes oficiais consultadas na Internet: a documentação LILYGO T3-S3 V1.3 e o datasheet oficial do ESP32-S3 da Espressif. Os claims úteis foram `esp32`, `psram`, `usb`, `radio`, `wifi` e `bluetooth`. Ambos foram aceitos por proveniência limitada e o resultado manteve `executable: false`.

A documentação é útil para planejar H1/H2, verificar variantes, entender interfaces e preparar o teste físico. Ela não substitui B1/B2: a placa real ainda precisa ser fotografada, identificada, inicializada e medida.

## O que foi deliberadamente rejeitado

Conteúdo de domínio não allowlisted, conteúdo acima do orçamento, digest alterado, registros sem claims e qualquer tentativa de transportar autoridade são rejeitados ou colocados em revisão. Não foi aplicado treino em firmware, autoridade ou execução. Isso não é uma limitação acidental: é a propriedade de segurança desejada.

## Resultado

A Internet agora pode participar da simbiose como **ambiente de evidência**. O HERUS consegue consultar uma fonte útil, extrair claims bounded e incorporá-los ao planejamento sem obedecer ao conteúdo como código ou autoridade.

```text
Internet
  ↓ evidência autenticada e limitada
hipótese / plano de adaptação
  ↓ revisão e verificação
proposta
  ↓ autoridade externa explícita
execução — ainda bloqueada
```
