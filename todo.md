# HERUS — Interface háptica LRA

- [ ] Fixar que o atuador nunca concede autoridade nem executa intenção sozinho.
- [ ] Definir padrões táteis finitos para confirmação, atenção, espera, recusa, bloqueio e emergência.
- [ ] Definir duração, repetição, intensidade e prioridade de cada padrão.
- [ ] Selecionar uma LRA e um driver háptico I2C compatível com ESP32-S3.
- [ ] Definir alimentação, desacoplamento, montagem e limites de consumo.
- [ ] Implementar simulador proposal-only dos padrões.
- [ ] Integrar o driver ao firmware somente após B1/B2.
- [ ] Medir reconhecimento humano, consumo e falhas do atuador.
- [ ] Publicar o contrato háptico e suas limitações.

# HERUS — Descoberta autônoma de hospedeiros

- [ ] Definir um protocolo de sondagem que não entregue o HostProfile completo ao HERUS.
- [ ] Definir capacidades observáveis, capacidades ocultas, erros e limites de cada hospedeiro.
- [ ] Construir pelo menos três hospedeiros de teste estruturalmente diferentes.
- [ ] Implementar um oráculo independente para verificar respostas das sondagens.
- [ ] Medir precisão do perfil descoberto, custo de sondagem e falsos positivos.
- [ ] Bloquear uma Skill quando a capacidade não for comprovada, mesmo que pareça plausível.
- [ ] Forçar renegociação após deriva ou mudança de revisão.
- [ ] Demonstrar transferência para um hospedeiro não usado durante a síntese.
- [ ] Repetir o protocolo no hospedeiro físico de pulso após B1/B2.
- [ ] Publicar resultados, falhas e limites sem declarar adaptação geral prematuramente.

# HERUS — Máximo host-only antes do hardware

- [ ] Auditar o que já é evidência, hipótese, simulação e lacuna física.
- [ ] Definir critérios para não chamar um ambiente fechado de adaptação geral.
- [ ] Criar hospedeiros virtuais abertos com interfaces, custos e capacidades desconhecidas.
- [ ] Implementar descoberta contínua com mudanças durante a sessão.
- [ ] Implementar memória de experimentos, crenças e contraexemplos.
- [ ] Fazer o HERUS escolher quais sondagens e experimentos executar sob orçamento.
- [ ] Compor e sintetizar Skills para objetivos novos em hospedeiros diferentes.
- [ ] Testar transferência aberta, deriva, conflito, falha e comportamento hostil.
- [ ] Medir custo, latência, cobertura, abstinência e generalização.
- [ ] Registrar o ponto exato em que somente hardware físico pode responder.

# HERUS — Simbiose entre humanos, máquinas e ambientes

- [ ] Formalizar os modos: hospedeiro simbiótico, guardião pessoal, rede defensiva e conector de domínio.
- [ ] Definir o núcleo de permissões: descobrir não autoriza, conectar não confia, proposta não executa.
- [ ] Definir contratos de USB, BLE, LoRa, Wi-Fi, UART, I2C, SPI, CAN e RS-485.
- [ ] Implementar descoberta defensiva autorizada de conexões Bluetooth.
- [ ] Detectar pareamento inesperado, mudança de identidade, deriva e comportamento anômalo.
- [ ] Definir quarentena, revogação, permitir uma vez e permitir por sessão.
- [ ] Mapear estados de segurança para padrões LRA proposal-only.
- [ ] Simular adaptação a robô legado sem acionar motores.
- [ ] Testar finanças em sandbox e sistemas críticos em shadow mode.
- [ ] Integrar canais externos somente após o contrato de permissão passar nos testes adversariais.
- [ ] Publicar a visão de simbiose, resultados e limites de uso real.

# HERUS — ASA aberto e primeiro hospedeiro físico

- [ ] Reformular os modos anteriores como cenários, não como limites fechados da arquitetura.
- [ ] Definir o protocolo universal de descoberta para qualquer hospedeiro observável.
- [ ] Representar lacunas de conhecimento como perguntas finitas e verificáveis.
- [ ] Definir o celular e o notebook como hospedeiros auxiliares e pontes de conhecimento.
- [ ] Tratar a Internet como ambiente externo não confiável, com proveniência e escopo.
- [ ] Impedir envio silencioso de segredos, dados pessoais ou autoridade para serviços externos.
- [ ] Permitir aquisição de evidência sem executar código externo automaticamente.
- [ ] Sintetizar Skills novas somente após verificador independente, sandbox e orçamento.
- [ ] Preparar o bring-up B1/B2 do primeiro hardware físico.
- [ ] Medir latência, memória, energia, rádio, perdas, reinicialização e deriva no hospedeiro real.
- [ ] Integrar feedback LRA real somente depois dos gates de segurança do firmware.
- [ ] Repetir descoberta e renegociação com o ASA conectado a celular ou notebook.
- [ ] Publicar o que foi aprendido, o que falhou e o que continua não demonstrado.

# HERUS — Reorganização editorial e narrativa atual

- [ ] Auditar o README antigo e identificar afirmações que já não representam o projeto.
- [ ] Definir a separação entre produto pessoal, Paper-Core, ASA, firmware e evidência de pesquisa.
- [ ] Reescrever o README com a proposta de memória pessoal soberana, conversa local e comunicação essencial.
- [ ] Criar uma linha do tempo verificável das principais evoluções do HERUS.
- [ ] Consolidar links para documentos normativos, evidências, testes e hardware.
- [ ] Remover linguagem de AGI ou capacidades não demonstradas da narrativa pública.
- [ ] Corrigir referências cruzadas antigas e títulos inconsistentes.
- [ ] Executar a suíte completa e publicar a organização no GitHub.
