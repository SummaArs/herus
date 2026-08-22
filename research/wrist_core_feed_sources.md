# Fontes para o contrato HERUS on-wrist / Core feed

## RFC 9019 — arquitetura de atualização IoT

URL: https://www.rfc-editor.org/rfc/rfc9019

A arquitetura define manifesto como metadado protegido que descreve o payload e o autor, e trata o dispositivo receptor como responsável por analisar e verificar o manifesto. O documento também destaca que a arquitetura de manifesto pode ser aplicada a dados arbitrários, como configurações e chaves, não somente firmware. Para o HERUS, isso fundamenta um envelope de feed versionado e verificável, mas não autoriza reutilizar a autoridade de firmware para incorporar memória pessoal.

Princípios aproveitados: integridade/autenticidade do manifesto; aplicabilidade ao dispositivo; separação entre produtor, servidor e consumidor; recuperação e rejeição como partes do ciclo; ausência do servidor não deve ser confundida com ausência de inteligência local.

## IETF SUIT Manifest draft-37

URL: https://datatracker.ietf.org/doc/html/draft-ietf-suit-manifest-37

O draft descreve um manifesto CBOR para dados/código, com envelope, bloco de autenticação, metadados críticos, sequência, digest e comandos. Ele explicita a ordem de verificar assinatura, aplicabilidade e payload antes de instalação, e distingue condições sem efeitos colaterais de diretivas com efeitos colaterais. É um Internet-Draft ativo, portanto não será tratado como padrão final no HERUS.

Princípios aproveitados: compatibilidade antes de processamento; digest e versão; parser limitado; condições sem efeito colateral; rejeição antes de incorporação; separar metadados do conteúdo.

## Uptane Standard 2.1.0

URL: https://uptane.org/docs/latest/standard/uptane-standard

O padrão define papéis Root, Targets, Snapshot e Timestamp, delegação de metadados e defesas contra rollback, freeze, mix-and-match e bundles parciais. O HERUS não precisa copiar Uptane nem adotar sua topologia de repositórios; a lição útil é separar quem produz, quem autoriza e quem verifica, além de não tratar um único pacote assinado como prova de toda a cadeia.

## in-toto Attestation Framework

URL: https://github.com/in-toto/attestation

O framework define atestações verificáveis sobre a origem/produção de software. Ele é relevante para proveniência de pacotes do Core, mas a atestação de origem não transforma o conteúdo em verdade pessoal: no HERUS, proveniência, validade semântica e confirmação do usuário continuarão sendo gates distintos.

## Decisão própria do HERUS

O feed de conhecimento não será firmware update e não será chamada para uma LLM. Será um pacote de proposta de conhecimento local. O HERUS verificará sintaxe, versão de registry, namespaces, handles, digest, orçamento, compatibilidade e proveniência antes de apresentar ou incorporar qualquer item. Pacote do Core não pode executar ação, enviar rádio, criar memória pessoal sem confirmação, alterar trust anchors ou substituir o reasoner local.
