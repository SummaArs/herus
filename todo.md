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
