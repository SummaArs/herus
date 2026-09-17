# Achados verificados — pesquisa sexto sentido soberano — 2026-09-17

## Haptics

A revisão de van Wegen et al. (2023) identifica 13 estudos com testes de usuários de dispositivos hápticos vestíveis em sujeitos saudáveis. O trabalho mostra que feedback háptico pode representar propriedades de objetos, mas exige soluções tecnológicas complexas e testes reais em ambientes de uso. A revisão também aponta que treinamento do usuário pode ser necessário para melhorar a interpretação de vibrações.
Fonte: https://pmc.ncbi.nlm.nih.gov/articles/PMC9919508/

## Privacidade de wearables

A análise sistemática viva de Doherty et al. (2025) avaliou políticas de 17 fabricantes em 24 critérios e sete dimensões. Riscos altos foram frequentes em transparência de relatórios e divulgação de vulnerabilidades. O resultado é relevante para o HERUS: wearables comerciais não devem ser tratados como padrão suficiente de soberania, privacidade ou auditabilidade.
Fonte: https://pmc.ncbi.nlm.nih.gov/articles/PMC12167361/

## Interoperabilidade

A arquitetura W3C Web of Things define descrições legíveis por máquina, affordances e intermediários que podem representar dispositivos físicos e compor Things virtuais. O padrão é uma base de descoberta e interoperabilidade, não uma política completa de autoridade. O HERUS pode usar descrições de capacidade como evidência de descoberta, mas precisa adicionar seu contrato próprio de pré-condições, efeitos permitidos e aprovação humana.
Fonte: https://www.w3.org/TR/wot-architecture11/

## Robótica

ROS 2 fornece autenticação, criptografia, integridade e controle de acesso por meio de DDS-Security, com enclaves, certificados e permissões. A documentação informa que a segurança pode ficar desligada por padrão e que uma configuração inválida pode permitir inicialização sem segurança, salvo estratégia Enforce. Para o HERUS, descoberta de um grafo ROS não deve implicar autorização; o modo fail-closed precisa ser obrigatório no adaptador.
Fonte: https://docs.ros.org/en/humble/Concepts/Intermediate/About-Security.html

## Zero trust

O NIST SP 800-207 define zero trust como ausência de confiança implícita baseada em localização de rede ou propriedade. Autenticação e autorização devem ser funções distintas antes do acesso a recursos. Isso reforça a separação HERUS entre identidade, capacidade observada, proposta e autorização externa.
Fonte: https://csrc.nist.gov/pubs/sp/800/207/final

## Wearables de uso crítico

O NISTIR 8235 trata dispositivos móveis e vestíveis de primeiros respondedores como capazes de ampliar operações, mas também de introduzir vulnerabilidades no ambiente de trabalho. O objetivo da orientação é ajudar organizações a selecionar e projetar dispositivos seguros. Para o HERUS, o pulso deve ser uma interface de assurance com requisitos de ciclo de vida, não apenas um acessório de notificações.
Fonte: https://www.nist.gov/publications/security-guidance-first-responder-mobile-and-wearable-devices

## Consequências preliminares para HERUS

1. A comunicação háptica precisa de um vocabulário treinável, testes de confusão e confirmação explícita para ações de alto risco.
2. A soberania precisa ser verificável por armazenamento local, exportação de logs, minimização de dados, desligamento físico e ausência de dependência obrigatória de nuvem.
3. W3C WoT é candidato a camada de descrição de capacidades; não deve substituir o HIR e o verificador de autoridade.
4. ROS 2 é candidato a adaptador de robótica; segurança de transporte não equivale à autorização HERUS.
5. Zero trust e capability-based security devem ser incorporados no contrato de host e no protocolo de handoff.
6. A pesquisa disponível apoia a necessidade de experimentos, mas não comprova que um pulso possa controlar qualquer tecnologia. Essa alegação requer testes de interoperabilidade, segurança e fatores humanos por domínio.
