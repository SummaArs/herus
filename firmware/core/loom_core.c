/* loom_core.c — GERADO por tools/loom.py. NAO EDITE.
 * Fonte: research/loom/core/  (todos os .hlx.json)   digest 1971b64c99487a133a5f8e79883cbe70
 */
#include "loom_core.h"
#include <string.h>

const loom_entry_t LOOM_ENTRY[LOOM_ENTRY_COUNT] = {
    { "0", 1, 2, 5, 1024 },  /* pt QTY.N_0 */
    { "10 minutos", 10, 2, 3, 770 },  /* pt TIME.MIN_10 */
    { "15 minutos", 10, 2, 3, 777 },  /* pt TIME.MIN_15 */
    { "1 minuto", 8, 2, 3, 776 },  /* pt TIME.MIN_1 */
    { "1 hora", 6, 2, 3, 772 },  /* pt TIME.HORA_1 */
    { "100", 3, 2, 5, 1033 },  /* pt QTY.N_100 */
    { "10", 2, 2, 5, 1030 },  /* pt QTY.N_10 */
    { "1", 1, 2, 5, 1025 },  /* pt QTY.N_1 */
    { "2 horas", 7, 2, 3, 778 },  /* pt TIME.HORA_2 */
    { "20", 2, 2, 5, 1031 },  /* pt QTY.N_20 */
    { "2", 1, 2, 5, 1026 },  /* pt QTY.N_2 */
    { "30 minutos", 10, 2, 3, 771 },  /* pt TIME.MIN_30 */
    { "3", 1, 2, 5, 1027 },  /* pt QTY.N_3 */
    { "4", 1, 2, 5, 1028 },  /* pt QTY.N_4 */
    { "5 minutos", 9, 2, 3, 769 },  /* pt TIME.MIN_5 */
    { "50", 2, 2, 5, 1032 },  /* pt QTY.N_50 */
    { "5", 1, 2, 5, 1029 },  /* pt QTY.N_5 */
    { "acabei de chegar", 16, 2, 2, 1 },  /* pt EV.CHEGUEI */
    { "automaticamente", 15, 7, 0, 0 },  /* pt authority */
    { "acampamento", 11, 2, 4, 524 },  /* pt LOC.ACAMPAMENTO */
    { "acompanhado", 11, 2, 6, 1288 },  /* pt ST.ACOMPANHADO */
    { "algo errado", 11, 2, 2, 59 },  /* pt EV.ALGO_ERRADO */
    { "aguardando", 10, 2, 2, 6 },  /* pt EV.ESPERANDO */
    { "ambulancia", 10, 2, 2, 29 },  /* pt EV.MEDICO */
    { "a caminho", 9, 2, 2, 8 },  /* pt EV.A_CAMINHO */
    { "aeroporto", 9, 2, 4, 521 },  /* pt LOC.AEROPORTO */
    { "academia", 8, 2, 4, 515 },  /* pt LOC.TREINO */
    { "acidente", 8, 2, 2, 26 },  /* pt EV.ACIDENTE */
    { "acordado", 8, 2, 2, 38 },  /* pt EV.ACORDEI */
    { "amo voce", 8, 2, 2, 17 },  /* pt EV.TE_AMO */
    { "atrasado", 8, 2, 2, 3 },  /* pt EV.ATRASADO */
    { "a noite", 7, 2, 3, 775 },  /* pt TIME.NOITE */
    { "a tarde", 7, 2, 3, 780 },  /* pt TIME.TARDE_DIA */
    { "acordei", 7, 2, 2, 38 },  /* pt EV.ACORDEI */
    { "atencao", 7, 2, 2, 22 },  /* pt EV.CUIDADO */
    { "abrigo", 6, 2, 2, 33 },  /* pt EV.ABRIGO */
    { "acabei", 6, 2, 2, 9 },  /* pt EV.TERMINEI */
    { "amanha", 6, 2, 3, 774 },  /* pt TIME.AMANHA */
    { "anotar", 6, 1, 0, 3 },  /* pt op LEMBRAR */
    { "avisar", 6, 1, 0, 1 },  /* pt op COMUNICAR */
    { "achei", 5, 2, 2, 45 },  /* pt EV.ACHEI */
    { "agora", 5, 2, 3, 768 },  /* pt TIME.AGORA */
    { "ajuda", 5, 2, 2, 5 },  /* pt EV.AJUDA */
    { "amiga", 5, 2, 1, 265 },  /* pt PER.AMIGO */
    { "amigo", 5, 2, 1, 265 },  /* pt PER.AMIGO */
    { "anota", 5, 1, 0, 3 },  /* pt op LEMBRAR */
    { "audio", 5, 6, 0, 0 },  /* pt sensitive */
    { "avisa", 5, 1, 0, 1 },  /* pt op COMUNICAR */
    { "avise", 5, 1, 0, 1 },  /* pt op COMUNICAR */
    { "agua", 4, 2, 2, 31 },  /* pt EV.AGUA */
    { "aqui", 4, 0, 0, 0 },  /* pt stop */
    { "ana", 3, 2, 1, 258 },  /* pt PER.ANA */
    { "aos", 3, 0, 0, 0 },  /* pt stop */
    { "ate", 3, 0, 0, 0 },  /* pt stop */
    { "ao", 2, 0, 0, 0 },  /* pt stop */
    { "as", 2, 0, 0, 0 },  /* pt stop */
    { "a", 1, 0, 0, 0 },  /* pt stop */
    { "bateria acabando", 16, 2, 2, 10 },  /* pt EV.SEM_BATERIA */
    { "biometria", 9, 6, 0, 0 },  /* pt sensitive */
    { "boa noite", 9, 2, 2, 21 },  /* pt EV.BOA_NOITE */
    { "buscando", 8, 2, 2, 46 },  /* pt EV.PROCURANDO */
    { "bom dia", 7, 2, 2, 20 },  /* pt EV.BOM_DIA */
    { "bateu", 5, 2, 2, 26 },  /* pt EV.ACIDENTE */
    { "base", 4, 2, 4, 527 },  /* pt LOC.BASE */
    { "bem", 3, 2, 6, 1280 },  /* pt ST.BEM */
    { "chama a policia", 15, 2, 2, 28 },  /* pt EV.POLICIA */
    { "carro quebrou", 13, 2, 2, 43 },  /* pt EV.VEICULO_QUEBROU */
    { "cinco minutos", 13, 2, 3, 769 },  /* pt TIME.MIN_5 */
    { "compra feita", 12, 2, 2, 47 },  /* pt EV.COMPREI */
    { "chegou aqui", 11, 2, 2, 50 },  /* pt EV.RECEBI */
    { "coordenadas", 11, 6, 0, 0 },  /* pt sensitive */
    { "com pressa", 10, 4, 0, 1 },  /* pt urgency ATENCAO */
    { "coordenada", 10, 6, 0, 0 },  /* pt sensitive */
    { "cancelado", 9, 2, 2, 53 },  /* pt EV.CANCELADO */
    { "cinquenta", 9, 2, 5, 1032 },  /* pt QTY.N_50 */
    { "confirmar", 9, 1, 0, 6 },  /* pt op CONFIRMAR */
    { "contato 1", 9, 2, 1, 270 },  /* pt PER.HANDLE_1 */
    { "contato 2", 9, 2, 1, 271 },  /* pt PER.HANDLE_2 */
    { "contato 3", 9, 2, 1, 272 },  /* pt PER.HANDLE_3 */
    { "contato 4", 9, 2, 1, 273 },  /* pt PER.HANDLE_4 */
    { "cancelar", 8, 1, 0, 7 },  /* pt op CANCELAR */
    { "chegando", 8, 2, 2, 8 },  /* pt EV.A_CAMINHO */
    { "com medo", 8, 2, 6, 1291 },  /* pt ST.COM_MEDO */
    { "comunica", 8, 1, 0, 1 },  /* pt op COMUNICAR */
    { "confirma", 8, 1, 0, 6 },  /* pt op CONFIRMAR */
    { "confirmo", 8, 1, 0, 6 },  /* pt op CONFIRMAR */
    { "contagem", 8, 2, 2, 60 },  /* pt EV.CONTAGEM */
    { "cancela", 7, 1, 0, 7 },  /* pt op CANCELAR */
    { "cansado", 7, 2, 2, 36 },  /* pt EV.CANSADO */
    { "cheguei", 7, 2, 2, 1 },  /* pt EV.CHEGUEI */
    { "com dor", 7, 2, 2, 30 },  /* pt EV.DOR */
    { "comecou", 7, 2, 2, 52 },  /* pt EV.COMECOU */
    { "comprei", 7, 2, 2, 47 },  /* pt EV.COMPREI */
    { "confuso", 7, 2, 2, 56 },  /* pt EV.NAO_ENTENDI */
    { "cuidado", 7, 2, 2, 22 },  /* pt EV.CUIDADO */
    { "cartao", 6, 6, 0, 0 },  /* pt sensitive */
    { "comida", 6, 2, 2, 32 },  /* pt EV.COMIDA */
    { "calmo", 5, 2, 6, 1290 },  /* pt ST.CALMO */
    { "calor", 5, 2, 2, 35 },  /* pt EV.CALOR */
    { "carro", 5, 2, 4, 523 },  /* pt LOC.VEICULO */
    { "chave", 5, 6, 0, 0 },  /* pt sensitive */
    { "chefe", 5, 2, 1, 266 },  /* pt PER.CHEFE */
    { "cinco", 5, 2, 5, 1029 },  /* pt QTY.N_5 */
    { "conta", 5, 1, 0, 1 },  /* pt op COMUNICAR */
    { "cade", 4, 5, 4, 0 },  /* pt qword ONDE */
    { "casa", 4, 2, 4, 512 },  /* pt LOC.CASA */
    { "cume", 4, 2, 4, 526 },  /* pt LOC.CUME */
    { "cem", 3, 2, 5, 1033 },  /* pt QTY.N_100 */
    { "com", 3, 0, 0, 0 },  /* pt stop */
    { "cpf", 3, 6, 0, 0 },  /* pt sensitive */
    { "desconsidere", 12, 7, 0, 0 },  /* pt authority */
    { "dez minutos", 11, 2, 3, 770 },  /* pt TIME.MIN_10 */
    { "duas horas", 10, 2, 3, 778 },  /* pt TIME.HORA_2 */
    { "despachei", 9, 2, 2, 49 },  /* pt EV.ENVIEI */
    { "de manha", 8, 2, 3, 779 },  /* pt TIME.MANHA */
    { "de tarde", 8, 2, 3, 780 },  /* pt TIME.TARDE_DIA */
    { "de volta", 8, 2, 2, 55 },  /* pt EV.VOLTANDO */
    { "desculpa", 8, 2, 2, 16 },  /* pt EV.DESCULPA */
    { "desculpe", 8, 2, 2, 16 },  /* pt EV.DESCULPA */
    { "dormindo", 8, 2, 2, 37 },  /* pt EV.DORMINDO */
    { "depois", 6, 2, 3, 782 },  /* pt TIME.DEPOIS */
    { "doendo", 6, 2, 2, 30 },  /* pt EV.DOR */
    { "dizer", 5, 1, 0, 1 },  /* pt op COMUNICAR */
    { "dois", 4, 2, 5, 1026 },  /* pt QTY.N_2 */
    { "dez", 3, 2, 5, 1030 },  /* pt QTY.N_10 */
    { "diz", 3, 1, 0, 1 },  /* pt op COMUNICAR */
    { "dor", 3, 2, 2, 30 },  /* pt EV.DOR */
    { "da", 2, 0, 0, 0 },  /* pt stop */
    { "de", 2, 0, 0, 0 },  /* pt stop */
    { "do", 2, 0, 0, 0 },  /* pt stop */
    { "estou esperando", 15, 2, 2, 6 },  /* pt EV.ESPERANDO */
    { "estacionamento", 14, 2, 4, 530 },  /* pt LOC.ESTACIONAMENTO */
    { "estou atrasado", 14, 2, 2, 3 },  /* pt EV.ATRASADO */
    { "estou perdido", 13, 2, 2, 11 },  /* pt EV.PERDIDO */
    { "estou saindo", 12, 2, 2, 2 },  /* pt EV.SAINDO */
    { "esta semana", 11, 2, 3, 783 },  /* pt TIME.SEMANA */
    { "em reuniao", 10, 2, 2, 40 },  /* pt EV.REUNIAO */
    { "emergencia", 10, 1, 0, 5 },  /* pt op SOCORRO */
    { "estou aqui", 10, 2, 2, 1 },  /* pt EV.CHEGUEI */
    { "estou indo", 10, 2, 2, 8 },  /* pt EV.A_CAMINHO */
    { "em perigo", 9, 2, 6, 1283 },  /* pt ST.EM_PERIGO */
    { "embedding", 9, 6, 0, 0 },  /* pt sensitive */
    { "encontrar", 9, 2, 2, 7 },  /* pt EV.ENCONTRAR */
    { "encontrei", 9, 2, 2, 45 },  /* pt EV.ACHEI */
    { "esperando", 9, 2, 2, 6 },  /* pt EV.ESPERANDO */
    { "em breve", 8, 2, 3, 785 },  /* pt TIME.EM_BREVE */
    { "encontro", 8, 2, 2, 7 },  /* pt EV.ENCONTRAR */
    { "entrada", 7, 2, 4, 529 },  /* pt LOC.ENTRADA */
    { "estacao", 7, 2, 4, 522 },  /* pt LOC.ESTACAO */
    { "estavel", 7, 2, 6, 1286 },  /* pt ST.ESTAVEL */
    { "estrada", 7, 2, 4, 516 },  /* pt LOC.ESTRADA */
    { "estudar", 7, 2, 2, 12 },  /* pt EV.ESTUDAR */
    { "exausto", 7, 2, 2, 36 },  /* pt EV.CANSADO */
    { "enviei", 6, 2, 2, 49 },  /* pt EV.ENVIEI */
    { "equipe", 6, 2, 1, 259 },  /* pt PER.EQUIPE */
    { "escola", 6, 2, 4, 518 },  /* pt LOC.ESCOLA */
    { "espera", 6, 2, 2, 25 },  /* pt EV.ESPERA */
    { "espere", 6, 2, 2, 25 },  /* pt EV.ESPERA */
    { "estudo", 6, 2, 2, 12 },  /* pt EV.ESTUDAR */
    { "estou", 5, 0, 0, 0 },  /* pt stop */
    { "esse", 4, 0, 0, 0 },  /* pt stop */
    { "esta", 4, 0, 0, 0 },  /* pt stop */
    { "em", 2, 0, 0, 0 },  /* pt stop */
    { "eu", 2, 0, 0, 0 },  /* pt stop */
    { "e", 1, 0, 0, 0 },  /* pt stop */
    { "fim de semana", 13, 2, 3, 784 },  /* pt TIME.FIM_DE_SEMANA */
    { "foi cancelado", 13, 2, 2, 53 },  /* pt EV.CANCELADO */
    { "felicidades", 11, 2, 2, 19 },  /* pt EV.PARABENS */
    { "faculdade", 9, 2, 4, 518 },  /* pt LOC.ESCOLA */
    { "finalizei", 9, 2, 2, 9 },  /* pt EV.TERMINEI */
    { "familia", 7, 2, 1, 260 },  /* pt PER.FAMILIA */
    { "foi mal", 7, 2, 2, 16 },  /* pt EV.DESCULPA */
    { "ferido", 6, 2, 6, 1284 },  /* pt ST.FERIDO */
    { "fumaca", 6, 2, 2, 27 },  /* pt EV.FOGO */
    { "falar", 5, 1, 0, 1 },  /* pt op COMUNICAR */
    { "favor", 5, 0, 0, 0 },  /* pt stop */
    { "filha", 5, 2, 1, 264 },  /* pt PER.FILHO */
    { "filho", 5, 2, 1, 264 },  /* pt PER.FILHO */
    { "fala", 4, 1, 0, 1 },  /* pt op COMUNICAR */
    { "fogo", 4, 2, 2, 27 },  /* pt EV.FOGO */
    { "fome", 4, 2, 2, 32 },  /* pt EV.COMIDA */
    { "frio", 4, 2, 2, 34 },  /* pt EV.FRIO */
    { "gravacoes", 9, 6, 0, 0 },  /* pt sensitive */
    { "gravacao", 8, 6, 0, 0 },  /* pt sensitive */
    { "grupo 1", 7, 2, 1, 274 },  /* pt PER.GRUPO_1 */
    { "grupo 2", 7, 2, 1, 275 },  /* pt PER.GRUPO_2 */
    { "guardar", 7, 1, 0, 3 },  /* pt op LEMBRAR */
    { "guarda", 6, 1, 0, 3 },  /* pt op LEMBRAR */
    { "guarde", 6, 1, 0, 3 },  /* pt op LEMBRAR */
    { "geral", 5, 2, 1, 261 },  /* pt PER.TODOS */
    { "grave", 5, 2, 6, 1285 },  /* pt ST.GRAVE */
    { "guia", 4, 2, 1, 267 },  /* pt PER.GUIA */
    { "gps", 3, 6, 0, 0 },  /* pt sensitive */
    { "houve acidente", 14, 2, 2, 26 },  /* pt EV.ACIDENTE */
    { "hoje a noite", 12, 2, 3, 775 },  /* pt TIME.NOITE */
    { "hospital", 8, 2, 4, 519 },  /* pt LOC.HOSPITAL */
    { "hoje", 4, 2, 3, 773 },  /* pt TIME.HOJE */
    { "impossivel", 10, 2, 2, 14 },  /* pt EV.IMPOSSIVEL */
    { "isso mesmo", 10, 2, 2, 13 },  /* pt EV.SIM */
    { "incendio", 8, 2, 2, 27 },  /* pt EV.FOGO */
    { "iniciou", 7, 2, 2, 52 },  /* pt EV.COMECOU */
    { "ignora", 6, 7, 0, 0 },  /* pt authority */
    { "ignore", 6, 7, 0, 0 },  /* pt authority */
    { "isso", 4, 0, 0, 0 },  /* pt stop */
    { "ja cheguei", 10, 2, 2, 1 },  /* pt EV.CHEGUEI */
    { "joao", 4, 2, 1, 256 },  /* pt PER.JOAO */
    { "ja", 2, 0, 0, 0 },  /* pt stop */
    { "liga pra mim", 12, 2, 2, 41 },  /* pt EV.LIGA_PRA_MIM */
    { "localizacao", 11, 6, 0, 0 },  /* pt sensitive */
    { "lembrar", 7, 1, 0, 3 },  /* pt op LEMBRAR */
    { "lembra", 6, 1, 0, 3 },  /* pt op LEMBRAR */
    { "logo", 4, 2, 3, 785 },  /* pt TIME.EM_BREVE */
    { "loja", 4, 2, 4, 520 },  /* pt LOC.MERCADO */
    { "la", 2, 0, 0, 0 },  /* pt stop */
    { "mudou o plano", 13, 2, 2, 54 },  /* pt EV.PLANO_MUDOU */
    { "mais tarde", 10, 2, 3, 782 },  /* pt TIME.DEPOIS */
    { "me espera", 9, 2, 2, 25 },  /* pt EV.ESPERA */
    { "meia hora", 9, 2, 3, 771 },  /* pt TIME.MIN_30 */
    { "me ajuda", 8, 2, 2, 5 },  /* pt EV.AJUDA */
    { "me ligue", 8, 2, 2, 41 },  /* pt EV.LIGA_PRA_MIM */
    { "me perdi", 8, 2, 2, 11 },  /* pt EV.PERDIDO */
    { "me liga", 7, 2, 2, 41 },  /* pt EV.LIGA_PRA_MIM */
    { "mercado", 7, 2, 4, 520 },  /* pt LOC.MERCADO */
    { "mandar", 6, 1, 0, 1 },  /* pt op COMUNICAR */
    { "mandei", 6, 2, 2, 49 },  /* pt EV.ENVIEI */
    { "medico", 6, 2, 2, 29 },  /* pt EV.MEDICO */
    { "metade", 6, 2, 5, 1036 },  /* pt QTY.METADE */
    { "manda", 5, 1, 0, 1 },  /* pt op COMUNICAR */
    { "mande", 5, 1, 0, 1 },  /* pt op COMUNICAR */
    { "manha", 5, 2, 3, 779 },  /* pt TIME.MANHA */
    { "maria", 5, 2, 1, 257 },  /* pt PER.MARIA */
    { "minha", 5, 0, 0, 0 },  /* pt stop */
    { "muito", 5, 2, 5, 1035 },  /* pt QTY.MUITO */
    { "mae", 3, 2, 1, 262 },  /* pt PER.MAE */
    { "mal", 3, 2, 6, 1281 },  /* pt ST.MAL */
    { "meu", 3, 0, 0, 0 },  /* pt stop */
    { "me", 2, 0, 0, 0 },  /* pt stop */
    { "nao posso falar", 15, 2, 2, 42 },  /* pt EV.NAO_POSSO_FALAR */
    { "nao entendi", 11, 2, 2, 56 },  /* pt EV.NAO_ENTENDI */
    { "nos vemos", 9, 2, 2, 7 },  /* pt EV.ENCONTRAR */
    { "nao da", 6, 2, 2, 14 },  /* pt EV.IMPOSSIVEL */
    { "nenhum", 6, 2, 5, 1024 },  /* pt QTY.N_0 */
    { "nao", 3, 3, 0, 0 },  /* pt negation */
    { "nos", 3, 0, 0, 0 },  /* pt stop */
    { "na", 2, 0, 0, 0 },  /* pt stop */
    { "no", 2, 0, 0, 0 },  /* pt stop */
    { "obrigada", 8, 2, 2, 15 },  /* pt EV.OBRIGADO */
    { "obrigado", 8, 2, 2, 15 },  /* pt EV.OBRIGADO */
    { "ontem", 5, 2, 3, 781 },  /* pt TIME.ONTEM */
    { "onde", 4, 5, 4, 0 },  /* pt qword ONDE */
    { "ola", 3, 2, 2, 20 },  /* pt EV.BOM_DIA */
    { "oi", 2, 2, 2, 20 },  /* pt EV.BOM_DIA */
    { "os", 2, 0, 0, 0 },  /* pt stop */
    { "o", 1, 0, 0, 0 },  /* pt stop */
    { "ponto de encontro", 17, 2, 4, 528 },  /* pt LOC.PONTO_ENCONTRO */
    { "por conta propria", 17, 7, 0, 0 },  /* pt authority */
    { "preciso de abrigo", 17, 2, 2, 33 },  /* pt EV.ABRIGO */
    { "preciso de comida", 17, 2, 2, 32 },  /* pt EV.COMIDA */
    { "preciso de ajuda", 16, 2, 2, 5 },  /* pt EV.AJUDA */
    { "perigo a frente", 15, 2, 2, 22 },  /* pt EV.CUIDADO */
    { "preciso de agua", 15, 2, 2, 31 },  /* pt EV.AGUA */
    { "pouca bateria", 13, 2, 2, 10 },  /* pt EV.SEM_BATERIA */
    { "preciso ajuda", 13, 2, 2, 5 },  /* pt EV.AJUDA */
    { "preciso falar", 13, 2, 2, 58 },  /* pt EV.PRECISO_FALAR */
    { "portao norte", 12, 2, 4, 514 },  /* pt LOC.PORTAO_N */
    { "plano novo", 10, 2, 2, 54 },  /* pt EV.PLANO_MUDOU */
    { "procurando", 10, 2, 2, 46 },  /* pt EV.PROCURANDO */
    { "pagamento", 9, 2, 2, 48 },  /* pt EV.PAGAMENTO */
    { "para tudo", 9, 2, 2, 23 },  /* pt EV.PARE */
    { "perguntar", 9, 1, 0, 2 },  /* pt op PERGUNTAR */
    { "parabens", 8, 2, 2, 19 },  /* pt EV.PARABENS */
    { "pergunta", 8, 1, 0, 2 },  /* pt op PERGUNTAR */
    { "planejar", 8, 1, 0, 4 },  /* pt op PLANEJAR */
    { "positivo", 8, 2, 2, 13 },  /* pt EV.SIM */
    { "perdido", 7, 2, 2, 11 },  /* pt EV.PERDIDO */
    { "planeja", 7, 1, 0, 4 },  /* pt op PLANEJAR */
    { "policia", 7, 2, 2, 28 },  /* pt EV.POLICIA */
    { "paguei", 6, 2, 2, 48 },  /* pt EV.PAGAMENTO */
    { "pronto", 6, 2, 2, 51 },  /* pt EV.PRONTO */
    { "pagar", 5, 2, 2, 48 },  /* pt EV.PAGAMENTO */
    { "parar", 5, 2, 2, 23 },  /* pt EV.PARE */
    { "pouco", 5, 2, 5, 1034 },  /* pt QTY.POUCO */
    { "preso", 5, 2, 6, 1289 },  /* pt ST.PRESO */
    { "pane", 4, 2, 2, 43 },  /* pt EV.VEICULO_QUEBROU */
    { "para", 4, 0, 0, 0 },  /* pt stop */
    { "pare", 4, 2, 2, 23 },  /* pt EV.PARE */
    { "pai", 3, 2, 1, 263 },  /* pt PER.PAI */
    { "por", 3, 0, 0, 0 },  /* pt stop */
    { "pra", 3, 0, 0, 0 },  /* pt stop */
    { "pro", 3, 0, 0, 0 },  /* pt stop */
    { "quinze minutos", 14, 2, 3, 777 },  /* pt TIME.MIN_15 */
    { "quantos somos", 13, 2, 2, 60 },  /* pt EV.CONTAGEM */
    { "que horas", 9, 5, 3, 0 },  /* pt qword QUANDO */
    { "quantas", 7, 5, 5, 0 },  /* pt qword QUANTO */
    { "quantos", 7, 5, 5, 0 },  /* pt qword QUANTO */
    { "quando", 6, 5, 3, 0 },  /* pt qword QUANDO */
    { "quatro", 6, 2, 5, 1028 },  /* pt QTY.N_4 */
    { "quem", 4, 5, 1, 0 },  /* pt qword QUEM */
    { "que", 3, 0, 0, 0 },  /* pt stop */
    { "resgate", 7, 2, 1, 268 },  /* pt PER.SOCORRISTA */
    { "reuniao", 7, 2, 2, 40 },  /* pt EV.REUNIAO */
    { "rapido", 6, 4, 0, 1 },  /* pt urgency ATENCAO */
    { "recebi", 6, 2, 2, 50 },  /* pt EV.RECEBI */
    { "repete", 6, 2, 2, 57 },  /* pt EV.REPETE */
    { "repita", 6, 2, 2, 57 },  /* pt EV.REPETE */
    { "rio", 3, 2, 4, 525 },  /* pt LOC.RIO */
    { "sem confirmacao", 15, 7, 0, 0 },  /* pt authority */
    { "sem poder falar", 15, 2, 2, 42 },  /* pt EV.NAO_POSSO_FALAR */
    { "sinto sua falta", 15, 2, 2, 18 },  /* pt EV.SAUDADE */
    { "sem confirmar", 13, 7, 0, 0 },  /* pt authority */
    { "sem problema", 12, 2, 2, 4 },  /* pt EV.TUDO_BEM */
    { "sem bateria", 11, 2, 2, 10 },  /* pt EV.SEM_BATERIA */
    { "sempre que", 10, 7, 0, 0 },  /* pt authority */
    { "socorrista", 10, 2, 1, 268 },  /* pt PER.SOCORRISTA */
    { "sem sinal", 9, 2, 2, 44 },  /* pt EV.SEM_SINAL */
    { "sem agua", 8, 2, 2, 31 },  /* pt EV.AGUA */
    { "sem rede", 8, 2, 2, 44 },  /* pt EV.SEM_SINAL */
    { "saudade", 7, 2, 2, 18 },  /* pt EV.SAUDADE */
    { "socorro", 7, 1, 0, 5 },  /* pt op SOCORRO */
    { "sozinho", 7, 7, 0, 0 },  /* pt authority */
    { "saindo", 6, 2, 2, 2 },  /* pt EV.SAINDO */
    { "seguro", 6, 2, 6, 1282 },  /* pt ST.SEGURO */
    { "senhas", 6, 6, 0, 0 },  /* pt sensitive */
    { "senha", 5, 6, 0, 0 },  /* pt sensitive */
    { "sobre", 5, 0, 0, 0 },  /* pt stop */
    { "sim", 3, 2, 2, 13 },  /* pt EV.SIM */
    { "se", 2, 0, 0, 0 },  /* pt stop */
    { "so", 2, 2, 6, 1287 },  /* pt ST.SOZINHO_ST */
    { "tem algo errado", 15, 2, 2, 59 },  /* pt EV.ALGO_ERRADO */
    { "temos que falar", 15, 2, 2, 58 },  /* pt EV.PRECISO_FALAR */
    { "to esperando", 12, 2, 2, 6 },  /* pt EV.ESPERANDO */
    { "to atrasado", 11, 2, 2, 3 },  /* pt EV.ATRASADO */
    { "trabalhando", 11, 2, 2, 39 },  /* pt EV.TRABALHANDO */
    { "transcricao", 11, 6, 0, 0 },  /* pt sensitive */
    { "tudo pronto", 11, 2, 2, 51 },  /* pt EV.PRONTO */
    { "to perdido", 10, 2, 2, 11 },  /* pt EV.PERDIDO */
    { "tudo certo", 10, 2, 2, 4 },  /* pt EV.TUDO_BEM */
    { "to saindo", 9, 2, 2, 2 },  /* pt EV.SAINDO */
    { "terminei", 8, 2, 2, 9 },  /* pt EV.TERMINEI */
    { "tou aqui", 8, 2, 2, 1 },  /* pt EV.CHEGUEI */
    { "trabalho", 8, 2, 4, 513 },  /* pt LOC.TRABALHO */
    { "tudo bem", 8, 2, 2, 4 },  /* pt EV.TUDO_BEM */
    { "to aqui", 7, 2, 2, 1 },  /* pt EV.CHEGUEI */
    { "to indo", 7, 2, 2, 8 },  /* pt EV.A_CAMINHO */
    { "te amo", 6, 2, 2, 17 },  /* pt EV.TE_AMO */
    { "treino", 6, 2, 4, 515 },  /* pt LOC.TREINO */
    { "trilha", 6, 2, 4, 517 },  /* pt LOC.TRILHA */
    { "tchau", 5, 2, 2, 21 },  /* pt EV.BOA_NOITE */
    { "tenho", 5, 0, 0, 0 },  /* pt stop */
    { "todos", 5, 2, 1, 261 },  /* pt PER.TODOS */
    { "total", 5, 2, 5, 1037 },  /* pt QTY.TOTAL */
    { "time", 4, 2, 1, 259 },  /* pt PER.EQUIPE */
    { "topo", 4, 2, 4, 526 },  /* pt LOC.CUME */
    { "tres", 4, 2, 5, 1027 },  /* pt QTY.N_3 */
    { "tem", 3, 0, 0, 0 },  /* pt stop */
    { "tou", 3, 0, 0, 0 },  /* pt stop */
    { "ta", 2, 0, 0, 0 },  /* pt stop */
    { "to", 2, 0, 0, 0 },  /* pt stop */
    { "um minuto", 9, 2, 3, 776 },  /* pt TIME.MIN_1 */
    { "uma hora", 8, 2, 3, 772 },  /* pt TIME.HORA_1 */
    { "urgente", 7, 4, 0, 2 },  /* pt urgency URGENTE */
    { "uma", 3, 0, 0, 0 },  /* pt stop */
    { "um", 2, 0, 0, 0 },  /* pt stop */
    { "veiculo quebrado", 16, 2, 2, 43 },  /* pt EV.VEICULO_QUEBROU */
    { "vou atrasar", 11, 2, 2, 3 },  /* pt EV.ATRASADO */
    { "vou dormir", 10, 2, 2, 37 },  /* pt EV.DORMINDO */
    { "voltando", 8, 2, 2, 55 },  /* pt EV.VOLTANDO */
    { "vou sair", 8, 2, 2, 2 },  /* pt EV.SAINDO */
    { "veiculo", 7, 2, 4, 523 },  /* pt LOC.VEICULO */
    { "vizinho", 7, 2, 1, 269 },  /* pt PER.VIZINHO */
    { "vem ca", 6, 2, 2, 24 },  /* pt EV.VEM */
    { "valeu", 5, 2, 2, 15 },  /* pt EV.OBRIGADO */
    { "venha", 5, 2, 2, 24 },  /* pt EV.VEM */
    { "vinte", 5, 2, 5, 1031 },  /* pt QTY.N_20 */
    { "voce", 4, 0, 0, 0 },  /* pt stop */
    { "vem", 3, 2, 2, 24 },  /* pt EV.VEM */
    { "vou", 3, 0, 0, 0 },  /* pt stop */
    { "vc", 2, 0, 0, 0 },  /* pt stop */
    { "0", 1, 2, 5, 1024 },  /* en QTY.N_0 */
    { "10 minutes", 10, 2, 3, 770 },  /* en TIME.MIN_10 */
    { "15 minutes", 10, 2, 3, 777 },  /* en TIME.MIN_15 */
    { "1 minute", 8, 2, 3, 776 },  /* en TIME.MIN_1 */
    { "1 hour", 6, 2, 3, 772 },  /* en TIME.HORA_1 */
    { "100", 3, 2, 5, 1033 },  /* en QTY.N_100 */
    { "10", 2, 2, 5, 1030 },  /* en QTY.N_10 */
    { "1", 1, 2, 5, 1025 },  /* en QTY.N_1 */
    { "2 hours", 7, 2, 3, 778 },  /* en TIME.HORA_2 */
    { "20", 2, 2, 5, 1031 },  /* en QTY.N_20 */
    { "2", 1, 2, 5, 1026 },  /* en QTY.N_2 */
    { "30 minutes", 10, 2, 3, 771 },  /* en TIME.MIN_30 */
    { "3", 1, 2, 5, 1027 },  /* en QTY.N_3 */
    { "4", 1, 2, 5, 1028 },  /* en QTY.N_4 */
    { "5 minutes", 9, 2, 3, 769 },  /* en TIME.MIN_5 */
    { "50", 2, 2, 5, 1032 },  /* en QTY.N_50 */
    { "5", 1, 2, 5, 1029 },  /* en QTY.N_5 */
    { "automatically", 13, 7, 0, 0 },  /* en authority */
    { "accompanied", 11, 2, 6, 1288 },  /* en ST.ACOMPANHADO */
    { "acknowledge", 11, 1, 0, 6 },  /* en op CONFIRMAR */
    { "affirmative", 11, 2, 2, 13 },  /* en EV.SIM */
    { "afternoon", 9, 2, 3, 780 },  /* en TIME.TARDE_DIA */
    { "ambulance", 9, 2, 2, 29 },  /* en EV.MEDICO */
    { "a little", 8, 2, 5, 1034 },  /* en QTY.POUCO */
    { "accident", 8, 2, 2, 26 },  /* en EV.ACIDENTE */
    { "all good", 8, 2, 2, 4 },  /* en EV.TUDO_BEM */
    { "airport", 7, 2, 4, 521 },  /* en LOC.AEROPORTO */
    { "all set", 7, 2, 2, 51 },  /* en EV.PRONTO */
    { "arrived", 7, 2, 2, 1 },  /* en EV.CHEGUEI */
    { "afraid", 6, 2, 6, 1291 },  /* en ST.COM_MEDO */
    { "a lot", 5, 2, 5, 1035 },  /* en QTY.MUITO */
    { "abort", 5, 1, 0, 7 },  /* en op CANCELAR */
    { "about", 5, 0, 0, 0 },  /* en stop */
    { "alone", 5, 2, 6, 1287 },  /* en ST.SOZINHO_ST */
    { "audio", 5, 6, 0, 0 },  /* en sensitive */
    { "awake", 5, 2, 2, 38 },  /* en EV.ACORDEI */
    { "ack", 3, 1, 0, 6 },  /* en op CONFIRMAR */
    { "ana", 3, 2, 1, 258 },  /* en PER.ANA */
    { "and", 3, 0, 0, 0 },  /* en stop */
    { "are", 3, 0, 0, 0 },  /* en stop */
    { "ask", 3, 1, 0, 2 },  /* en op PERGUNTAR */
    { "am", 2, 0, 0, 0 },  /* en stop */
    { "an", 2, 0, 0, 0 },  /* en stop */
    { "at", 2, 0, 0, 0 },  /* en stop */
    { "a", 1, 0, 0, 0 },  /* en stop */
    { "battery dying", 13, 2, 2, 10 },  /* en EV.SEM_BATERIA */
    { "by yourself", 11, 7, 0, 0 },  /* en authority */
    { "broke down", 10, 2, 2, 43 },  /* en EV.VEICULO_QUEBROU */
    { "biometric", 9, 6, 0, 0 },  /* en sensitive */
    { "busy now", 8, 2, 2, 42 },  /* en EV.NAO_POSSO_FALAR */
    { "bought", 6, 2, 2, 47 },  /* en EV.COMPREI */
    { "base", 4, 2, 4, 527 },  /* en LOC.BASE */
    { "boss", 4, 2, 1, 266 },  /* en PER.CHEFE */
    { "bad", 3, 2, 6, 1281 },  /* en ST.MAL */
    { "bye", 3, 2, 2, 21 },  /* en EV.BOA_NOITE */
    { "be", 2, 0, 0, 0 },  /* en stop */
    { "congratulations", 15, 2, 2, 19 },  /* en EV.PARABENS */
    { "call police", 11, 2, 2, 28 },  /* en EV.POLICIA */
    { "cannot talk", 11, 2, 2, 42 },  /* en EV.NAO_POSSO_FALAR */
    { "coordinates", 11, 6, 0, 0 },  /* en sensitive */
    { "called off", 10, 2, 2, 53 },  /* en EV.CANCELADO */
    { "coordinate", 10, 6, 0, 0 },  /* en sensitive */
    { "cancelled", 9, 2, 2, 53 },  /* en EV.CANCELADO */
    { "come here", 9, 2, 2, 24 },  /* en EV.VEM */
    { "contact 1", 9, 2, 1, 270 },  /* en PER.HANDLE_1 */
    { "contact 2", 9, 2, 1, 271 },  /* en PER.HANDLE_2 */
    { "contact 3", 9, 2, 1, 272 },  /* en PER.HANDLE_3 */
    { "contact 4", 9, 2, 1, 273 },  /* en PER.HANDLE_4 */
    { "campsite", 8, 2, 4, 524 },  /* en LOC.ACAMPAMENTO */
    { "car park", 8, 2, 4, 530 },  /* en LOC.ESTACIONAMENTO */
    { "congrats", 8, 2, 2, 19 },  /* en EV.PARABENS */
    { "critical", 8, 2, 6, 1285 },  /* en ST.GRAVE */
    { "call me", 7, 2, 2, 41 },  /* en EV.LIGA_PRA_MIM */
    { "careful", 7, 2, 2, 22 },  /* en EV.CUIDADO */
    { "confirm", 7, 1, 0, 6 },  /* en op CONFIRMAR */
    { "campus", 6, 2, 4, 518 },  /* en LOC.ESCOLA */
    { "cancel", 6, 1, 0, 7 },  /* en op CANCELAR */
    { "cannot", 6, 2, 2, 14 },  /* en EV.IMPOSSIVEL */
    { "clinic", 6, 2, 4, 519 },  /* en LOC.HOSPITAL */
    { "coming", 6, 2, 2, 8 },  /* en EV.A_CAMINHO */
    { "child", 5, 2, 1, 264 },  /* en PER.FILHO */
    { "count", 5, 2, 2, 60 },  /* en EV.CONTAGEM */
    { "crash", 5, 2, 2, 26 },  /* en EV.ACIDENTE */
    { "calm", 4, 2, 6, 1290 },  /* en ST.CALMO */
    { "camp", 4, 2, 4, 524 },  /* en LOC.ACAMPAMENTO */
    { "cant", 4, 2, 2, 14 },  /* en EV.IMPOSSIVEL */
    { "card", 4, 6, 0, 0 },  /* en sensitive */
    { "cold", 4, 2, 2, 34 },  /* en EV.FRIO */
    { "come", 4, 2, 2, 24 },  /* en EV.VEM */
    { "crew", 4, 2, 1, 259 },  /* en PER.EQUIPE */
    { "did not understand", 18, 2, 2, 56 },  /* en EV.NAO_ENTENDI */
    { "disregard", 9, 7, 0, 0 },  /* en authority */
    { "doctor", 6, 2, 2, 29 },  /* en EV.MEDICO */
    { "done", 4, 2, 2, 9 },  /* en EV.TERMINEI */
    { "dad", 3, 2, 1, 263 },  /* en PER.PAI */
    { "embedding", 9, 6, 0, 0 },  /* en sensitive */
    { "emergency", 9, 1, 0, 5 },  /* en op SOCORRO */
    { "every one", 9, 2, 1, 261 },  /* en PER.TODOS */
    { "exhausted", 9, 2, 2, 36 },  /* en EV.CANSADO */
    { "entrance", 8, 2, 4, 529 },  /* en LOC.ENTRADA */
    { "everyone", 8, 2, 1, 261 },  /* en PER.TODOS */
    { "five minutes", 12, 2, 3, 769 },  /* en TIME.MIN_5 */
    { "front door", 10, 2, 4, 529 },  /* en LOC.ENTRADA */
    { "finished", 8, 2, 2, 9 },  /* en EV.TERMINEI */
    { "found it", 8, 2, 2, 45 },  /* en EV.ACHEI */
    { "freezing", 8, 2, 2, 34 },  /* en EV.FRIO */
    { "family", 6, 2, 1, 260 },  /* en PER.FAMILIA */
    { "father", 6, 2, 1, 263 },  /* en PER.PAI */
    { "friend", 6, 2, 1, 265 },  /* en PER.AMIGO */
    { "fifty", 5, 2, 5, 1032 },  /* en QTY.N_50 */
    { "found", 5, 2, 2, 45 },  /* en EV.ACHEI */
    { "fine", 4, 2, 6, 1280 },  /* en ST.BEM */
    { "fire", 4, 2, 2, 27 },  /* en EV.FOGO */
    { "five", 4, 2, 5, 1029 },  /* en QTY.N_5 */
    { "food", 4, 2, 2, 32 },  /* en EV.COMIDA */
    { "four", 4, 2, 5, 1028 },  /* en QTY.N_4 */
    { "few", 3, 2, 5, 1034 },  /* en QTY.POUCO */
    { "for", 3, 0, 0, 0 },  /* en stop */
    { "going to sleep", 14, 2, 2, 37 },  /* en EV.DORMINDO */
    { "good morning", 12, 2, 2, 20 },  /* en EV.BOM_DIA */
    { "good night", 10, 2, 2, 21 },  /* en EV.BOA_NOITE */
    { "group 1", 7, 2, 1, 274 },  /* en PER.GRUPO_1 */
    { "group 2", 7, 2, 1, 275 },  /* en PER.GRUPO_2 */
    { "got it", 6, 2, 2, 50 },  /* en EV.RECEBI */
    { "guide", 5, 2, 1, 267 },  /* en PER.GUIA */
    { "gps", 3, 6, 0, 0 },  /* en sensitive */
    { "gym", 3, 2, 4, 515 },  /* en LOC.TREINO */
    { "half an hour", 12, 2, 3, 771 },  /* en TIME.MIN_30 */
    { "heading back", 12, 2, 2, 55 },  /* en EV.VOLTANDO */
    { "heading out", 11, 2, 2, 2 },  /* en EV.SAINDO */
    { "headcount", 9, 2, 2, 60 },  /* en EV.CONTAGEM */
    { "hospital", 8, 2, 4, 519 },  /* en LOC.HOSPITAL */
    { "how many", 8, 5, 5, 0 },  /* en qword QUANTO */
    { "highway", 7, 2, 4, 516 },  /* en LOC.ESTRADA */
    { "hold on", 7, 2, 2, 25 },  /* en EV.ESPERA */
    { "hundred", 7, 2, 5, 1033 },  /* en QTY.N_100 */
    { "hungry", 6, 2, 2, 32 },  /* en EV.COMIDA */
    { "hello", 5, 2, 2, 20 },  /* en EV.BOM_DIA */
    { "house", 5, 2, 4, 512 },  /* en LOC.CASA */
    { "hurts", 5, 2, 2, 30 },  /* en EV.DOR */
    { "half", 4, 2, 5, 1036 },  /* en QTY.METADE */
    { "halt", 4, 2, 2, 23 },  /* en EV.PARE */
    { "have", 4, 0, 0, 0 },  /* en stop */
    { "heat", 4, 2, 2, 35 },  /* en EV.CALOR */
    { "help", 4, 2, 2, 5 },  /* en EV.AJUDA */
    { "home", 4, 2, 4, 512 },  /* en LOC.CASA */
    { "hurt", 4, 2, 6, 1284 },  /* en ST.FERIDO */
    { "has", 3, 0, 0, 0 },  /* en stop */
    { "hot", 3, 2, 2, 35 },  /* en EV.CALOR */
    { "hi", 2, 2, 2, 20 },  /* en EV.BOM_DIA */
    { "hq", 2, 2, 4, 527 },  /* en LOC.BASE */
    { "in the afternoon", 16, 2, 3, 780 },  /* en TIME.TARDE_DIA */
    { "in the morning", 14, 2, 3, 779 },  /* en TIME.MANHA */
    { "in a meeting", 12, 2, 2, 40 },  /* en EV.REUNIAO */
    { "i need help", 11, 2, 2, 5 },  /* en EV.AJUDA */
    { "i finished", 10, 2, 2, 9 },  /* en EV.TERMINEI */
    { "i love you", 10, 2, 2, 17 },  /* en EV.TE_AMO */
    { "i miss you", 10, 2, 2, 18 },  /* en EV.SAUDADE */
    { "im leaving", 10, 2, 2, 2 },  /* en EV.SAINDO */
    { "im waiting", 10, 2, 2, 6 },  /* en EV.ESPERANDO */
    { "it started", 10, 2, 2, 52 },  /* en EV.COMECOU */
    { "i arrived", 9, 2, 2, 1 },  /* en EV.CHEGUEI */
    { "i woke up", 9, 2, 2, 38 },  /* en EV.ACORDEI */
    { "in danger", 9, 2, 6, 1283 },  /* en ST.EM_PERIGO */
    { "in a bit", 8, 2, 3, 785 },  /* en TIME.EM_BREVE */
    { "im here", 7, 2, 2, 1 },  /* en EV.CHEGUEI */
    { "im late", 7, 2, 2, 3 },  /* en EV.ATRASADO */
    { "im lost", 7, 2, 2, 11 },  /* en EV.PERDIDO */
    { "in pain", 7, 2, 2, 30 },  /* en EV.DOR */
    { "injured", 7, 2, 6, 1284 },  /* en ST.FERIDO */
    { "ignore", 6, 7, 0, 0 },  /* en authority */
    { "im", 2, 0, 0, 0 },  /* en stop */
    { "in", 2, 0, 0, 0 },  /* en stop */
    { "is", 2, 0, 0, 0 },  /* en stop */
    { "it", 2, 0, 0, 0 },  /* en stop */
    { "i", 1, 0, 0, 0 },  /* en stop */
    { "just arrived", 12, 2, 2, 1 },  /* en EV.CHEGUEI */
    { "joao", 4, 2, 1, 256 },  /* en PER.JOAO */
    { "just", 4, 0, 0, 0 },  /* en stop */
    { "key", 3, 6, 0, 0 },  /* en sensitive */
    { "kid", 3, 2, 1, 264 },  /* en PER.FILHO */
    { "looking for it", 14, 2, 2, 46 },  /* en EV.PROCURANDO */
    { "low battery", 11, 2, 2, 10 },  /* en EV.SEM_BATERIA */
    { "lets meet", 9, 2, 2, 7 },  /* en EV.ENCONTRAR */
    { "location", 8, 6, 0, 0 },  /* en sensitive */
    { "love you", 8, 2, 2, 17 },  /* en EV.TE_AMO */
    { "leaving", 7, 2, 2, 2 },  /* en EV.SAINDO */
    { "later", 5, 2, 3, 782 },  /* en TIME.DEPOIS */
    { "late", 4, 2, 2, 3 },  /* en EV.ATRASADO */
    { "lead", 4, 2, 1, 267 },  /* en PER.GUIA */
    { "lost", 4, 2, 2, 11 },  /* en EV.PERDIDO */
    { "meeting point", 13, 2, 4, 528 },  /* en LOC.PONTO_ENCONTRO */
    { "my mistake", 10, 2, 2, 16 },  /* en EV.DESCULPA */
    { "miss you", 8, 2, 2, 18 },  /* en EV.SAUDADE */
    { "manager", 7, 2, 1, 266 },  /* en PER.CHEFE */
    { "meeting", 7, 2, 2, 40 },  /* en EV.REUNIAO */
    { "message", 7, 1, 0, 1 },  /* en op COMUNICAR */
    { "morning", 7, 2, 3, 779 },  /* en TIME.MANHA */
    { "market", 6, 2, 4, 520 },  /* en LOC.MERCADO */
    { "mayday", 6, 1, 0, 5 },  /* en op SOCORRO */
    { "mother", 6, 2, 1, 262 },  /* en PER.MAE */
    { "maria", 5, 2, 1, 257 },  /* en PER.MARIA */
    { "medic", 5, 2, 2, 29 },  /* en EV.MEDICO */
    { "many", 4, 2, 5, 1035 },  /* en QTY.MUITO */
    { "meet", 4, 2, 2, 7 },  /* en EV.ENCONTRAR */
    { "mom", 3, 2, 1, 262 },  /* en PER.MAE */
    { "me", 2, 0, 0, 0 },  /* en stop */
    { "my", 2, 0, 0, 0 },  /* en stop */
    { "need shelter", 12, 2, 2, 33 },  /* en EV.ABRIGO */
    { "need to talk", 12, 2, 2, 58 },  /* en EV.PRECISO_FALAR */
    { "no coverage", 11, 2, 2, 44 },  /* en EV.SEM_SINAL */
    { "need water", 10, 2, 2, 31 },  /* en EV.AGUA */
    { "no problem", 10, 2, 2, 4 },  /* en EV.TUDO_BEM */
    { "north gate", 10, 2, 4, 514 },  /* en LOC.PORTAO_N */
    { "need food", 9, 2, 2, 32 },  /* en EV.COMIDA */
    { "need help", 9, 2, 2, 5 },  /* en EV.AJUDA */
    { "neighbour", 9, 2, 1, 269 },  /* en PER.VIZINHO */
    { "no signal", 9, 2, 2, 44 },  /* en EV.SEM_SINAL */
    { "neighbor", 8, 2, 1, 269 },  /* en PER.VIZINHO */
    { "new plan", 8, 2, 2, 54 },  /* en EV.PLANO_MUDOU */
    { "notify", 6, 1, 0, 1 },  /* en op COMUNICAR */
    { "none", 4, 2, 5, 1024 },  /* en QTY.N_0 */
    { "note", 4, 1, 0, 3 },  /* en op LEMBRAR */
    { "not", 3, 3, 0, 0 },  /* en negation */
    { "now", 3, 2, 3, 768 },  /* en TIME.AGORA */
    { "no", 2, 3, 0, 0 },  /* en negation */
    { "on your own", 11, 7, 0, 0 },  /* en authority */
    { "on the way", 10, 2, 2, 8 },  /* en EV.A_CAMINHO */
    { "one minute", 10, 2, 3, 776 },  /* en TIME.MIN_1 */
    { "on my way", 9, 2, 2, 8 },  /* en EV.A_CAMINHO */
    { "one hour", 8, 2, 3, 772 },  /* en TIME.HORA_1 */
    { "office", 6, 2, 4, 513 },  /* en LOC.TRABALHO */
    { "one", 3, 2, 5, 1025 },  /* en QTY.N_1 */
    { "of", 2, 0, 0, 0 },  /* en stop */
    { "ok", 2, 2, 2, 4 },  /* en EV.TUDO_BEM */
    { "on", 2, 0, 0, 0 },  /* en stop */
    { "plan changed", 12, 2, 2, 54 },  /* en EV.PLANO_MUDOU */
    { "passwords", 9, 6, 0, 0 },  /* en sensitive */
    { "purchased", 9, 2, 2, 47 },  /* en EV.COMPREI */
    { "password", 8, 6, 0, 0 },  /* en sensitive */
    { "phone me", 8, 2, 2, 41 },  /* en EV.LIGA_PRA_MIM */
    { "parking", 7, 2, 4, 530 },  /* en LOC.ESTACIONAMENTO */
    { "payment", 7, 2, 2, 48 },  /* en EV.PAGAMENTO */
    { "please", 6, 0, 0, 0 },  /* en stop */
    { "police", 6, 2, 2, 28 },  /* en EV.POLICIA */
    { "paid", 4, 2, 2, 48 },  /* en EV.PAGAMENTO */
    { "pain", 4, 2, 2, 30 },  /* en EV.DOR */
    { "plan", 4, 1, 0, 4 },  /* en op PLANEJAR */
    { "pay", 3, 2, 2, 48 },  /* en EV.PAGAMENTO */
    { "quick", 5, 4, 0, 1 },  /* en urgency ATENCAO */
    { "running late", 12, 2, 2, 3 },  /* en EV.ATRASADO */
    { "recordings", 10, 6, 0, 0 },  /* en sensitive */
    { "recording", 9, 6, 0, 0 },  /* en sensitive */
    { "responder", 9, 2, 1, 268 },  /* en PER.SOCORRISTA */
    { "returning", 9, 2, 2, 55 },  /* en EV.VOLTANDO */
    { "right now", 9, 2, 3, 768 },  /* en TIME.AGORA */
    { "received", 8, 2, 2, 50 },  /* en EV.RECEBI */
    { "remember", 8, 1, 0, 3 },  /* en op LEMBRAR */
    { "remind", 6, 1, 0, 3 },  /* en op LEMBRAR */
    { "repeat", 6, 2, 2, 57 },  /* en EV.REPETE */
    { "rescue", 6, 2, 1, 268 },  /* en PER.SOCORRISTA */
    { "ready", 5, 2, 2, 51 },  /* en EV.PRONTO */
    { "river", 5, 2, 4, 525 },  /* en LOC.RIO */
    { "road", 4, 2, 4, 516 },  /* en LOC.ESTRADA */
    { "something is off", 16, 2, 2, 59 },  /* en EV.ALGO_ERRADO */
    { "something wrong", 15, 2, 2, 59 },  /* en EV.ALGO_ERRADO */
    { "say again", 9, 2, 2, 57 },  /* en EV.REPETE */
    { "searching", 9, 2, 2, 46 },  /* en EV.PROCURANDO */
    { "schedule", 8, 1, 0, 4 },  /* en op PLANEJAR */
    { "sleeping", 8, 2, 2, 37 },  /* en EV.DORMINDO */
    { "studying", 8, 2, 2, 12 },  /* en EV.ESTUDAR */
    { "see you", 7, 2, 2, 7 },  /* en EV.ENCONTRAR */
    { "serious", 7, 2, 6, 1285 },  /* en ST.GRAVE */
    { "shelter", 7, 2, 2, 33 },  /* en EV.ABRIGO */
    { "shipped", 7, 2, 2, 49 },  /* en EV.ENVIEI */
    { "shortly", 7, 2, 3, 785 },  /* en TIME.EM_BREVE */
    { "started", 7, 2, 2, 52 },  /* en EV.COMECOU */
    { "station", 7, 2, 4, 522 },  /* en LOC.ESTACAO */
    { "scared", 6, 2, 6, 1291 },  /* en ST.COM_MEDO */
    { "school", 6, 2, 4, 518 },  /* en LOC.ESCOLA */
    { "stable", 6, 2, 6, 1286 },  /* en ST.ESTAVEL */
    { "summit", 6, 2, 4, 526 },  /* en LOC.CUME */
    { "smoke", 5, 2, 2, 27 },  /* en EV.FOGO */
    { "sorry", 5, 2, 2, 16 },  /* en EV.DESCULPA */
    { "stuck", 5, 2, 6, 1289 },  /* en ST.PRESO */
    { "study", 5, 2, 2, 12 },  /* en EV.ESTUDAR */
    { "safe", 4, 2, 6, 1282 },  /* en ST.SEGURO */
    { "save", 4, 1, 0, 3 },  /* en op LEMBRAR */
    { "sent", 4, 2, 2, 49 },  /* en EV.ENVIEI */
    { "shop", 4, 2, 4, 520 },  /* en LOC.MERCADO */
    { "soon", 4, 4, 0, 1 },  /* en urgency ATENCAO */
    { "stop", 4, 2, 2, 23 },  /* en EV.PARE */
    { "say", 3, 1, 0, 1 },  /* en op COMUNICAR */
    { "sos", 3, 1, 0, 5 },  /* en op SOCORRO */
    { "ssn", 3, 6, 0, 0 },  /* en sensitive */
    { "ten minutes", 11, 2, 3, 770 },  /* en TIME.MIN_10 */
    { "transcript", 10, 6, 0, 0 },  /* en sensitive */
    { "thank you", 9, 2, 2, 15 },  /* en EV.OBRIGADO */
    { "this week", 9, 2, 3, 783 },  /* en TIME.SEMANA */
    { "trailhead", 9, 2, 4, 517 },  /* en LOC.TRILHA */
    { "tomorrow", 8, 2, 3, 774 },  /* en TIME.AMANHA */
    { "training", 8, 2, 4, 515 },  /* en LOC.TREINO */
    { "the car", 7, 2, 4, 523 },  /* en LOC.VEICULO */
    { "tonight", 7, 2, 3, 775 },  /* en TIME.NOITE */
    { "trapped", 7, 2, 6, 1289 },  /* en ST.PRESO */
    { "thanks", 6, 2, 2, 15 },  /* en EV.OBRIGADO */
    { "twenty", 6, 2, 5, 1031 },  /* en QTY.N_20 */
    { "three", 5, 2, 5, 1027 },  /* en QTY.N_3 */
    { "tired", 5, 2, 2, 36 },  /* en EV.CANSADO */
    { "today", 5, 2, 3, 773 },  /* en TIME.HOJE */
    { "total", 5, 2, 5, 1037 },  /* en QTY.TOTAL */
    { "trail", 5, 2, 4, 517 },  /* en LOC.TRILHA */
    { "team", 4, 2, 1, 259 },  /* en PER.EQUIPE */
    { "tell", 4, 1, 0, 1 },  /* en op COMUNICAR */
    { "that", 4, 0, 0, 0 },  /* en stop */
    { "this", 4, 0, 0, 0 },  /* en stop */
    { "ten", 3, 2, 5, 1030 },  /* en QTY.N_10 */
    { "the", 3, 0, 0, 0 },  /* en stop */
    { "top", 3, 2, 4, 526 },  /* en LOC.CUME */
    { "two", 3, 2, 5, 1026 },  /* en QTY.N_2 */
    { "to", 2, 0, 0, 0 },  /* en stop */
    { "unclear", 7, 2, 2, 56 },  /* en EV.NAO_ENTENDI */
    { "unable", 6, 2, 2, 14 },  /* en EV.IMPOSSIVEL */
    { "unwell", 6, 2, 6, 1281 },  /* en ST.MAL */
    { "urgent", 6, 4, 0, 2 },  /* en urgency URGENTE */
    { "us", 2, 0, 0, 0 },  /* en stop */
    { "vehicle broken", 14, 2, 2, 43 },  /* en EV.VEICULO_QUEBROU */
    { "vehicle", 7, 2, 4, 523 },  /* en LOC.VEICULO */
    { "without confirmation", 20, 7, 0, 0 },  /* en authority */
    { "without confirming", 18, 7, 0, 0 },  /* en authority */
    { "we should talk", 14, 2, 2, 58 },  /* en EV.PRECISO_FALAR */
    { "wait for me", 11, 2, 2, 25 },  /* en EV.ESPERA */
    { "with others", 11, 2, 6, 1288 },  /* en ST.ACOMPANHADO */
    { "watch out", 9, 2, 2, 22 },  /* en EV.CUIDADO */
    { "what time", 9, 5, 3, 0 },  /* en qword QUANDO */
    { "whenever", 8, 7, 0, 0 },  /* en authority */
    { "waiting", 7, 2, 2, 6 },  /* en EV.ESPERANDO */
    { "warning", 7, 2, 2, 22 },  /* en EV.CUIDADO */
    { "weekend", 7, 2, 3, 784 },  /* en TIME.FIM_DE_SEMANA */
    { "working", 7, 2, 2, 39 },  /* en EV.TRABALHANDO */
    { "water", 5, 2, 2, 31 },  /* en EV.AGUA */
    { "where", 5, 5, 4, 0 },  /* en qword ONDE */
    { "wait", 4, 2, 2, 25 },  /* en EV.ESPERA */
    { "well", 4, 2, 6, 1280 },  /* en ST.BEM */
    { "when", 4, 5, 3, 0 },  /* en qword QUANDO */
    { "will", 4, 0, 0, 0 },  /* en stop */
    { "with", 4, 0, 0, 0 },  /* en stop */
    { "work", 4, 2, 4, 513 },  /* en LOC.TRABALHO */
    { "was", 3, 0, 0, 0 },  /* en stop */
    { "who", 3, 5, 1, 0 },  /* en qword QUEM */
    { "we", 2, 0, 0, 0 },  /* en stop */
    { "yesterday", 9, 2, 3, 781 },  /* en TIME.ONTEM */
    { "yes", 3, 2, 2, 13 },  /* en EV.SIM */
    { "you", 3, 0, 0, 0 },  /* en stop */
    { "0", 1, 2, 5, 1024 },  /* es QTY.N_0 */
    { "10 minutos", 10, 2, 3, 770 },  /* es TIME.MIN_10 */
    { "15 minutos", 10, 2, 3, 777 },  /* es TIME.MIN_15 */
    { "1 minuto", 8, 2, 3, 776 },  /* es TIME.MIN_1 */
    { "1 hora", 6, 2, 3, 772 },  /* es TIME.HORA_1 */
    { "100", 3, 2, 5, 1033 },  /* es QTY.N_100 */
    { "10", 2, 2, 5, 1030 },  /* es QTY.N_10 */
    { "1", 1, 2, 5, 1025 },  /* es QTY.N_1 */
    { "2 horas", 7, 2, 3, 778 },  /* es TIME.HORA_2 */
    { "20", 2, 2, 5, 1031 },  /* es QTY.N_20 */
    { "2", 1, 2, 5, 1026 },  /* es QTY.N_2 */
    { "30 minutos", 10, 2, 3, 771 },  /* es TIME.MIN_30 */
    { "3", 1, 2, 5, 1027 },  /* es QTY.N_3 */
    { "4", 1, 2, 5, 1028 },  /* es QTY.N_4 */
    { "5 minutos", 9, 2, 3, 769 },  /* es TIME.MIN_5 */
    { "50", 2, 2, 5, 1032 },  /* es QTY.N_50 */
    { "5", 1, 2, 5, 1029 },  /* es QTY.N_5 */
    { "automaticamente", 15, 7, 0, 0 },  /* es authority */
    { "algo anda mal", 13, 2, 2, 59 },  /* es EV.ALGO_ERRADO */
    { "acompanado", 10, 2, 6, 1288 },  /* es ST.ACOMPANHADO */
    { "aeropuerto", 10, 2, 4, 521 },  /* es LOC.AEROPORTO */
    { "afirmativo", 10, 2, 2, 13 },  /* es EV.SIM */
    { "ambulancia", 10, 2, 2, 29 },  /* es EV.MEDICO */
    { "accidente", 9, 2, 2, 26 },  /* es EV.ACIDENTE */
    { "algo mal", 8, 2, 2, 59 },  /* es EV.ALGO_ERRADO */
    { "atencion", 8, 2, 2, 22 },  /* es EV.CUIDADO */
    { "atrapado", 8, 2, 6, 1289 },  /* es ST.PRESO */
    { "a salvo", 7, 2, 6, 1282 },  /* es ST.SEGURO */
    { "agotado", 7, 2, 2, 36 },  /* es EV.CANSADO */
    { "avisale", 7, 1, 0, 1 },  /* es op COMUNICAR */
    { "apunta", 6, 1, 0, 3 },  /* es op LEMBRAR */
    { "acabe", 5, 2, 2, 9 },  /* es EV.TERMINEI */
    { "adios", 5, 2, 2, 21 },  /* es EV.BOA_NOITE */
    { "ahora", 5, 2, 3, 768 },  /* es TIME.AGORA */
    { "amiga", 5, 2, 1, 265 },  /* es PER.AMIGO */
    { "amigo", 5, 2, 1, 265 },  /* es PER.AMIGO */
    { "audio", 5, 6, 0, 0 },  /* es sensitive */
    { "avisa", 5, 1, 0, 1 },  /* es op COMUNICAR */
    { "ayuda", 5, 2, 2, 5 },  /* es EV.AJUDA */
    { "agua", 4, 2, 2, 31 },  /* es EV.AGUA */
    { "alto", 4, 2, 2, 23 },  /* es EV.PARE */
    { "ayer", 4, 2, 3, 781 },  /* es TIME.ONTEM */
    { "ana", 3, 2, 1, 258 },  /* es PER.ANA */
    { "al", 2, 0, 0, 0 },  /* es stop */
    { "a", 1, 0, 0, 0 },  /* es stop */
    { "buenas noches", 13, 2, 2, 21 },  /* es EV.BOA_NOITE */
    { "buenos dias", 11, 2, 2, 20 },  /* es EV.BOM_DIA */
    { "biometria", 9, 6, 0, 0 },  /* es sensitive */
    { "buscando", 8, 2, 2, 46 },  /* es EV.PROCURANDO */
    { "base", 4, 2, 4, 527 },  /* es LOC.BASE */
    { "bien", 4, 2, 6, 1280 },  /* es ST.BEM */
    { "cambio el plan", 14, 2, 2, 54 },  /* es EV.PLANO_MUDOU */
    { "cuantos somos", 13, 2, 2, 60 },  /* es EV.CONTAGEM */
    { "coordenadas", 11, 6, 0, 0 },  /* es sensitive */
    { "campamento", 10, 2, 4, 524 },  /* es LOC.ACAMPAMENTO */
    { "contacto 1", 10, 2, 1, 270 },  /* es PER.HANDLE_1 */
    { "contacto 2", 10, 2, 1, 271 },  /* es PER.HANDLE_2 */
    { "contacto 3", 10, 2, 1, 272 },  /* es PER.HANDLE_3 */
    { "contacto 4", 10, 2, 1, 273 },  /* es PER.HANDLE_4 */
    { "contrasena", 10, 6, 0, 0 },  /* es sensitive */
    { "coordenada", 10, 6, 0, 0 },  /* es sensitive */
    { "cancelado", 9, 2, 2, 53 },  /* es EV.CANCELADO */
    { "carretera", 9, 2, 4, 516 },  /* es LOC.ESTRADA */
    { "con miedo", 9, 2, 6, 1291 },  /* es ST.COM_MEDO */
    { "cancelar", 8, 1, 0, 7 },  /* es op CANCELAR */
    { "comprado", 8, 2, 2, 47 },  /* es EV.COMPREI */
    { "comunica", 8, 1, 0, 1 },  /* es op COMUNICAR */
    { "confirma", 8, 1, 0, 6 },  /* es op CONFIRMAR */
    { "confirmo", 8, 1, 0, 6 },  /* es op CONFIRMAR */
    { "cancela", 7, 1, 0, 7 },  /* es op CANCELAR */
    { "cansado", 7, 2, 2, 36 },  /* es EV.CANSADO */
    { "comenzo", 7, 2, 2, 52 },  /* es EV.COMECOU */
    { "confuso", 7, 2, 2, 56 },  /* es EV.NAO_ENTENDI */
    { "cuantos", 7, 5, 5, 0 },  /* es qword QUANTO */
    { "cuidado", 7, 2, 2, 22 },  /* es EV.CUIDADO */
    { "choque", 6, 2, 2, 26 },  /* es EV.ACIDENTE */
    { "comida", 6, 2, 2, 32 },  /* es EV.COMIDA */
    { "compre", 6, 2, 2, 47 },  /* es EV.COMPREI */
    { "cuando", 6, 5, 3, 0 },  /* es qword QUANDO */
    { "cuatro", 6, 2, 5, 1028 },  /* es QTY.N_4 */
    { "cumbre", 6, 2, 4, 526 },  /* es LOC.CUME */
    { "calor", 5, 2, 2, 35 },  /* es EV.CALOR */
    { "cinco", 5, 2, 5, 1029 },  /* es QTY.N_5 */
    { "clave", 5, 6, 0, 0 },  /* es sensitive */
    { "coche", 5, 2, 4, 523 },  /* es LOC.VEICULO */
    { "casa", 4, 2, 4, 512 },  /* es LOC.CASA */
    { "cien", 4, 2, 5, 1033 },  /* es QTY.N_100 */
    { "con", 3, 0, 0, 0 },  /* es stop */
    { "de vuelta", 9, 2, 2, 55 },  /* es EV.VOLTANDO */
    { "despierto", 9, 2, 2, 38 },  /* es EV.ACORDEI */
    { "durmiendo", 9, 2, 2, 37 },  /* es EV.DORMINDO */
    { "disculpa", 8, 2, 2, 16 },  /* es EV.DESCULPA */
    { "despues", 7, 2, 3, 782 },  /* es TIME.DEPOIS */
    { "detente", 7, 2, 2, 23 },  /* es EV.PARE */
    { "dolor", 5, 2, 2, 30 },  /* es EV.DOR */
    { "donde", 5, 5, 4, 0 },  /* es qword ONDE */
    { "diez", 4, 2, 5, 1030 },  /* es QTY.N_10 */
    { "dile", 4, 1, 0, 1 },  /* es op COMUNICAR */
    { "del", 3, 0, 0, 0 },  /* es stop */
    { "dos", 3, 2, 5, 1026 },  /* es QTY.N_2 */
    { "de", 2, 0, 0, 0 },  /* es stop */
    { "estacionamiento", 15, 2, 4, 530 },  /* es LOC.ESTACIONAMENTO */
    { "estoy perdido", 13, 2, 2, 11 },  /* es EV.PERDIDO */
    { "encontrarnos", 12, 2, 2, 7 },  /* es EV.ENCONTRAR */
    { "enhorabuena", 11, 2, 2, 19 },  /* es EV.PARABENS */
    { "esta semana", 11, 2, 3, 783 },  /* es TIME.SEMANA */
    { "emergencia", 10, 1, 0, 5 },  /* es op SOCORRO */
    { "en peligro", 10, 2, 6, 1283 },  /* es ST.EM_PERIGO */
    { "en reunion", 10, 2, 2, 40 },  /* es EV.REUNIAO */
    { "encontrado", 10, 2, 2, 45 },  /* es EV.ACHEI */
    { "esta noche", 10, 2, 3, 775 },  /* es TIME.NOITE */
    { "estoy aqui", 10, 2, 2, 1 },  /* es EV.CHEGUEI */
    { "en camino", 9, 2, 2, 8 },  /* es EV.A_CAMINHO */
    { "esperando", 9, 2, 2, 6 },  /* es EV.ESPERANDO */
    { "en breve", 8, 2, 3, 785 },  /* es TIME.EM_BREVE */
    { "esperame", 8, 2, 2, 25 },  /* es EV.ESPERA */
    { "estacion", 8, 2, 4, 522 },  /* es LOC.ESTACAO */
    { "estudiar", 8, 2, 2, 12 },  /* es EV.ESTUDAR */
    { "entrada", 7, 2, 4, 529 },  /* es LOC.ENTRADA */
    { "enviado", 7, 2, 2, 49 },  /* es EV.ENVIEI */
    { "escuela", 7, 2, 4, 518 },  /* es LOC.ESCOLA */
    { "estable", 7, 2, 6, 1286 },  /* es ST.ESTAVEL */
    { "estudio", 7, 2, 2, 12 },  /* es EV.ESTUDAR */
    { "extrano", 7, 2, 2, 18 },  /* es EV.SAUDADE */
    { "empezo", 6, 2, 2, 52 },  /* es EV.COMECOU */
    { "equipo", 6, 2, 1, 259 },  /* es PER.EQUIPE */
    { "espera", 6, 2, 2, 25 },  /* es EV.ESPERA */
    { "envie", 5, 2, 2, 49 },  /* es EV.ENVIEI */
    { "estoy", 5, 0, 0, 0 },  /* es stop */
    { "esta", 4, 0, 0, 0 },  /* es stop */
    { "el", 2, 0, 0, 0 },  /* es stop */
    { "en", 2, 0, 0, 0 },  /* es stop */
    { "fin de semana", 13, 2, 3, 784 },  /* es TIME.FIM_DE_SEMANA */
    { "felicidades", 11, 2, 2, 19 },  /* es EV.PARABENS */
    { "familia", 7, 2, 1, 260 },  /* es PER.FAMILIA */
    { "favor", 5, 0, 0, 0 },  /* es stop */
    { "fuego", 5, 2, 2, 27 },  /* es EV.FOGO */
    { "frio", 4, 2, 2, 34 },  /* es EV.FRIO */
    { "grabacion", 9, 6, 0, 0 },  /* es sensitive */
    { "gimnasio", 8, 2, 4, 515 },  /* es LOC.TREINO */
    { "gracias", 7, 2, 2, 15 },  /* es EV.OBRIGADO */
    { "grupo 1", 7, 2, 1, 274 },  /* es PER.GRUPO_1 */
    { "grupo 2", 7, 2, 1, 275 },  /* es PER.GRUPO_2 */
    { "guarda", 6, 1, 0, 3 },  /* es op LEMBRAR */
    { "grave", 5, 2, 6, 1285 },  /* es ST.GRAVE */
    { "guia", 4, 2, 1, 267 },  /* es PER.GUIA */
    { "gps", 3, 6, 0, 0 },  /* es sensitive */
    { "hospital", 8, 2, 4, 519 },  /* es LOC.HOSPITAL */
    { "hambre", 6, 2, 2, 32 },  /* es EV.COMIDA */
    { "herido", 6, 2, 6, 1284 },  /* es ST.FERIDO */
    { "hija", 4, 2, 1, 264 },  /* es PER.FILHO */
    { "hijo", 4, 2, 1, 264 },  /* es PER.FILHO */
    { "hola", 4, 2, 2, 20 },  /* es EV.BOM_DIA */
    { "hoy", 3, 2, 3, 773 },  /* es TIME.HOJE */
    { "imposible", 9, 2, 2, 14 },  /* es EV.IMPOSSIVEL */
    { "incendio", 8, 2, 2, 27 },  /* es EV.FOGO */
    { "ignora", 6, 7, 0, 0 },  /* es authority */
    { "ignore", 6, 7, 0, 0 },  /* es authority */
    { "jefe", 4, 2, 1, 266 },  /* es PER.CHEFE */
    { "joao", 4, 2, 1, 256 },  /* es PER.JOAO */
    { "lo encontre", 11, 2, 2, 45 },  /* es EV.ACHEI */
    { "llamame", 7, 2, 2, 41 },  /* es EV.LIGA_PRA_MIM */
    { "llegue", 6, 2, 2, 1 },  /* es EV.CHEGUEI */
    { "listo", 5, 2, 2, 51 },  /* es EV.PRONTO */
    { "las", 3, 0, 0, 0 },  /* es stop */
    { "los", 3, 0, 0, 0 },  /* es stop */
    { "me voy a dormir", 15, 2, 2, 37 },  /* es EV.DORMINDO */
    { "me desperte", 11, 2, 2, 38 },  /* es EV.ACORDEI */
    { "media hora", 10, 2, 3, 771 },  /* es TIME.MIN_30 */
    { "mas tarde", 9, 2, 3, 782 },  /* es TIME.DEPOIS */
    { "me duele", 8, 2, 2, 30 },  /* es EV.DOR */
    { "mercado", 7, 2, 4, 520 },  /* es LOC.MERCADO */
    { "manana", 6, 2, 3, 774 },  /* es TIME.AMANHA */
    { "medico", 6, 2, 2, 29 },  /* es EV.MEDICO */
    { "madre", 5, 2, 1, 262 },  /* es PER.MAE */
    { "maria", 5, 2, 1, 257 },  /* es PER.MARIA */
    { "mitad", 5, 2, 5, 1036 },  /* es QTY.METADE */
    { "mucho", 5, 2, 5, 1035 },  /* es QTY.MUITO */
    { "mama", 4, 2, 1, 262 },  /* es PER.MAE */
    { "mal", 3, 2, 6, 1281 },  /* es ST.MAL */
    { "me", 2, 0, 0, 0 },  /* es stop */
    { "mi", 2, 0, 0, 0 },  /* es stop */
    { "necesito hablar", 15, 2, 2, 58 },  /* es EV.PRECISO_FALAR */
    { "no puedo hablar", 15, 2, 2, 42 },  /* es EV.NAO_POSSO_FALAR */
    { "necesito ayuda", 14, 2, 2, 5 },  /* es EV.AJUDA */
    { "necesito agua", 13, 2, 2, 31 },  /* es EV.AGUA */
    { "no entiendo", 11, 2, 2, 56 },  /* es EV.NAO_ENTENDI */
    { "nos vemos", 9, 2, 2, 7 },  /* es EV.ENCONTRAR */
    { "no puedo", 8, 2, 2, 14 },  /* es EV.IMPOSSIVEL */
    { "ninguno", 7, 2, 5, 1024 },  /* es QTY.N_0 */
    { "no", 2, 3, 0, 0 },  /* es negation */
    { "oficina", 7, 2, 4, 513 },  /* es LOC.TRABALHO */
    { "punto de encuentro", 18, 2, 4, 528 },  /* es LOC.PONTO_ENCONTRO */
    { "por la manana", 13, 2, 3, 779 },  /* es TIME.MANHA */
    { "por tu cuenta", 13, 7, 0, 0 },  /* es authority */
    { "poca bateria", 12, 2, 2, 10 },  /* es EV.SEM_BATERIA */
    { "por la tarde", 12, 2, 3, 780 },  /* es TIME.TARDE_DIA */
    { "puerta norte", 12, 2, 4, 514 },  /* es LOC.PORTAO_N */
    { "plan nuevo", 10, 2, 2, 54 },  /* es EV.PLANO_MUDOU */
    { "planificar", 10, 1, 0, 4 },  /* es op PLANEJAR */
    { "pregunta", 8, 1, 0, 2 },  /* es op PERGUNTAR */
    { "perdido", 7, 2, 2, 11 },  /* es EV.PERDIDO */
    { "policia", 7, 2, 2, 28 },  /* es EV.POLICIA */
    { "perdon", 6, 2, 2, 16 },  /* es EV.DESCULPA */
    { "planea", 6, 1, 0, 4 },  /* es op PLANEJAR */
    { "pronto", 6, 2, 3, 785 },  /* es TIME.EM_BREVE */
    { "padre", 5, 2, 1, 263 },  /* es PER.PAI */
    { "pagar", 5, 2, 2, 48 },  /* es EV.PAGAMENTO */
    { "pago", 4, 2, 2, 48 },  /* es EV.PAGAMENTO */
    { "papa", 4, 2, 1, 263 },  /* es PER.PAI */
    { "para", 4, 0, 0, 0 },  /* es stop */
    { "poco", 4, 2, 5, 1034 },  /* es QTY.POUCO */
    { "por", 3, 0, 0, 0 },  /* es stop */
    { "que hora", 8, 5, 3, 0 },  /* es qword QUANDO */
    { "quien", 5, 5, 1, 0 },  /* es qword QUEM */
    { "que", 3, 0, 0, 0 },  /* es stop */
    { "retrasado", 9, 2, 2, 3 },  /* es EV.ATRASADO */
    { "recibido", 8, 2, 2, 50 },  /* es EV.RECEBI */
    { "recordar", 8, 1, 0, 3 },  /* es op LEMBRAR */
    { "recuento", 8, 2, 2, 60 },  /* es EV.CONTAGEM */
    { "recuerda", 8, 1, 0, 3 },  /* es op LEMBRAR */
    { "refugio", 7, 2, 2, 33 },  /* es EV.ABRIGO */
    { "rescate", 7, 2, 1, 268 },  /* es PER.SOCORRISTA */
    { "reunion", 7, 2, 2, 40 },  /* es EV.REUNIAO */
    { "rapido", 6, 4, 0, 1 },  /* es urgency ATENCAO */
    { "recibi", 6, 2, 2, 50 },  /* es EV.RECEBI */
    { "repite", 6, 2, 2, 57 },  /* es EV.REPETE */
    { "rio", 3, 2, 4, 525 },  /* es LOC.RIO */
    { "sin confirmacion", 16, 7, 0, 0 },  /* es authority */
    { "sin cobertura", 13, 2, 2, 44 },  /* es EV.SEM_SINAL */
    { "sin confirmar", 13, 7, 0, 0 },  /* es authority */
    { "sin problema", 12, 2, 2, 4 },  /* es EV.TUDO_BEM */
    { "siempre que", 11, 7, 0, 0 },  /* es authority */
    { "sin bateria", 11, 2, 2, 10 },  /* es EV.SEM_BATERIA */
    { "se averio", 9, 2, 2, 43 },  /* es EV.VEICULO_QUEBROU */
    { "sin senal", 9, 2, 2, 44 },  /* es EV.SEM_SINAL */
    { "saliendo", 8, 2, 2, 2 },  /* es EV.SAINDO */
    { "sendero", 7, 2, 4, 517 },  /* es LOC.TRILHA */
    { "socorro", 7, 1, 0, 5 },  /* es op SOCORRO */
    { "sobre", 5, 0, 0, 0 },  /* es stop */
    { "solo", 4, 2, 6, 1287 },  /* es ST.SOZINHO_ST */
    { "se", 2, 0, 0, 0 },  /* es stop */
    { "si", 2, 2, 2, 13 },  /* es EV.SIM */
    { "te echo de menos", 16, 2, 2, 18 },  /* es EV.SAUDADE */
    { "transcripcion", 13, 6, 0, 0 },  /* es sensitive */
    { "todo listo", 10, 2, 2, 51 },  /* es EV.PRONTO */
    { "trabajando", 10, 2, 2, 39 },  /* es EV.TRABALHANDO */
    { "te quiero", 9, 2, 2, 17 },  /* es EV.TE_AMO */
    { "todo bien", 9, 2, 2, 4 },  /* es EV.TUDO_BEM */
    { "tranquilo", 9, 2, 6, 1290 },  /* es ST.CALMO */
    { "tarjeta", 7, 6, 0, 0 },  /* es sensitive */
    { "termine", 7, 2, 2, 9 },  /* es EV.TERMINEI */
    { "trabajo", 7, 2, 4, 513 },  /* es LOC.TRABALHO */
    { "te amo", 6, 2, 2, 17 },  /* es EV.TE_AMO */
    { "tienda", 6, 2, 4, 520 },  /* es LOC.MERCADO */
    { "todos", 5, 2, 1, 261 },  /* es PER.TODOS */
    { "total", 5, 2, 5, 1037 },  /* es QTY.TOTAL */
    { "tres", 4, 2, 5, 1027 },  /* es QTY.N_3 */
    { "te", 2, 0, 0, 0 },  /* es stop */
    { "ubicacion", 9, 6, 0, 0 },  /* es sensitive */
    { "urgente", 7, 4, 0, 2 },  /* es urgency URGENTE */
    { "una", 3, 0, 0, 0 },  /* es stop */
    { "un", 2, 0, 0, 0 },  /* es stop */
    { "vehiculo averiado", 17, 2, 2, 43 },  /* es EV.VEICULO_QUEBROU */
    { "voy para alla", 13, 2, 2, 8 },  /* es EV.A_CAMINHO */
    { "volviendo", 9, 2, 2, 55 },  /* es EV.VOLTANDO */
    { "voy tarde", 9, 2, 2, 3 },  /* es EV.ATRASADO */
    { "vehiculo", 8, 2, 4, 523 },  /* es LOC.VEICULO */
    { "ven aca", 7, 2, 2, 24 },  /* es EV.VEM */
    { "vecino", 6, 2, 1, 269 },  /* es PER.VIZINHO */
    { "veinte", 6, 2, 5, 1031 },  /* es QTY.N_20 */
    { "ven", 3, 2, 2, 24 },  /* es EV.VEM */
    { "ya llegue", 9, 2, 2, 1 },  /* es EV.CHEGUEI */
    { "ya salgo", 8, 2, 2, 2 },  /* es EV.SAINDO */
    { "ya", 2, 0, 0, 0 },  /* es stop */
    { "yo", 2, 0, 0, 0 },  /* es stop */
    { "y", 1, 0, 0, 0 },  /* es stop */
    { "0", 1, 2, 5, 1024 },  /* fr QTY.N_0 */
    { "10 minutes", 10, 2, 3, 770 },  /* fr TIME.MIN_10 */
    { "15 minutes", 10, 2, 3, 777 },  /* fr TIME.MIN_15 */
    { "1 minute", 8, 2, 3, 776 },  /* fr TIME.MIN_1 */
    { "1 heure", 7, 2, 3, 772 },  /* fr TIME.HORA_1 */
    { "100", 3, 2, 5, 1033 },  /* fr QTY.N_100 */
    { "10", 2, 2, 5, 1030 },  /* fr QTY.N_10 */
    { "1", 1, 2, 5, 1025 },  /* fr QTY.N_1 */
    { "2 heures", 8, 2, 3, 778 },  /* fr TIME.HORA_2 */
    { "20", 2, 2, 5, 1031 },  /* fr QTY.N_20 */
    { "2", 1, 2, 5, 1026 },  /* fr QTY.N_2 */
    { "30 minutes", 10, 2, 3, 771 },  /* fr TIME.MIN_30 */
    { "3", 1, 2, 5, 1027 },  /* fr QTY.N_3 */
    { "4", 1, 2, 5, 1028 },  /* fr QTY.N_4 */
    { "5 minutes", 9, 2, 3, 769 },  /* fr TIME.MIN_5 */
    { "50", 2, 2, 5, 1032 },  /* fr QTY.N_50 */
    { "5", 1, 2, 5, 1029 },  /* fr QTY.N_5 */
    { "automatiquement", 15, 7, 0, 0 },  /* fr authority */
    { "appelle moi", 11, 2, 2, 41 },  /* fr EV.LIGA_PRA_MIM */
    { "attends moi", 11, 2, 2, 25 },  /* fr EV.ESPERA */
    { "accompagne", 10, 2, 6, 1288 },  /* fr ST.ACOMPANHADO */
    { "annulation", 10, 2, 2, 53 },  /* fr EV.CANCELADO */
    { "apres midi", 10, 2, 3, 780 },  /* fr TIME.TARDE_DIA */
    { "au travail", 10, 2, 2, 39 },  /* fr EV.TRABALHANDO */
    { "aujourdhui", 10, 2, 3, 773 },  /* fr TIME.HOJE */
    { "ambulance", 9, 2, 2, 29 },  /* fr EV.MEDICO */
    { "attention", 9, 2, 2, 22 },  /* fr EV.CUIDADO */
    { "au revoir", 9, 2, 2, 21 },  /* fr EV.BOA_NOITE */
    { "accident", 8, 2, 2, 26 },  /* fr EV.ACIDENTE */
    { "aeroport", 8, 2, 4, 521 },  /* fr LOC.AEROPORTO */
    { "annonce", 7, 1, 0, 1 },  /* fr op COMUNICAR */
    { "attends", 7, 2, 2, 25 },  /* fr EV.ESPERA */
    { "achete", 6, 2, 2, 47 },  /* fr EV.COMPREI */
    { "annule", 6, 1, 0, 7 },  /* fr op CANCELAR */
    { "arrete", 6, 2, 2, 23 },  /* fr EV.PARE */
    { "arrive", 6, 2, 2, 1 },  /* fr EV.CHEGUEI */
    { "aucun", 5, 2, 5, 1024 },  /* fr QTY.N_0 */
    { "audio", 5, 6, 0, 0 },  /* fr sensitive */
    { "abri", 4, 2, 2, 33 },  /* fr EV.ABRIGO */
    { "aide", 4, 2, 2, 5 },  /* fr EV.AJUDA */
    { "avec", 4, 0, 0, 0 },  /* fr stop */
    { "ami", 3, 2, 1, 265 },  /* fr PER.AMIGO */
    { "ana", 3, 2, 1, 258 },  /* fr PER.ANA */
    { "aux", 3, 0, 0, 0 },  /* fr stop */
    { "au", 2, 0, 0, 0 },  /* fr stop */
    { "a", 1, 0, 0, 0 },  /* fr stop */
    { "batterie faible", 15, 2, 2, 10 },  /* fr EV.SEM_BATERIA */
    { "besoin daide", 12, 2, 2, 5 },  /* fr EV.AJUDA */
    { "besoin deau", 11, 2, 2, 31 },  /* fr EV.AGUA */
    { "bonne nuit", 10, 2, 2, 21 },  /* fr EV.BOA_NOITE */
    { "biometrie", 9, 6, 0, 0 },  /* fr sensitive */
    { "beaucoup", 8, 2, 5, 1035 },  /* fr QTY.MUITO */
    { "bientot", 7, 2, 3, 785 },  /* fr TIME.EM_BREVE */
    { "bonjour", 7, 2, 2, 20 },  /* fr EV.BOM_DIA */
    { "blesse", 6, 2, 6, 1284 },  /* fr ST.FERIDO */
    { "bureau", 6, 2, 4, 513 },  /* fr LOC.TRABALHO */
    { "bravo", 5, 2, 2, 19 },  /* fr EV.PARABENS */
    { "base", 4, 2, 4, 527 },  /* fr LOC.BASE */
    { "bien", 4, 2, 6, 1280 },  /* fr ST.BEM */
    { "cette semaine", 13, 2, 3, 783 },  /* fr TIME.SEMANA */
    { "chaque fois", 11, 7, 0, 0 },  /* fr authority */
    { "coordonnees", 11, 6, 0, 0 },  /* fr sensitive */
    { "coordonnee", 10, 6, 0, 0 },  /* fr sensitive */
    { "campement", 9, 2, 4, 524 },  /* fr LOC.ACAMPAMENTO */
    { "contact 1", 9, 2, 1, 270 },  /* fr PER.HANDLE_1 */
    { "contact 2", 9, 2, 1, 271 },  /* fr PER.HANDLE_2 */
    { "contact 3", 9, 2, 1, 272 },  /* fr PER.HANDLE_3 */
    { "contact 4", 9, 2, 1, 273 },  /* fr PER.HANDLE_4 */
    { "commence", 8, 2, 2, 52 },  /* fr EV.COMECOU */
    { "confirme", 8, 1, 0, 6 },  /* fr op CONFIRMAR */
    { "ce soir", 7, 2, 3, 775 },  /* fr TIME.NOITE */
    { "chaleur", 7, 2, 2, 35 },  /* fr EV.CALOR */
    { "combien", 7, 5, 5, 0 },  /* fr qword QUANTO */
    { "coince", 6, 2, 6, 1289 },  /* fr ST.PRESO */
    { "compte", 6, 2, 2, 60 },  /* fr EV.CONTAGEM */
    { "calme", 5, 2, 6, 1290 },  /* fr ST.CALMO */
    { "carte", 5, 6, 0, 0 },  /* fr sensitive */
    { "chaud", 5, 2, 2, 35 },  /* fr EV.CALOR */
    { "cent", 4, 2, 5, 1033 },  /* fr QTY.N_100 */
    { "chef", 4, 2, 1, 266 },  /* fr PER.CHEFE */
    { "cinq", 4, 2, 5, 1029 },  /* fr QTY.N_5 */
    { "cle", 3, 6, 0, 0 },  /* fr sensitive */
    { "de toi meme", 11, 7, 0, 0 },  /* fr authority */
    { "daccord", 7, 2, 2, 13 },  /* fr EV.SIM */
    { "demande", 7, 1, 0, 2 },  /* fr op PERGUNTAR */
    { "douleur", 7, 2, 2, 30 },  /* fr EV.DOR */
    { "demain", 6, 2, 3, 774 },  /* fr TIME.AMANHA */
    { "desole", 6, 2, 2, 16 },  /* fr EV.DESCULPA */
    { "deja", 4, 0, 0, 0 },  /* fr stop */
    { "deux", 4, 2, 5, 1026 },  /* fr QTY.N_2 */
    { "des", 3, 0, 0, 0 },  /* fr stop */
    { "dis", 3, 1, 0, 1 },  /* fr op COMUNICAR */
    { "dix", 3, 2, 5, 1030 },  /* fr QTY.N_10 */
    { "de", 2, 0, 0, 0 },  /* fr stop */
    { "du", 2, 0, 0, 0 },  /* fr stop */
    { "enregistrement", 14, 6, 0, 0 },  /* fr sensitive */
    { "en partance", 11, 2, 2, 2 },  /* fr EV.SAINDO */
    { "en securite", 11, 2, 6, 1282 },  /* fr ST.SEGURO */
    { "en attente", 10, 2, 2, 6 },  /* fr EV.ESPERANDO */
    { "en danger", 9, 2, 6, 1283 },  /* fr ST.EM_PERIGO */
    { "en retard", 9, 2, 2, 3 },  /* fr EV.ATRASADO */
    { "effectif", 8, 2, 2, 60 },  /* fr EV.CONTAGEM */
    { "en panne", 8, 2, 2, 43 },  /* fr EV.VEICULO_QUEBROU */
    { "en route", 8, 2, 2, 8 },  /* fr EV.A_CAMINHO */
    { "effraye", 7, 2, 6, 1291 },  /* fr ST.COM_MEDO */
    { "etudier", 7, 2, 2, 12 },  /* fr EV.ESTUDAR */
    { "enfant", 6, 2, 1, 264 },  /* fr PER.FILHO */
    { "entree", 6, 2, 4, 529 },  /* fr LOC.ENTRADA */
    { "envoye", 6, 2, 2, 49 },  /* fr EV.ENVIEI */
    { "epuise", 6, 2, 2, 36 },  /* fr EV.CANSADO */
    { "equipe", 6, 2, 1, 259 },  /* fr PER.EQUIPE */
    { "ecole", 5, 2, 4, 518 },  /* fr LOC.ESCOLA */
    { "etude", 5, 2, 2, 12 },  /* fr EV.ESTUDAR */
    { "eau", 3, 2, 2, 31 },  /* fr EV.AGUA */
    { "est", 3, 0, 0, 0 },  /* fr stop */
    { "et", 2, 0, 0, 0 },  /* fr stop */
    { "felicitation", 12, 2, 2, 19 },  /* fr EV.PARABENS */
    { "famille", 7, 2, 1, 260 },  /* fr PER.FAMILIA */
    { "fatigue", 7, 2, 2, 36 },  /* fr EV.CANSADO */
    { "froid", 5, 2, 2, 34 },  /* fr EV.FRIO */
    { "faim", 4, 2, 2, 32 },  /* fr EV.COMIDA */
    { "feu", 3, 2, 2, 27 },  /* fr EV.FOGO */
    { "groupe 1", 8, 2, 1, 274 },  /* fr PER.GRUPO_1 */
    { "groupe 2", 8, 2, 1, 275 },  /* fr PER.GRUPO_2 */
    { "garde", 5, 1, 0, 3 },  /* fr op LEMBRAR */
    { "grave", 5, 2, 6, 1285 },  /* fr ST.GRAVE */
    { "guide", 5, 2, 1, 267 },  /* fr PER.GUIA */
    { "gare", 4, 2, 4, 522 },  /* fr LOC.ESTACAO */
    { "gps", 3, 6, 0, 0 },  /* fr sensitive */
    { "hopital", 7, 2, 4, 519 },  /* fr LOC.HOSPITAL */
    { "hier", 4, 2, 3, 781 },  /* fr TIME.ONTEM */
    { "il faut parler", 14, 2, 2, 58 },  /* fr EV.PRECISO_FALAR */
    { "impossible", 10, 2, 2, 14 },  /* fr EV.IMPOSSIVEL */
    { "incendie", 8, 2, 2, 27 },  /* fr EV.FOGO */
    { "ignore", 6, 7, 0, 0 },  /* fr authority */
    { "je peux pas parler", 18, 2, 2, 42 },  /* fr EV.NAO_POSSO_FALAR */
    { "je suis arrive", 14, 2, 2, 1 },  /* fr EV.CHEGUEI */
    { "je vais dormir", 14, 2, 2, 37 },  /* fr EV.DORMINDO */
    { "je suis perdu", 13, 2, 2, 11 },  /* fr EV.PERDIDO */
    { "je travaille", 12, 2, 2, 39 },  /* fr EV.TRABALHANDO */
    { "je peux pas", 11, 2, 2, 14 },  /* fr EV.IMPOSSIVEL */
    { "je cherche", 10, 2, 2, 46 },  /* fr EV.PROCURANDO */
    { "je reviens", 10, 2, 2, 55 },  /* fr EV.VOLTANDO */
    { "jai fini", 8, 2, 2, 9 },  /* fr EV.TERMINEI */
    { "jattends", 8, 2, 2, 6 },  /* fr EV.ESPERANDO */
    { "je taime", 8, 2, 2, 17 },  /* fr EV.TE_AMO */
    { "jai mal", 7, 2, 2, 30 },  /* fr EV.DOR */
    { "jarrive", 7, 2, 2, 8 },  /* fr EV.A_CAMINHO */
    { "je part", 7, 2, 2, 2 },  /* fr EV.SAINDO */
    { "je dor", 6, 2, 2, 37 },  /* fr EV.DORMINDO */
    { "joao", 4, 2, 1, 256 },  /* fr PER.JOAO */
    { "je", 2, 0, 0, 0 },  /* fr stop */
    { "localisation", 12, 6, 0, 0 },  /* fr sensitive */
    { "le matin", 8, 2, 3, 779 },  /* fr TIME.MANHA */
    { "les", 3, 0, 0, 0 },  /* fr stop */
    { "la", 2, 0, 0, 0 },  /* fr stop */
    { "le", 2, 0, 0, 0 },  /* fr stop */
    { "maintenant", 10, 2, 3, 768 },  /* fr TIME.AGORA */
    { "motdepasse", 10, 6, 0, 0 },  /* fr sensitive */
    { "magasin", 7, 2, 4, 520 },  /* fr LOC.MERCADO */
    { "medecin", 7, 2, 2, 29 },  /* fr EV.MEDICO */
    { "maison", 6, 2, 4, 512 },  /* fr LOC.CASA */
    { "marche", 6, 2, 4, 520 },  /* fr LOC.MERCADO */
    { "moitie", 6, 2, 5, 1036 },  /* fr QTY.METADE */
    { "maman", 5, 2, 1, 262 },  /* fr PER.MAE */
    { "maria", 5, 2, 1, 257 },  /* fr PER.MARIA */
    { "merci", 5, 2, 2, 15 },  /* fr EV.OBRIGADO */
    { "mere", 4, 2, 1, 262 },  /* fr PER.MAE */
    { "mal", 3, 2, 6, 1281 },  /* fr ST.MAL */
    { "moi", 3, 0, 0, 0 },  /* fr stop */
    { "mon", 3, 0, 0, 0 },  /* fr stop */
    { "ma", 2, 0, 0, 0 },  /* fr stop */
    { "me", 2, 0, 0, 0 },  /* fr stop */
    { "nourriture", 10, 2, 2, 32 },  /* fr EV.COMIDA */
    { "note", 4, 1, 0, 3 },  /* fr op LEMBRAR */
    { "non", 3, 3, 0, 0 },  /* fr negation */
    { "on se voit", 10, 2, 2, 7 },  /* fr EV.ENCONTRAR */
    { "oui", 3, 2, 2, 13 },  /* fr EV.SIM */
    { "ou", 2, 5, 4, 0 },  /* fr qword ONDE */
    { "point de rencontre", 18, 2, 4, 528 },  /* fr LOC.PONTO_ENCONTRO */
    { "plus de batterie", 16, 2, 2, 10 },  /* fr EV.SEM_BATERIA */
    { "pas de probleme", 15, 2, 2, 4 },  /* fr EV.TUDO_BEM */
    { "pas de reseau", 13, 2, 2, 44 },  /* fr EV.SEM_SINAL */
    { "pas compris", 11, 2, 2, 56 },  /* fr EV.NAO_ENTENDI */
    { "plan change", 11, 2, 2, 54 },  /* fr EV.PLANO_MUDOU */
    { "porte nord", 10, 2, 4, 514 },  /* fr LOC.PORTAO_N */
    { "plus tard", 9, 2, 3, 782 },  /* fr TIME.DEPOIS */
    { "paiement", 8, 2, 2, 48 },  /* fr EV.PAGAMENTO */
    { "planifie", 8, 1, 0, 4 },  /* fr op PLANEJAR */
    { "prevenir", 8, 1, 0, 1 },  /* fr op COMUNICAR */
    { "previens", 8, 1, 0, 1 },  /* fr op COMUNICAR */
    { "prudence", 8, 2, 2, 22 },  /* fr EV.CUIDADO */
    { "parking", 7, 2, 4, 530 },  /* fr LOC.ESTACIONAMENTO */
    { "pardon", 6, 2, 2, 16 },  /* fr EV.DESCULPA */
    { "police", 6, 2, 2, 28 },  /* fr EV.POLICIA */
    { "payer", 5, 2, 2, 48 },  /* fr EV.PAGAMENTO */
    { "perdu", 5, 2, 2, 11 },  /* fr EV.PERDIDO */
    { "papa", 4, 2, 1, 263 },  /* fr PER.PAI */
    { "pere", 4, 2, 1, 263 },  /* fr PER.PAI */
    { "pour", 4, 0, 0, 0 },  /* fr stop */
    { "pret", 4, 2, 2, 51 },  /* fr EV.PRONTO */
    { "pas", 3, 3, 0, 0 },  /* fr negation */
    { "peu", 3, 2, 5, 1034 },  /* fr QTY.POUCO */
    { "quelque chose ne va pas", 23, 2, 2, 59 },  /* fr EV.ALGO_ERRADO */
    { "quatre", 6, 2, 5, 1028 },  /* fr QTY.N_4 */
    { "quand", 5, 5, 3, 0 },  /* fr qword QUANDO */
    { "que", 3, 0, 0, 0 },  /* fr stop */
    { "qui", 3, 5, 1, 0 },  /* fr qword QUEM */
    { "recherche", 9, 2, 2, 46 },  /* fr EV.PROCURANDO */
    { "rencontre", 9, 2, 2, 7 },  /* fr EV.ENCONTRAR */
    { "rappelle", 8, 1, 0, 3 },  /* fr op LEMBRAR */
    { "reveille", 8, 2, 2, 38 },  /* fr EV.ACORDEI */
    { "reunion", 7, 2, 2, 40 },  /* fr EV.REUNIAO */
    { "riviere", 7, 2, 4, 525 },  /* fr LOC.RIO */
    { "repete", 6, 2, 2, 57 },  /* fr EV.REPETE */
    { "retour", 6, 2, 2, 55 },  /* fr EV.VOLTANDO */
    { "route", 5, 2, 4, 516 },  /* fr LOC.ESTRADA */
    { "recu", 4, 2, 2, 50 },  /* fr EV.RECEBI */
    { "sans confirmation", 17, 7, 0, 0 },  /* fr authority */
    { "salle de sport", 14, 2, 4, 515 },  /* fr LOC.TREINO */
    { "sans confirmer", 14, 7, 0, 0 },  /* fr authority */
    { "secouriste", 10, 2, 1, 268 },  /* fr PER.SOCORRISTA */
    { "secours", 7, 1, 0, 5 },  /* fr op SOCORRO */
    { "sentier", 7, 2, 4, 517 },  /* fr LOC.TRILHA */
    { "sommet", 6, 2, 4, 526 },  /* fr LOC.CUME */
    { "stable", 6, 2, 6, 1286 },  /* fr ST.ESTAVEL */
    { "salut", 5, 2, 2, 20 },  /* fr EV.BOM_DIA */
    { "seul", 4, 2, 6, 1287 },  /* fr ST.SOZINHO_ST */
    { "stop", 4, 2, 2, 23 },  /* fr EV.PARE */
    { "suis", 4, 0, 0, 0 },  /* fr stop */
    { "sil", 3, 0, 0, 0 },  /* fr stop */
    { "transcription", 13, 6, 0, 0 },  /* fr sensitive */
    { "tout va bien", 12, 2, 2, 4 },  /* fr EV.TUDO_BEM */
    { "tu me manque", 12, 2, 2, 18 },  /* fr EV.SAUDADE */
    { "termine", 7, 2, 2, 9 },  /* fr EV.TERMINEI */
    { "trouve", 6, 2, 2, 45 },  /* fr EV.ACHEI */
    { "total", 5, 2, 5, 1037 },  /* fr QTY.TOTAL */
    { "trois", 5, 2, 5, 1027 },  /* fr QTY.N_3 */
    { "tous", 4, 2, 1, 261 },  /* fr PER.TODOS */
    { "te", 2, 0, 0, 0 },  /* fr stop */
    { "urgence", 7, 1, 0, 5 },  /* fr op SOCORRO */
    { "urgent", 6, 4, 0, 2 },  /* fr urgency URGENTE */
    { "une", 3, 0, 0, 0 },  /* fr stop */
    { "un", 2, 0, 0, 0 },  /* fr stop */
    { "viens ici", 9, 2, 2, 24 },  /* fr EV.VEM */
    { "voiture", 7, 2, 4, 523 },  /* fr LOC.VEICULO */
    { "voisin", 6, 2, 1, 269 },  /* fr PER.VIZINHO */
    { "viens", 5, 2, 2, 24 },  /* fr EV.VEM */
    { "vingt", 5, 2, 5, 1031 },  /* fr QTY.N_20 */
    { "vite", 4, 4, 0, 1 },  /* fr urgency ATENCAO */
    { "vous", 4, 0, 0, 0 },  /* fr stop */
    { "week end", 8, 2, 3, 784 },  /* fr TIME.FIM_DE_SEMANA */
    { "0", 1, 2, 5, 1024 },  /* it QTY.N_0 */
    { "10 minuti", 9, 2, 3, 770 },  /* it TIME.MIN_10 */
    { "15 minuti", 9, 2, 3, 777 },  /* it TIME.MIN_15 */
    { "1 minuto", 8, 2, 3, 776 },  /* it TIME.MIN_1 */
    { "1 ora", 5, 2, 3, 772 },  /* it TIME.HORA_1 */
    { "100", 3, 2, 5, 1033 },  /* it QTY.N_100 */
    { "10", 2, 2, 5, 1030 },  /* it QTY.N_10 */
    { "1", 1, 2, 5, 1025 },  /* it QTY.N_1 */
    { "2 ore", 5, 2, 3, 778 },  /* it TIME.HORA_2 */
    { "20", 2, 2, 5, 1031 },  /* it QTY.N_20 */
    { "2", 1, 2, 5, 1026 },  /* it QTY.N_2 */
    { "30 minuti", 9, 2, 3, 771 },  /* it TIME.MIN_30 */
    { "3", 1, 2, 5, 1027 },  /* it QTY.N_3 */
    { "4", 1, 2, 5, 1028 },  /* it QTY.N_4 */
    { "5 minuti", 8, 2, 3, 769 },  /* it TIME.MIN_5 */
    { "50", 2, 2, 5, 1032 },  /* it QTY.N_50 */
    { "5", 1, 2, 5, 1029 },  /* it QTY.N_5 */
    { "automaticamente", 15, 7, 0, 0 },  /* it authority */
    { "accompagnato", 12, 2, 6, 1288 },  /* it ST.ACOMPANHADO */
    { "affermativo", 11, 2, 2, 13 },  /* it EV.SIM */
    { "arrivederci", 11, 2, 2, 21 },  /* it EV.BOA_NOITE */
    { "attenzione", 10, 2, 2, 22 },  /* it EV.CUIDADO */
    { "aeroporto", 9, 2, 4, 521 },  /* it LOC.AEROPORTO */
    { "al lavoro", 9, 2, 2, 39 },  /* it EV.TRABALHANDO */
    { "al sicuro", 9, 2, 6, 1282 },  /* it ST.SEGURO */
    { "ambulanza", 9, 2, 2, 29 },  /* it EV.MEDICO */
    { "annullato", 9, 2, 2, 53 },  /* it EV.CANCELADO */
    { "aspettami", 9, 2, 2, 25 },  /* it EV.ESPERA */
    { "arrivato", 8, 2, 2, 1 },  /* it EV.CHEGUEI */
    { "a breve", 7, 2, 3, 785 },  /* it TIME.EM_BREVE */
    { "annulla", 7, 1, 0, 7 },  /* it op CANCELAR */
    { "aspetta", 7, 2, 2, 25 },  /* it EV.ESPERA */
    { "aspetto", 7, 2, 2, 6 },  /* it EV.ESPERANDO */
    { "adesso", 6, 2, 3, 768 },  /* it TIME.AGORA */
    { "annota", 6, 1, 0, 3 },  /* it op LEMBRAR */
    { "avvisa", 6, 1, 0, 1 },  /* it op COMUNICAR */
    { "acqua", 5, 2, 2, 31 },  /* it EV.AGUA */
    { "aiuto", 5, 2, 2, 5 },  /* it EV.AJUDA */
    { "amico", 5, 2, 1, 265 },  /* it PER.AMIGO */
    { "audio", 5, 6, 0, 0 },  /* it sensitive */
    { "auto", 4, 2, 4, 523 },  /* it LOC.VEICULO */
    { "ana", 3, 2, 1, 258 },  /* it PER.ANA */
    { "al", 2, 0, 0, 0 },  /* it stop */
    { "a", 1, 0, 0, 0 },  /* it stop */
    { "batteria scarica", 16, 2, 2, 10 },  /* it EV.SEM_BATERIA */
    { "bisogno di acqua", 16, 2, 2, 31 },  /* it EV.AGUA */
    { "buonanotte", 10, 2, 2, 21 },  /* it EV.BOA_NOITE */
    { "buongiorno", 10, 2, 2, 20 },  /* it EV.BOM_DIA */
    { "biometria", 9, 6, 0, 0 },  /* it sensitive */
    { "bloccato", 8, 2, 6, 1289 },  /* it ST.PRESO */
    { "bravo", 5, 2, 2, 19 },  /* it EV.PARABENS */
    { "base", 4, 2, 4, 527 },  /* it LOC.BASE */
    { "bene", 4, 2, 6, 1280 },  /* it ST.BEM */
    { "congratulazioni", 15, 2, 2, 19 },  /* it EV.PARABENS */
    { "ci vediamo", 10, 2, 2, 7 },  /* it EV.ENCONTRAR */
    { "contatto 1", 10, 2, 1, 270 },  /* it PER.HANDLE_1 */
    { "contatto 2", 10, 2, 1, 271 },  /* it PER.HANDLE_2 */
    { "contatto 3", 10, 2, 1, 272 },  /* it PER.HANDLE_3 */
    { "contatto 4", 10, 2, 1, 273 },  /* it PER.HANDLE_4 */
    { "coordinata", 10, 6, 0, 0 },  /* it sensitive */
    { "coordinate", 10, 6, 0, 0 },  /* it sensitive */
    { "conteggio", 9, 2, 2, 60 },  /* it EV.CONTAGEM */
    { "cercando", 8, 2, 2, 46 },  /* it EV.PROCURANDO */
    { "chiamami", 8, 2, 2, 41 },  /* it EV.LIGA_PRA_MIM */
    { "comprato", 8, 2, 2, 47 },  /* it EV.COMPREI */
    { "comunica", 8, 1, 0, 1 },  /* it op COMUNICAR */
    { "conferma", 8, 1, 0, 6 },  /* it op CONFIRMAR */
    { "chiave", 6, 6, 0, 0 },  /* it sensitive */
    { "chiedi", 6, 1, 0, 2 },  /* it op PERGUNTAR */
    { "cinque", 6, 2, 5, 1029 },  /* it QTY.N_5 */
    { "caldo", 5, 2, 2, 35 },  /* it EV.CALOR */
    { "calmo", 5, 2, 6, 1290 },  /* it ST.CALMO */
    { "campo", 5, 2, 4, 524 },  /* it LOC.ACAMPAMENTO */
    { "carta", 5, 6, 0, 0 },  /* it sensitive */
    { "cento", 5, 2, 5, 1033 },  /* it QTY.N_100 */
    { "capo", 4, 2, 1, 266 },  /* it PER.CHEFE */
    { "casa", 4, 2, 4, 512 },  /* it LOC.CASA */
    { "ciao", 4, 2, 2, 20 },  /* it EV.BOM_DIA */
    { "cibo", 4, 2, 2, 32 },  /* it EV.COMIDA */
    { "cima", 4, 2, 4, 526 },  /* it LOC.CUME */
    { "che", 3, 0, 0, 0 },  /* it stop */
    { "chi", 3, 5, 1, 0 },  /* it qword QUEM */
    { "con", 3, 0, 0, 0 },  /* it stop */
    { "devo parlarti", 13, 2, 2, 58 },  /* it EV.PRECISO_FALAR */
    { "di ritorno", 10, 2, 2, 55 },  /* it EV.VOLTANDO */
    { "da solo", 7, 7, 0, 0 },  /* it authority */
    { "dolore", 6, 2, 2, 30 },  /* it EV.DOR */
    { "domani", 6, 2, 3, 774 },  /* it TIME.AMANHA */
    { "dieci", 5, 2, 5, 1030 },  /* it QTY.N_10 */
    { "digli", 5, 1, 0, 1 },  /* it op COMUNICAR */
    { "dille", 5, 1, 0, 1 },  /* it op COMUNICAR */
    { "dormo", 5, 2, 2, 37 },  /* it EV.DORMINDO */
    { "dopo", 4, 2, 3, 782 },  /* it TIME.DEPOIS */
    { "dove", 4, 5, 4, 0 },  /* it qword ONDE */
    { "del", 3, 0, 0, 0 },  /* it stop */
    { "due", 3, 2, 5, 1026 },  /* it QTY.N_2 */
    { "di", 2, 0, 0, 0 },  /* it stop */
    { "emergenza", 9, 1, 0, 5 },  /* it op SOCORRO */
    { "esausto", 7, 2, 2, 36 },  /* it EV.CANSADO */
    { "esco", 4, 2, 2, 2 },  /* it EV.SAINDO */
    { "e", 1, 0, 0, 0 },  /* it stop */
    { "fine settimana", 14, 2, 3, 784 },  /* it TIME.FIM_DE_SEMANA */
    { "famiglia", 8, 2, 1, 260 },  /* it PER.FAMILIA */
    { "fermati", 7, 2, 2, 23 },  /* it EV.PARE */
    { "favore", 6, 0, 0, 0 },  /* it stop */
    { "ferito", 6, 2, 6, 1284 },  /* it ST.FERIDO */
    { "figlia", 6, 2, 1, 264 },  /* it PER.FILHO */
    { "figlio", 6, 2, 1, 264 },  /* it PER.FILHO */
    { "finito", 6, 2, 2, 9 },  /* it EV.TERMINEI */
    { "freddo", 6, 2, 2, 34 },  /* it EV.FRIO */
    { "fiume", 5, 2, 4, 525 },  /* it LOC.RIO */
    { "fuoco", 5, 2, 2, 27 },  /* it EV.FOGO */
    { "fame", 4, 2, 2, 32 },  /* it EV.COMIDA */
    { "gruppo 1", 8, 2, 1, 274 },  /* it PER.GRUPO_1 */
    { "gruppo 2", 8, 2, 1, 275 },  /* it PER.GRUPO_2 */
    { "grazie", 6, 2, 2, 15 },  /* it EV.OBRIGADO */
    { "guasto", 6, 2, 2, 43 },  /* it EV.VEICULO_QUEBROU */
    { "grave", 5, 2, 6, 1285 },  /* it ST.GRAVE */
    { "guida", 5, 2, 1, 267 },  /* it PER.GUIA */
    { "gia", 3, 0, 0, 0 },  /* it stop */
    { "gli", 3, 0, 0, 0 },  /* it stop */
    { "gps", 3, 6, 0, 0 },  /* it sensitive */
    { "ho bisogno di aiuto", 19, 2, 2, 5 },  /* it EV.AJUDA */
    { "ho finito", 9, 2, 2, 9 },  /* it EV.TERMINEI */
    { "impossibile", 11, 2, 2, 14 },  /* it EV.IMPOSSIVEL */
    { "in pericolo", 11, 2, 6, 1283 },  /* it ST.EM_PERIGO */
    { "in ritardo", 10, 2, 2, 3 },  /* it EV.ATRASADO */
    { "in arrivo", 9, 2, 2, 8 },  /* it EV.A_CAMINHO */
    { "in attesa", 9, 2, 2, 6 },  /* it EV.ESPERANDO */
    { "incidente", 9, 2, 2, 26 },  /* it EV.ACIDENTE */
    { "incendio", 8, 2, 2, 27 },  /* it EV.FOGO */
    { "incontro", 8, 2, 2, 7 },  /* it EV.ENCONTRAR */
    { "ingresso", 8, 2, 4, 529 },  /* it LOC.ENTRADA */
    { "iniziato", 8, 2, 2, 52 },  /* it EV.COMECOU */
    { "inviato", 7, 2, 2, 49 },  /* it EV.ENVIEI */
    { "ignora", 6, 7, 0, 0 },  /* it authority */
    { "ieri", 4, 2, 3, 781 },  /* it TIME.ONTEM */
    { "il", 2, 0, 0, 0 },  /* it stop */
    { "in", 2, 0, 0, 0 },  /* it stop */
    { "io", 2, 0, 0, 0 },  /* it stop */
    { "i", 1, 0, 0, 0 },  /* it stop */
    { "joao", 4, 2, 1, 256 },  /* it PER.JOAO */
    { "lavorando", 9, 2, 2, 39 },  /* it EV.TRABALHANDO */
    { "la", 2, 0, 0, 0 },  /* it stop */
    { "le", 2, 0, 0, 0 },  /* it stop */
    { "lo", 2, 0, 0, 0 },  /* it stop */
    { "mi sono perso", 13, 2, 2, 11 },  /* it EV.PERDIDO */
    { "mi dispiace", 11, 2, 2, 16 },  /* it EV.DESCULPA */
    { "mi fa male", 10, 2, 2, 30 },  /* it EV.DOR */
    { "mi manchi", 9, 2, 2, 18 },  /* it EV.SAUDADE */
    { "mattina", 7, 2, 3, 779 },  /* it TIME.MANHA */
    { "mercato", 7, 2, 4, 520 },  /* it LOC.MERCADO */
    { "medico", 6, 2, 2, 29 },  /* it EV.MEDICO */
    { "madre", 5, 2, 1, 262 },  /* it PER.MAE */
    { "mamma", 5, 2, 1, 262 },  /* it PER.MAE */
    { "maria", 5, 2, 1, 257 },  /* it PER.MARIA */
    { "molto", 5, 2, 5, 1035 },  /* it QTY.MUITO */
    { "male", 4, 2, 6, 1281 },  /* it ST.MAL */
    { "meta", 4, 2, 5, 1036 },  /* it QTY.METADE */
    { "me", 2, 0, 0, 0 },  /* it stop */
    { "mi", 2, 0, 0, 0 },  /* it stop */
    { "non posso parlare", 17, 2, 2, 42 },  /* it EV.NAO_POSSO_FALAR */
    { "nessun problema", 15, 2, 2, 4 },  /* it EV.TUDO_BEM */
    { "non ho capito", 13, 2, 2, 56 },  /* it EV.NAO_ENTENDI */
    { "non posso", 9, 2, 2, 14 },  /* it EV.IMPOSSIVEL */
    { "negozio", 7, 2, 4, 520 },  /* it LOC.MERCADO */
    { "nessuno", 7, 2, 5, 1024 },  /* it QTY.N_0 */
    { "non", 3, 3, 0, 0 },  /* it negation */
    { "no", 2, 3, 0, 0 },  /* it negation */
    { "ogni volta", 10, 7, 0, 0 },  /* it authority */
    { "ospedale", 8, 2, 4, 519 },  /* it LOC.HOSPITAL */
    { "oggi", 4, 2, 3, 773 },  /* it TIME.HOJE */
    { "punto di incontro", 17, 2, 4, 528 },  /* it LOC.PONTO_ENCONTRO */
    { "piano cambiato", 14, 2, 2, 54 },  /* it EV.PLANO_MUDOU */
    { "parcheggio", 10, 2, 4, 530 },  /* it LOC.ESTACIONAMENTO */
    { "pomeriggio", 10, 2, 3, 780 },  /* it TIME.TARDE_DIA */
    { "porta nord", 10, 2, 4, 514 },  /* it LOC.PORTAO_N */
    { "pagamento", 9, 2, 2, 48 },  /* it EV.PAGAMENTO */
    { "pianifica", 9, 1, 0, 4 },  /* it op PLANEJAR */
    { "piu tardi", 9, 2, 3, 782 },  /* it TIME.DEPOIS */
    { "posizione", 9, 6, 0, 0 },  /* it sensitive */
    { "palestra", 8, 2, 4, 515 },  /* it LOC.TREINO */
    { "password", 8, 6, 0, 0 },  /* it sensitive */
    { "prudenza", 8, 2, 2, 22 },  /* it EV.CUIDADO */
    { "polizia", 7, 2, 2, 28 },  /* it EV.POLICIA */
    { "pagare", 6, 2, 2, 48 },  /* it EV.PAGAMENTO */
    { "presto", 6, 4, 0, 1 },  /* it urgency ATENCAO */
    { "pronto", 6, 2, 2, 51 },  /* it EV.PRONTO */
    { "padre", 5, 2, 1, 263 },  /* it PER.PAI */
    { "perso", 5, 2, 2, 11 },  /* it EV.PERDIDO */
    { "papa", 4, 2, 1, 263 },  /* it PER.PAI */
    { "poco", 4, 2, 5, 1034 },  /* it QTY.POUCO */
    { "per", 3, 0, 0, 0 },  /* it stop */
    { "questa settimana", 16, 2, 3, 783 },  /* it TIME.SEMANA */
    { "qualcosa non va", 15, 2, 2, 59 },  /* it EV.ALGO_ERRADO */
    { "quattro", 7, 2, 5, 1028 },  /* it QTY.N_4 */
    { "quando", 6, 5, 3, 0 },  /* it qword QUANDO */
    { "quanti", 6, 5, 5, 0 },  /* it qword QUANTO */
    { "registrazione", 13, 6, 0, 0 },  /* it sensitive */
    { "ricevuto", 8, 2, 2, 50 },  /* it EV.RECEBI */
    { "riunione", 8, 2, 2, 40 },  /* it EV.REUNIAO */
    { "ricorda", 7, 1, 0, 3 },  /* it op LEMBRAR */
    { "riparo", 6, 2, 2, 33 },  /* it EV.ABRIGO */
    { "ripeti", 6, 2, 2, 57 },  /* it EV.REPETE */
    { "senza confermare", 16, 7, 0, 0 },  /* it authority */
    { "senza conferma", 14, 7, 0, 0 },  /* it authority */
    { "senza segnale", 13, 2, 2, 44 },  /* it EV.SEM_SINAL */
    { "sono arrivato", 13, 2, 2, 1 },  /* it EV.CHEGUEI */
    { "soccorritore", 12, 2, 1, 268 },  /* it PER.SOCORRISTA */
    { "sto uscendo", 11, 2, 2, 2 },  /* it EV.SAINDO */
    { "sto venendo", 11, 2, 2, 8 },  /* it EV.A_CAMINHO */
    { "spaventato", 10, 2, 6, 1291 },  /* it ST.COM_MEDO */
    { "svegliato", 9, 2, 2, 38 },  /* it EV.ACORDEI */
    { "sentiero", 8, 2, 4, 517 },  /* it LOC.TRILHA */
    { "soccorso", 8, 1, 0, 5 },  /* it op SOCORRO */
    { "stazione", 8, 2, 4, 522 },  /* it LOC.ESTACAO */
    { "studiare", 8, 2, 2, 12 },  /* it EV.ESTUDAR */
    { "squadra", 7, 2, 1, 259 },  /* it PER.EQUIPE */
    { "stabile", 7, 2, 6, 1286 },  /* it ST.ESTAVEL */
    { "stasera", 7, 2, 3, 775 },  /* it TIME.NOITE */
    { "sveglio", 7, 2, 2, 38 },  /* it EV.ACORDEI */
    { "scuola", 6, 2, 4, 518 },  /* it LOC.ESCOLA */
    { "stanco", 6, 2, 2, 36 },  /* it EV.CANSADO */
    { "strada", 6, 2, 4, 516 },  /* it LOC.ESTRADA */
    { "studio", 6, 2, 2, 12 },  /* it EV.ESTUDAR */
    { "salva", 5, 1, 0, 3 },  /* it op LEMBRAR */
    { "scusa", 5, 2, 2, 16 },  /* it EV.DESCULPA */
    { "solo", 4, 2, 6, 1287 },  /* it ST.SOZINHO_ST */
    { "sono", 4, 0, 0, 0 },  /* it stop */
    { "stop", 4, 2, 2, 23 },  /* it EV.PARE */
    { "si", 2, 2, 2, 13 },  /* it EV.SIM */
    { "su", 2, 0, 0, 0 },  /* it stop */
    { "trascrizione", 12, 6, 0, 0 },  /* it sensitive */
    { "tutto bene", 10, 2, 2, 4 },  /* it EV.TUDO_BEM */
    { "trovato", 7, 2, 2, 45 },  /* it EV.ACHEI */
    { "ti amo", 6, 2, 2, 17 },  /* it EV.TE_AMO */
    { "totale", 6, 2, 5, 1037 },  /* it QTY.TOTAL */
    { "torno", 5, 2, 2, 55 },  /* it EV.VOLTANDO */
    { "tutti", 5, 2, 1, 261 },  /* it PER.TODOS */
    { "tre", 3, 2, 5, 1027 },  /* it QTY.N_3 */
    { "ti", 2, 0, 0, 0 },  /* it stop */
    { "ufficio", 7, 2, 4, 513 },  /* it LOC.TRABALHO */
    { "urgente", 7, 4, 0, 2 },  /* it urgency URGENTE */
    { "una", 3, 0, 0, 0 },  /* it stop */
    { "uno", 3, 0, 0, 0 },  /* it stop */
    { "un", 2, 0, 0, 0 },  /* it stop */
    { "vado a dormire", 14, 2, 2, 37 },  /* it EV.DORMINDO */
    { "veicolo rotto", 13, 2, 2, 43 },  /* it EV.VEICULO_QUEBROU */
    { "vieni qui", 9, 2, 2, 24 },  /* it EV.VEM */
    { "vicino", 6, 2, 1, 269 },  /* it PER.VIZINHO */
    { "venti", 5, 2, 5, 1031 },  /* it QTY.N_20 */
    { "vieni", 5, 2, 2, 24 },  /* it EV.VEM */
    { "0", 1, 2, 5, 1024 },  /* de QTY.N_0 */
    { "10 minuten", 10, 2, 3, 770 },  /* de TIME.MIN_10 */
    { "15 minuten", 10, 2, 3, 777 },  /* de TIME.MIN_15 */
    { "1 minute", 8, 2, 3, 776 },  /* de TIME.MIN_1 */
    { "1 stunde", 8, 2, 3, 772 },  /* de TIME.HORA_1 */
    { "100", 3, 2, 5, 1033 },  /* de QTY.N_100 */
    { "10", 2, 2, 5, 1030 },  /* de QTY.N_10 */
    { "1", 1, 2, 5, 1025 },  /* de QTY.N_1 */
    { "2 stunden", 9, 2, 3, 778 },  /* de TIME.HORA_2 */
    { "20", 2, 2, 5, 1031 },  /* de QTY.N_20 */
    { "2", 1, 2, 5, 1026 },  /* de QTY.N_2 */
    { "30 minuten", 10, 2, 3, 771 },  /* de TIME.MIN_30 */
    { "3", 1, 2, 5, 1027 },  /* de QTY.N_3 */
    { "4", 1, 2, 5, 1028 },  /* de QTY.N_4 */
    { "5 minuten", 9, 2, 3, 769 },  /* de TIME.MIN_5 */
    { "50", 2, 2, 5, 1032 },  /* de QTY.N_50 */
    { "5", 1, 2, 5, 1029 },  /* de QTY.N_5 */
    { "auf dem ruckweg", 15, 2, 2, 55 },  /* de EV.VOLTANDO */
    { "auf dem weg", 11, 2, 2, 8 },  /* de EV.A_CAMINHO */
    { "automatisch", 11, 7, 0, 0 },  /* de authority */
    { "angekommen", 10, 2, 2, 1 },  /* de EV.CHEGUEI */
    { "aufgewacht", 10, 2, 2, 38 },  /* de EV.ACORDEI */
    { "abbrechen", 9, 1, 0, 7 },  /* de op CANCELAR */
    { "akku leer", 9, 2, 2, 10 },  /* de EV.SEM_BATERIA */
    { "alles gut", 9, 2, 2, 4 },  /* de EV.TUDO_BEM */
    { "am suchen", 9, 2, 2, 46 },  /* de EV.PROCURANDO */
    { "abgesagt", 8, 2, 2, 53 },  /* de EV.CANCELADO */
    { "aufnahme", 8, 6, 0, 0 },  /* de sensitive */
    { "achtung", 7, 2, 2, 22 },  /* de EV.CUIDADO */
    { "arbeite", 7, 2, 2, 39 },  /* de EV.TRABALHANDO */
    { "allein", 6, 2, 6, 1287 },  /* de ST.SOZINHO_ST */
    { "anzahl", 6, 2, 2, 60 },  /* de EV.CONTAGEM */
    { "angst", 5, 2, 6, 1291 },  /* de ST.COM_MEDO */
    { "audio", 5, 6, 0, 0 },  /* de sensitive */
    { "alle", 4, 2, 1, 261 },  /* de PER.TODOS */
    { "arzt", 4, 2, 2, 29 },  /* de EV.MEDICO */
    { "auto", 4, 2, 4, 523 },  /* de LOC.VEICULO */
    { "ana", 3, 2, 1, 258 },  /* de PER.ANA */
    { "am", 2, 0, 0, 0 },  /* de stop */
    { "an", 2, 0, 0, 0 },  /* de stop */
    { "bei der arbeit", 14, 2, 2, 39 },  /* de EV.TRABALHANDO */
    { "benachrichtige", 14, 1, 0, 1 },  /* de op COMUNICAR */
    { "brauche wasser", 14, 2, 2, 31 },  /* de EV.AGUA */
    { "brauche hilfe", 13, 2, 2, 5 },  /* de EV.AJUDA */
    { "besprechung", 11, 2, 2, 40 },  /* de EV.REUNIAO */
    { "begleitet", 9, 2, 6, 1288 },  /* de ST.ACOMPANHADO */
    { "bestatige", 9, 1, 0, 6 },  /* de op CONFIRMAR */
    { "biometrie", 9, 6, 0, 0 },  /* de sensitive */
    { "begonnen", 8, 2, 2, 52 },  /* de EV.COMECOU */
    { "bekommen", 8, 2, 2, 50 },  /* de EV.RECEBI */
    { "bezahlen", 8, 2, 2, 48 },  /* de EV.PAGAMENTO */
    { "bahnhof", 7, 2, 4, 522 },  /* de LOC.ESTACAO */
    { "bereit", 6, 2, 2, 51 },  /* de EV.PRONTO */
    { "basis", 5, 2, 4, 527 },  /* de LOC.BASE */
    { "bitte", 5, 0, 0, 0 },  /* de stop */
    { "brand", 5, 2, 2, 27 },  /* de EV.FOGO */
    { "bald", 4, 2, 3, 785 },  /* de TIME.EM_BREVE */
    { "buro", 4, 2, 4, 513 },  /* de LOC.TRABALHO */
    { "bin", 3, 0, 0, 0 },  /* de stop */
    { "chef", 4, 2, 1, 266 },  /* de PER.CHEFE */
    { "diese woche", 11, 2, 3, 783 },  /* de TIME.SEMANA */
    { "dringend", 8, 4, 0, 2 },  /* de urgency URGENTE */
    { "danke", 5, 2, 2, 15 },  /* de EV.OBRIGADO */
    { "dass", 4, 0, 0, 0 },  /* de stop */
    { "drei", 4, 2, 5, 1027 },  /* de QTY.N_3 */
    { "das", 3, 0, 0, 0 },  /* de stop */
    { "der", 3, 0, 0, 0 },  /* de stop */
    { "die", 3, 0, 0, 0 },  /* de stop */
    { "du", 2, 0, 0, 0 },  /* de stop */
    { "etwas stimmt nicht", 18, 2, 2, 59 },  /* de EV.ALGO_ERRADO */
    { "eingeschlossen", 14, 2, 6, 1289 },  /* de ST.PRESO */
    { "entschuldigung", 14, 2, 2, 16 },  /* de EV.DESCULPA */
    { "einverstanden", 13, 2, 2, 13 },  /* de EV.SIM */
    { "es tut weh", 10, 2, 2, 30 },  /* de EV.DOR */
    { "erschopft", 9, 2, 2, 36 },  /* de EV.CANSADO */
    { "erhalten", 8, 2, 2, 50 },  /* de EV.RECEBI */
    { "erinnere", 8, 1, 0, 3 },  /* de op LEMBRAR */
    { "erledigt", 8, 2, 2, 9 },  /* de EV.TERMINEI */
    { "eingang", 7, 2, 4, 529 },  /* de LOC.ENTRADA */
    { "einen", 5, 0, 0, 0 },  /* de stop */
    { "essen", 5, 2, 2, 32 },  /* de EV.COMIDA */
    { "eine", 4, 0, 0, 0 },  /* de stop */
    { "ein", 3, 0, 0, 0 },  /* de stop */
    { "fahrzeug kaputt", 15, 2, 2, 43 },  /* de EV.VEICULO_QUEBROU */
    { "fitnessstudio", 13, 2, 4, 515 },  /* de LOC.TREINO */
    { "flughafen", 9, 2, 4, 521 },  /* de LOC.AEROPORTO */
    { "familie", 7, 2, 1, 260 },  /* de PER.FAMILIA */
    { "fertig", 6, 2, 2, 9 },  /* de EV.TERMINEI */
    { "freund", 6, 2, 1, 265 },  /* de PER.AMIGO */
    { "fuhrer", 6, 2, 1, 267 },  /* de PER.GUIA */
    { "feuer", 5, 2, 2, 27 },  /* de EV.FOGO */
    { "fluss", 5, 2, 4, 525 },  /* de LOC.RIO */
    { "frage", 5, 1, 0, 2 },  /* de op PERGUNTAR */
    { "funf", 4, 2, 5, 1029 },  /* de QTY.N_5 */
    { "fur", 3, 0, 0, 0 },  /* de stop */
    { "gehe schlafen", 13, 2, 2, 37 },  /* de EV.DORMINDO */
    { "guten morgen", 12, 2, 2, 20 },  /* de EV.BOM_DIA */
    { "gratulation", 11, 2, 2, 19 },  /* de EV.PARABENS */
    { "gute nacht", 10, 2, 2, 21 },  /* de EV.BOA_NOITE */
    { "gestartet", 9, 2, 2, 52 },  /* de EV.COMECOU */
    { "gefunden", 8, 2, 2, 45 },  /* de EV.ACHEI */
    { "gesendet", 8, 2, 2, 49 },  /* de EV.ENVIEI */
    { "gruppe 1", 8, 2, 1, 274 },  /* de PER.GRUPO_1 */
    { "gruppe 2", 8, 2, 1, 275 },  /* de PER.GRUPO_2 */
    { "gekauft", 7, 2, 2, 47 },  /* de EV.COMPREI */
    { "gestern", 7, 2, 3, 781 },  /* de TIME.ONTEM */
    { "gesamt", 6, 2, 5, 1037 },  /* de QTY.TOTAL */
    { "gipfel", 6, 2, 4, 526 },  /* de LOC.CUME */
    { "gps", 3, 6, 0, 0 },  /* de sensitive */
    { "gut", 3, 2, 6, 1280 },  /* de ST.BEM */
    { "herzlichen gluckwunsch", 22, 2, 2, 19 },  /* de EV.PARABENS */
    { "heute abend", 11, 2, 3, 775 },  /* de TIME.NOITE */
    { "hundert", 7, 2, 5, 1033 },  /* de QTY.N_100 */
    { "halfte", 6, 2, 5, 1036 },  /* de QTY.METADE */
    { "hunger", 6, 2, 2, 32 },  /* de EV.COMIDA */
    { "hallo", 5, 2, 2, 20 },  /* de EV.BOM_DIA */
    { "heiss", 5, 2, 2, 35 },  /* de EV.CALOR */
    { "heute", 5, 2, 3, 773 },  /* de TIME.HOJE */
    { "hilfe", 5, 2, 2, 5 },  /* de EV.AJUDA */
    { "hitze", 5, 2, 2, 35 },  /* de EV.CALOR */
    { "halt", 4, 2, 2, 23 },  /* de EV.PARE */
    { "haus", 4, 2, 4, 512 },  /* de LOC.CASA */
    { "ich habe mich verlaufen", 23, 2, 2, 11 },  /* de EV.PERDIDO */
    { "ich vermisse dich", 17, 2, 2, 18 },  /* de EV.SAUDADE */
    { "ich liebe dich", 14, 2, 2, 17 },  /* de EV.TE_AMO */
    { "ich bin da", 10, 2, 2, 1 },  /* de EV.CHEGUEI */
    { "immer wenn", 10, 7, 0, 0 },  /* de authority */
    { "ich warte", 9, 2, 2, 6 },  /* de EV.ESPERANDO */
    { "ignoriere", 9, 7, 0, 0 },  /* de authority */
    { "in gefahr", 9, 2, 6, 1283 },  /* de ST.EM_PERIGO */
    { "ich gehe", 8, 2, 2, 2 },  /* de EV.SAINDO */
    { "ich", 3, 0, 0, 0 },  /* de stop */
    { "ist", 3, 0, 0, 0 },  /* de stop */
    { "im", 2, 0, 0, 0 },  /* de stop */
    { "in", 2, 0, 0, 0 },  /* de stop */
    { "jetzt", 5, 2, 3, 768 },  /* de TIME.AGORA */
    { "joao", 4, 2, 1, 256 },  /* de PER.JOAO */
    { "ja", 2, 2, 2, 13 },  /* de EV.SIM */
    { "kann nicht sprechen", 19, 2, 2, 42 },  /* de EV.NAO_POSSO_FALAR */
    { "kein problem", 12, 2, 2, 4 },  /* de EV.TUDO_BEM */
    { "komme spater", 12, 2, 2, 3 },  /* de EV.ATRASADO */
    { "komme zuruck", 12, 2, 2, 55 },  /* de EV.VOLTANDO */
    { "krankenwagen", 12, 2, 2, 29 },  /* de EV.MEDICO */
    { "kein signal", 11, 2, 2, 44 },  /* de EV.SEM_SINAL */
    { "koordinaten", 11, 6, 0, 0 },  /* de sensitive */
    { "krankenhaus", 11, 2, 4, 519 },  /* de LOC.HOSPITAL */
    { "kann nicht", 10, 2, 2, 14 },  /* de EV.IMPOSSIVEL */
    { "koordinate", 10, 6, 0, 0 },  /* de sensitive */
    { "kein netz", 9, 2, 2, 44 },  /* de EV.SEM_SINAL */
    { "kontakt 1", 9, 2, 1, 270 },  /* de PER.HANDLE_1 */
    { "kontakt 2", 9, 2, 1, 271 },  /* de PER.HANDLE_2 */
    { "kontakt 3", 9, 2, 1, 272 },  /* de PER.HANDLE_3 */
    { "kontakt 4", 9, 2, 1, 273 },  /* de PER.HANDLE_4 */
    { "komm her", 8, 2, 2, 24 },  /* de EV.VEM */
    { "kritisch", 8, 2, 6, 1285 },  /* de ST.GRAVE */
    { "karte", 5, 6, 0, 0 },  /* de sensitive */
    { "keine", 5, 2, 5, 1024 },  /* de QTY.N_0 */
    { "komme", 5, 2, 2, 8 },  /* de EV.A_CAMINHO */
    { "kalt", 4, 2, 2, 34 },  /* de EV.FRIO */
    { "kein", 4, 3, 0, 0 },  /* de negation */
    { "kind", 4, 2, 1, 264 },  /* de PER.FILHO */
    { "komm", 4, 2, 2, 24 },  /* de EV.VEM */
    { "lernen", 6, 2, 2, 12 },  /* de EV.ESTUDAR */
    { "laden", 5, 2, 4, 520 },  /* de LOC.MERCADO */
    { "lager", 5, 2, 4, 524 },  /* de LOC.ACAMPAMENTO */
    { "muss reden", 10, 2, 2, 58 },  /* de EV.PRECISO_FALAR */
    { "meeting", 7, 2, 2, 40 },  /* de EV.REUNIAO */
    { "moment", 6, 2, 2, 25 },  /* de EV.ESPERA */
    { "morgen", 6, 2, 3, 774 },  /* de TIME.AMANHA */
    { "mutter", 6, 2, 1, 262 },  /* de PER.MAE */
    { "maria", 5, 2, 1, 257 },  /* de PER.MARIA */
    { "markt", 5, 2, 4, 520 },  /* de LOC.MERCADO */
    { "melde", 5, 1, 0, 1 },  /* de op COMUNICAR */
    { "merke", 5, 1, 0, 3 },  /* de op LEMBRAR */
    { "mama", 4, 2, 1, 262 },  /* de PER.MAE */
    { "mich", 4, 0, 0, 0 },  /* de stop */
    { "mude", 4, 2, 2, 36 },  /* de EV.CANSADO */
    { "mir", 3, 0, 0, 0 },  /* de stop */
    { "mit", 3, 0, 0, 0 },  /* de stop */
    { "nicht verstanden", 16, 2, 2, 56 },  /* de EV.NAO_ENTENDI */
    { "nachmittag", 10, 2, 3, 780 },  /* de TIME.TARDE_DIA */
    { "nachbar", 7, 2, 1, 269 },  /* de PER.VIZINHO */
    { "nordtor", 7, 2, 4, 514 },  /* de LOC.PORTAO_N */
    { "notfall", 7, 1, 0, 5 },  /* de op SOCORRO */
    { "notiere", 7, 1, 0, 3 },  /* de op LEMBRAR */
    { "notruf", 6, 1, 0, 5 },  /* de op SOCORRO */
    { "nicht", 5, 3, 0, 0 },  /* de negation */
    { "ohne zu bestatigen", 18, 7, 0, 0 },  /* de authority */
    { "ohne bestatigung", 16, 7, 0, 0 },  /* de authority */
    { "plan geandert", 13, 2, 2, 54 },  /* de EV.PLANO_MUDOU */
    { "personenzahl", 12, 2, 2, 60 },  /* de EV.CONTAGEM */
    { "parkplatz", 9, 2, 4, 530 },  /* de LOC.ESTACIONAMENTO */
    { "passwort", 8, 6, 0, 0 },  /* de sensitive */
    { "polizei", 7, 2, 2, 28 },  /* de EV.POLICIA */
    { "panne", 5, 2, 2, 43 },  /* de EV.VEICULO_QUEBROU */
    { "plane", 5, 1, 0, 4 },  /* de op PLANEJAR */
    { "papa", 4, 2, 1, 263 },  /* de PER.PAI */
    { "ruf mich an", 11, 2, 2, 41 },  /* de EV.LIGA_PRA_MIM */
    { "retter", 6, 2, 1, 268 },  /* de PER.SOCORRISTA */
    { "ruhig", 5, 2, 6, 1290 },  /* de ST.CALMO */
    { "schlussel", 9, 6, 0, 0 },  /* de sensitive */
    { "storniere", 9, 1, 0, 7 },  /* de op CANCELAR */
    { "schlecht", 8, 2, 6, 1281 },  /* de ST.MAL */
    { "standort", 8, 6, 0, 0 },  /* de sensitive */
    { "schlafe", 7, 2, 2, 37 },  /* de EV.DORMINDO */
    { "schmerz", 7, 2, 2, 30 },  /* de EV.DOR */
    { "schnell", 7, 4, 0, 1 },  /* de urgency ATENCAO */
    { "strasse", 7, 2, 4, 516 },  /* de LOC.ESTRADA */
    { "studium", 7, 2, 2, 12 },  /* de EV.ESTUDAR */
    { "schule", 6, 2, 4, 518 },  /* de LOC.ESCOLA */
    { "sicher", 6, 2, 6, 1282 },  /* de ST.SEGURO */
    { "spater", 6, 2, 3, 782 },  /* de TIME.DEPOIS */
    { "stabil", 6, 2, 6, 1286 },  /* de ST.ESTAVEL */
    { "schon", 5, 0, 0, 0 },  /* de stop */
    { "sorry", 5, 2, 2, 16 },  /* de EV.DESCULPA */
    { "stopp", 5, 2, 2, 23 },  /* de EV.PARE */
    { "suche", 5, 2, 2, 46 },  /* de EV.PROCURANDO */
    { "sage", 4, 1, 0, 1 },  /* de op COMUNICAR */
    { "sag", 3, 1, 0, 1 },  /* de op COMUNICAR */
    { "transkript", 10, 6, 0, 0 },  /* de sensitive */
    { "treffpunkt", 10, 2, 4, 528 },  /* de LOC.PONTO_ENCONTRO */
    { "treffen", 7, 2, 2, 7 },  /* de EV.ENCONTRAR */
    { "tschuss", 7, 2, 2, 21 },  /* de EV.BOA_NOITE */
    { "team", 4, 2, 1, 259 },  /* de PER.EQUIPE */
    { "unterwegs raus", 14, 2, 2, 2 },  /* de EV.SAINDO */
    { "unterkunft", 10, 2, 2, 33 },  /* de EV.ABRIGO */
    { "unmoglich", 9, 2, 2, 14 },  /* de EV.IMPOSSIVEL */
    { "unfall", 6, 2, 2, 26 },  /* de EV.ACIDENTE */
    { "uber", 4, 0, 0, 0 },  /* de stop */
    { "und", 3, 0, 0, 0 },  /* de stop */
    { "verschickt", 10, 2, 2, 49 },  /* de EV.ENVIEI */
    { "von selbst", 10, 7, 0, 0 },  /* de authority */
    { "verlaufen", 9, 2, 2, 11 },  /* de EV.PERDIDO */
    { "verspatet", 9, 2, 2, 3 },  /* de EV.ATRASADO */
    { "vormittag", 9, 2, 3, 779 },  /* de TIME.MANHA */
    { "verletzt", 8, 2, 6, 1284 },  /* de ST.FERIDO */
    { "vorsicht", 8, 2, 2, 22 },  /* de EV.CUIDADO */
    { "vater", 5, 2, 1, 263 },  /* de PER.PAI */
    { "viel", 4, 2, 5, 1035 },  /* de QTY.MUITO */
    { "vier", 4, 2, 5, 1028 },  /* de QTY.N_4 */
    { "wir treffen uns", 15, 2, 2, 7 },  /* de EV.ENCONTRAR */
    { "warte auf mich", 14, 2, 2, 25 },  /* de EV.ESPERA */
    { "wenig akku", 10, 2, 2, 10 },  /* de EV.SEM_BATERIA */
    { "wiederhole", 10, 2, 2, 57 },  /* de EV.REPETE */
    { "wochenende", 10, 2, 3, 784 },  /* de TIME.FIM_DE_SEMANA */
    { "wanderweg", 9, 2, 4, 517 },  /* de LOC.TRILHA */
    { "wie viele", 9, 5, 5, 0 },  /* de qword QUANTO */
    { "wasser", 6, 2, 2, 31 },  /* de EV.AGUA */
    { "warte", 5, 2, 2, 6 },  /* de EV.ESPERANDO */
    { "wenig", 5, 2, 5, 1034 },  /* de QTY.POUCO */
    { "wach", 4, 2, 2, 38 },  /* de EV.ACORDEI */
    { "wann", 4, 5, 3, 0 },  /* de qword QUANDO */
    { "wer", 3, 5, 1, 0 },  /* de qword QUEM */
    { "wo", 2, 5, 4, 0 },  /* de qword ONDE */
    { "zahlung", 7, 2, 2, 48 },  /* de EV.PAGAMENTO */
    { "zuhause", 7, 2, 4, 512 },  /* de LOC.CASA */
    { "zwanzig", 7, 2, 5, 1031 },  /* de QTY.N_20 */
    { "zehn", 4, 2, 5, 1030 },  /* de QTY.N_10 */
    { "zwei", 4, 2, 5, 1026 },  /* de QTY.N_2 */
    { "zu", 2, 0, 0, 0 },  /* de stop */
    { "0", 1, 2, 5, 1024 },  /* ja QTY.N_0 */
    { "1\346\231\202\351\226\223", 7, 2, 3, 772 },  /* ja TIME.HORA_1 */
    { "10\345\210\206", 5, 2, 3, 770 },  /* ja TIME.MIN_10 */
    { "15\345\210\206", 5, 2, 3, 777 },  /* ja TIME.MIN_15 */
    { "1\345\210\206", 4, 2, 3, 776 },  /* ja TIME.MIN_1 */
    { "100", 3, 2, 5, 1033 },  /* ja QTY.N_100 */
    { "10", 2, 2, 5, 1030 },  /* ja QTY.N_10 */
    { "1", 1, 2, 5, 1025 },  /* ja QTY.N_1 */
    { "2\346\231\202\351\226\223", 7, 2, 3, 778 },  /* ja TIME.HORA_2 */
    { "20", 2, 2, 5, 1031 },  /* ja QTY.N_20 */
    { "2", 1, 2, 5, 1026 },  /* ja QTY.N_2 */
    { "30\345\210\206", 5, 2, 3, 771 },  /* ja TIME.MIN_30 */
    { "3", 1, 2, 5, 1027 },  /* ja QTY.N_3 */
    { "4", 1, 2, 5, 1028 },  /* ja QTY.N_4 */
    { "5\345\210\206", 4, 2, 3, 769 },  /* ja TIME.MIN_5 */
    { "50", 2, 2, 5, 1032 },  /* ja QTY.N_50 */
    { "5", 1, 2, 5, 1029 },  /* ja QTY.N_5 */
    { "\343\201\212\345\276\205\343\201\241\343\201\217\343\201\240\343\201\225\343\201\204", 21, 2, 2, 25 },  /* ja EV.ESPERA */
    { "\343\201\223\343\201\243\343\201\241\343\201\253\346\235\245\343\201\246", 18, 2, 2, 24 },  /* ja EV.VEM */
    { "\343\201\202\343\202\212\343\201\214\343\201\250\343\201\206", 15, 2, 2, 15 },  /* ja EV.OBRIGADO */
    { "\343\201\212\343\202\201\343\201\247\343\201\250\343\201\206", 15, 2, 2, 19 },  /* ja EV.PARABENS */
    { "\343\201\223\343\202\223\343\201\253\343\201\241\343\201\257", 15, 2, 2, 20 },  /* ja EV.BOM_DIA */
    { "\343\201\225\343\202\210\343\201\206\343\201\252\343\202\211", 15, 2, 2, 21 },  /* ja EV.BOA_NOITE */
    { "\343\201\231\343\201\277\343\201\276\343\201\233\343\202\223", 15, 2, 2, 16 },  /* ja EV.DESCULPA */
    { "\343\202\217\343\201\213\343\202\211\343\201\252\343\201\204", 15, 2, 2, 56 },  /* ja EV.NAO_ENTENDI */
    { "\343\202\255\343\203\243\343\203\263\343\202\273\343\203\253", 15, 2, 2, 53 },  /* ja EV.CANCELADO */
    { "\343\203\221\343\202\271\343\203\257\343\203\274\343\203\211", 15, 6, 0, 0 },  /* ja sensitive */
    { "\343\202\260\343\203\253\343\203\274\343\203\2271", 13, 2, 1, 274 },  /* ja PER.GRUPO_1 */
    { "\343\202\260\343\203\253\343\203\274\343\203\2272", 13, 2, 1, 275 },  /* ja PER.GRUPO_2 */
    { "\343\201\212\343\201\257\343\202\210\343\201\206", 12, 2, 2, 20 },  /* ja EV.BOM_DIA */
    { "\343\201\212\343\202\204\343\201\231\343\201\277", 12, 2, 2, 21 },  /* ja EV.BOA_NOITE */
    { "\343\201\237\343\201\217\343\201\225\343\202\223", 12, 2, 5, 1035 },  /* ja QTY.MUITO */
    { "\343\201\247\343\201\215\343\201\252\343\201\204", 12, 2, 2, 14 },  /* ja EV.IMPOSSIVEL */
    { "\343\202\202\343\201\206\344\270\200\345\272\246", 12, 2, 2, 57 },  /* ja EV.REPETE */
    { "\343\202\217\343\201\213\343\201\243\343\201\237", 12, 8, 2, 56 },  /* ja EV.NAO_ENTENDI(neg) */
    { "\343\202\255\343\203\243\343\203\263\343\203\227", 12, 2, 4, 524 },  /* ja LOC.ACAMPAMENTO */
    { "\343\202\270\343\203\247\343\202\242\343\203\263", 12, 2, 1, 256 },  /* ja PER.JOAO */
    { "\343\201\224\343\202\201\343\202\223", 9, 2, 2, 16 },  /* ja EV.DESCULPA */
    { "\343\201\231\343\201\220\343\201\253", 9, 2, 3, 785 },  /* ja TIME.EM_BREVE */
    { "\343\202\253\343\203\274\343\203\211", 9, 6, 0, 0 },  /* ja sensitive */
    { "\343\202\254\343\202\244\343\203\211", 9, 2, 1, 267 },  /* ja PER.GUIA */
    { "\343\203\201\343\203\274\343\203\240", 9, 2, 1, 259 },  /* ja PER.EQUIPE */
    { "\343\203\236\343\203\252\343\202\242", 9, 2, 1, 257 },  /* ja PER.MARIA */
    { "\343\201\204\343\201\244", 6, 5, 3, 0 },  /* ja qword QUANDO */
    { "\343\201\213\343\202\211", 6, 0, 0, 0 },  /* ja stop */
    { "\343\201\221\343\201\214", 6, 2, 6, 1284 },  /* ja ST.FERIDO */
    { "\343\201\240\343\202\214", 6, 5, 1, 0 },  /* ja qword QUEM */
    { "\343\201\247\343\201\231", 6, 0, 0, 0 },  /* ja stop */
    { "\343\201\251\343\201\223", 6, 5, 4, 0 },  /* ja qword ONDE */
    { "\343\201\252\343\201\204", 6, 3, 0, 0 },  /* ja negation */
    { "\343\201\257\343\201\204", 6, 2, 2, 13 },  /* ja EV.SIM */
    { "\343\201\276\343\201\231", 6, 0, 0, 0 },  /* ja stop */
    { "\343\201\276\343\201\247", 6, 0, 0, 0 },  /* ja stop */
    { "\343\202\242\343\203\212", 6, 2, 1, 258 },  /* ja PER.ANA */
    { "\343\202\270\343\203\240", 6, 2, 4, 515 },  /* ja LOC.TREINO */
    { "\343\201\213", 3, 0, 0, 0 },  /* ja stop */
    { "\343\201\214", 3, 0, 0, 0 },  /* ja stop */
    { "\343\201\247", 3, 0, 0, 0 },  /* ja stop */
    { "\343\201\250", 3, 0, 0, 0 },  /* ja stop */
    { "\343\201\253", 3, 0, 0, 0 },  /* ja stop */
    { "\343\201\255", 3, 0, 0, 0 },  /* ja stop */
    { "\343\201\256", 3, 0, 0, 0 },  /* ja stop */
    { "\343\201\257", 3, 0, 0, 0 },  /* ja stop */
    { "\343\202\202", 3, 0, 0, 0 },  /* ja stop */
    { "\343\202\210", 3, 0, 0, 0 },  /* ja stop */
    { "\343\202\222", 3, 0, 0, 0 },  /* ja stop */
    { "\344\273\225\344\272\213\344\270\255\343\201\247\343\201\257\343\201\252\343\201\204", 21, 8, 2, 39 },  /* ja EV.TRABALHANDO(neg) */
    { "\344\270\255\346\255\242\343\201\247\343\201\257\343\201\252\343\201\204", 18, 8, 2, 53 },  /* ja EV.CANCELADO(neg) */
    { "\344\272\210\345\256\232\345\244\211\346\233\264\343\201\252\343\201\227", 18, 8, 2, 54 },  /* ja EV.PLANO_MUDOU(neg) */
    { "\344\272\213\346\225\205\343\201\247\343\201\257\343\201\252\343\201\204", 18, 8, 2, 26 },  /* ja EV.ACIDENTE(neg) */
    { "\344\275\225\343\201\213\343\201\212\343\201\213\343\201\227\343\201\204", 18, 2, 2, 59 },  /* ja EV.ALGO_ERRADO */
    { "\344\274\232\350\255\260\343\201\257\343\201\252\343\201\204", 15, 8, 2, 40 },  /* ja EV.REUNIAO(neg) */
    { "\344\275\223\350\252\277\343\201\214\346\202\252\343\201\204", 15, 2, 6, 1281 },  /* ja ST.MAL */
    { "\344\272\210\345\256\232\345\244\211\346\233\264", 12, 2, 2, 54 },  /* ja EV.PLANO_MUDOU */
    { "\344\274\232\343\201\204\343\201\237\343\201\204", 12, 2, 2, 18 },  /* ja EV.SAUDADE */
    { "\344\274\232\343\201\210\343\201\252\343\201\204", 12, 8, 2, 7 },  /* ja EV.ENCONTRAR(neg) */
    { "\344\273\225\344\272\213\344\270\255", 9, 2, 2, 39 },  /* ja EV.TRABALHANDO */
    { "\344\274\232\350\255\260\344\270\255", 9, 2, 2, 40 },  /* ja EV.REUNIAO */
    { "\344\274\235\343\201\210\343\201\246", 9, 1, 0, 1 },  /* ja op COMUNICAR */
    { "\344\270\200\344\272\272", 6, 2, 6, 1287 },  /* ja ST.SOZINHO_ST */
    { "\344\270\212\345\217\270", 6, 2, 1, 266 },  /* ja PER.CHEFE */
    { "\344\270\255\346\255\242", 6, 2, 2, 53 },  /* ja EV.CANCELADO */
    { "\344\272\206\350\247\243", 6, 2, 2, 13 },  /* ja EV.SIM */
    { "\344\272\210\345\256\232", 6, 1, 0, 4 },  /* ja op PLANEJAR */
    { "\344\272\213\346\225\205", 6, 2, 2, 26 },  /* ja EV.ACIDENTE */
    { "\344\272\272\346\225\260", 6, 2, 2, 60 },  /* ja EV.CONTAGEM */
    { "\344\273\212\345\244\234", 6, 2, 3, 775 },  /* ja TIME.NOITE */
    { "\344\273\212\346\227\245", 6, 2, 3, 773 },  /* ja TIME.HOJE */
    { "\344\273\212\351\200\261", 6, 2, 3, 783 },  /* ja TIME.SEMANA */
    { "\344\274\232\343\201\206", 6, 2, 2, 7 },  /* ja EV.ENCONTRAR */
    { "\344\274\232\350\255\260", 6, 2, 2, 40 },  /* ja EV.REUNIAO */
    { "\344\275\215\347\275\256", 6, 6, 0, 0 },  /* ja sensitive */
    { "\344\275\225\344\272\272", 6, 5, 5, 0 },  /* ja qword QUANTO */
    { "\344\273\212", 3, 2, 3, 768 },  /* ja TIME.AGORA */
    { "\345\217\227\343\201\221\345\217\226\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 24, 8, 2, 50 },  /* ja EV.RECEBI(neg) */
    { "\345\212\251\343\201\221\343\201\257\343\201\204\343\202\211\343\201\252\343\201\204", 21, 8, 2, 5 },  /* ja EV.AJUDA(neg) */
    { "\345\220\221\343\201\213\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 21, 8, 2, 8 },  /* ja EV.A_CAMINHO(neg) */
    { "\345\244\247\344\270\210\345\244\253\343\201\230\343\202\203\343\201\252\343\201\204", 21, 8, 2, 4 },  /* ja EV.TUDO_BEM(neg) */
    { "\345\247\213\343\201\276\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 21, 8, 2, 52 },  /* ja EV.COMECOU(neg) */
    { "\345\220\221\343\201\213\343\201\243\343\201\246\343\201\204\343\202\213", 18, 2, 2, 8 },  /* ja EV.A_CAMINHO */
    { "\345\276\205\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 18, 8, 2, 6 },  /* ja EV.ESPERANDO(neg) */
    { "\345\203\215\343\201\204\343\201\246\343\201\204\343\202\213", 15, 2, 2, 39 },  /* ja EV.TRABALHANDO */
    { "\345\205\205\351\233\273\343\201\214\343\201\252\343\201\204", 15, 2, 2, 10 },  /* ja EV.SEM_BATERIA */
    { "\345\207\272\347\231\272\343\201\227\343\201\252\343\201\204", 15, 8, 2, 2 },  /* ja EV.SAINDO(neg) */
    { "\345\213\211\345\274\267\343\201\227\343\201\252\343\201\204", 15, 8, 2, 12 },  /* ja EV.ESTUDAR(neg) */
    { "\345\214\273\350\200\205\343\201\257\344\270\215\350\246\201", 15, 8, 2, 29 },  /* ja EV.MEDICO(neg) */
    { "\345\217\227\343\201\221\345\217\226\343\201\243\343\201\237", 15, 2, 2, 50 },  /* ja EV.RECEBI */
    { "\345\220\214\346\204\217\343\201\227\343\201\252\343\201\204", 15, 8, 2, 13 },  /* ja EV.SIM(neg) */
    { "\345\257\235\343\201\246\343\201\204\343\201\252\343\201\204", 15, 8, 2, 37 },  /* ja EV.DORMINDO(neg) */
    { "\345\276\205\343\201\237\343\201\252\343\201\204\343\201\247", 15, 8, 2, 25 },  /* ja EV.ESPERA(neg) */
    { "\345\276\205\343\201\241\345\220\210\343\202\217\343\201\233", 15, 2, 2, 7 },  /* ja EV.ENCONTRAR */
    { "\345\276\205\343\201\243\343\201\246\343\201\204\343\202\213", 15, 2, 2, 6 },  /* ja EV.ESPERANDO */
    { "\345\217\226\343\202\212\346\266\210\343\201\227", 12, 1, 0, 7 },  /* ja op CANCELAR */
    { "\345\220\214\350\241\214\343\201\202\343\202\212", 12, 2, 6, 1288 },  /* ja ST.ACOMPANHADO */
    { "\345\225\217\351\241\214\343\201\252\343\201\204", 12, 2, 2, 4 },  /* ja EV.TUDO_BEM */
    { "\345\247\213\343\201\276\343\201\243\343\201\237", 12, 2, 2, 52 },  /* ja EV.COMECOU */
    { "\345\257\222\343\201\217\343\201\252\343\201\204", 12, 8, 2, 34 },  /* ja EV.FRIO(neg) */
    { "\345\212\251\343\201\221\343\201\246", 9, 2, 2, 5 },  /* ja EV.AJUDA */
    { "\345\213\235\346\211\213\343\201\253", 9, 7, 0, 0 },  /* ja authority */
    { "\345\215\261\343\201\252\343\201\204", 9, 2, 2, 22 },  /* ja EV.CUIDADO */
    { "\345\244\247\344\270\210\345\244\253", 9, 2, 2, 4 },  /* ja EV.TUDO_BEM */
    { "\345\244\247\345\245\275\343\201\215", 9, 2, 2, 17 },  /* ja EV.TE_AMO */
    { "\345\257\202\343\201\227\343\201\204", 9, 2, 2, 18 },  /* ja EV.SAUDADE */
    { "\345\276\205\343\201\243\343\201\246", 9, 2, 2, 25 },  /* ja EV.ESPERA */
    { "\345\201\234\346\255\242", 6, 2, 2, 23 },  /* ja EV.PARE */
    { "\345\205\203\346\260\227", 6, 2, 6, 1280 },  /* ja ST.BEM */
    { "\345\205\245\345\217\243", 6, 2, 4, 529 },  /* ja LOC.ENTRADA */
    { "\345\205\250\345\223\241", 6, 2, 1, 261 },  /* ja PER.TODOS */
    { "\345\205\250\351\203\250", 6, 2, 5, 1037 },  /* ja QTY.TOTAL */
    { "\345\207\272\343\202\213", 6, 2, 2, 2 },  /* ja EV.SAINDO */
    { "\345\207\272\347\231\272", 6, 2, 2, 2 },  /* ja EV.SAINDO */
    { "\345\210\260\347\235\200", 6, 2, 2, 1 },  /* ja EV.CHEGUEI */
    { "\345\213\211\345\274\267", 6, 2, 2, 12 },  /* ja EV.ESTUDAR */
    { "\345\214\227\351\226\200", 6, 2, 4, 514 },  /* ja LOC.PORTAO_N */
    { "\345\214\273\350\200\205", 6, 2, 2, 29 },  /* ja EV.MEDICO */
    { "\345\215\210\345\276\214", 6, 2, 3, 780 },  /* ja TIME.TARDE_DIA */
    { "\345\215\212\345\210\206", 6, 2, 5, 1036 },  /* ja QTY.METADE */
    { "\345\215\261\351\231\272", 6, 2, 6, 1283 },  /* ja ST.EM_PERIGO */
    { "\345\217\213\351\201\224", 6, 2, 1, 265 },  /* ja PER.AMIGO */
    { "\345\234\217\345\244\226", 6, 2, 2, 44 },  /* ja EV.SEM_SINAL */
    { "\345\237\272\345\234\260", 6, 2, 4, 527 },  /* ja LOC.BASE */
    { "\345\255\220\344\276\233", 6, 2, 1, 264 },  /* ja PER.FILHO */
    { "\345\255\246\346\240\241", 6, 2, 4, 518 },  /* ja LOC.ESCOLA */
    { "\345\255\246\347\277\222", 6, 2, 2, 12 },  /* ja EV.ESTUDAR */
    { "\345\256\211\345\205\250", 6, 2, 6, 1282 },  /* ja ST.SEGURO */
    { "\345\256\211\345\256\232", 6, 2, 6, 1286 },  /* ja ST.ESTAVEL */
    { "\345\256\214\344\272\206", 6, 2, 2, 9 },  /* ja EV.TERMINEI */
    { "\345\256\266\346\227\217", 6, 2, 1, 260 },  /* ja PER.FAMILIA */
    { "\345\257\222\343\201\204", 6, 2, 2, 34 },  /* ja EV.FRIO */
    { "\345\257\235\343\202\213", 6, 2, 2, 37 },  /* ja EV.DORMINDO */
    { "\345\260\221\343\201\227", 6, 2, 5, 1034 },  /* ja QTY.POUCO */
    { "\345\270\260\343\202\213", 6, 2, 2, 55 },  /* ja EV.VOLTANDO */
    { "\345\272\247\346\250\231", 6, 6, 0, 0 },  /* ja sensitive */
    { "\345\276\205\343\201\244", 6, 2, 2, 6 },  /* ja EV.ESPERANDO */
    { "\345\276\214\343\201\247", 6, 2, 3, 782 },  /* ja TIME.DEPOIS */
    { "\345\256\266", 3, 2, 4, 512 },  /* ja LOC.CASA */
    { "\345\267\235", 3, 2, 4, 525 },  /* ja LOC.RIO */
    { "\345\272\227", 3, 2, 4, 520 },  /* ja LOC.MERCADO */
    { "\346\272\226\345\202\231\343\201\247\343\201\215\343\201\246\343\201\204\343\201\252\343\201\204", 24, 8, 2, 51 },  /* ja EV.PRONTO(neg) */
    { "\346\224\257\346\211\225\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 21, 8, 2, 48 },  /* ja EV.PAGAMENTO(neg) */
    { "\346\225\205\351\232\234\343\201\227\343\201\246\343\201\204\343\201\252\343\201\204", 21, 8, 2, 43 },  /* ja EV.VEICULO_QUEBROU(neg) */
    { "\346\260\264\343\201\257\350\266\263\343\202\212\343\201\246\343\201\204\343\202\213", 21, 8, 2, 31 },  /* ja EV.AGUA(neg) */
    { "\346\210\273\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 18, 8, 2, 55 },  /* ja EV.VOLTANDO(neg) */
    { "\346\216\242\343\201\227\343\201\246\343\201\204\343\201\252\343\201\204", 18, 8, 2, 46 },  /* ja EV.PROCURANDO(neg) */
    { "\346\204\233\343\201\227\343\201\246\343\201\204\343\202\213", 15, 2, 2, 17 },  /* ja EV.TE_AMO */
    { "\346\216\242\343\201\227\343\201\246\343\201\204\343\202\213", 15, 2, 2, 46 },  /* ja EV.PROCURANDO */
    { "\346\255\242\343\201\276\343\202\211\343\201\252\343\201\204", 15, 8, 2, 23 },  /* ja EV.PARE(neg) */
    { "\346\260\264\343\201\214\343\201\273\343\201\227\343\201\204", 15, 2, 2, 31 },  /* ja EV.AGUA */
    { "\346\272\226\345\202\231\343\201\247\343\201\215\343\201\237", 15, 2, 2, 51 },  /* ja EV.PRONTO */
    { "\346\211\213\344\274\235\343\201\243\343\201\246", 12, 2, 2, 5 },  /* ja EV.AJUDA */
    { "\346\232\221\343\201\217\343\201\252\343\201\204", 12, 8, 2, 35 },  /* ja EV.CALOR(neg) */
    { "\346\235\245\343\201\252\343\201\204\343\201\247", 12, 8, 2, 24 },  /* ja EV.VEM(neg) */
    { "\346\255\242\343\201\276\343\201\243\343\201\246", 12, 2, 2, 23 },  /* ja EV.PARE */
    { "\346\224\257\346\211\225\343\201\204", 9, 2, 2, 48 },  /* ja EV.PAGAMENTO */
    { "\346\225\221\345\212\251\351\232\212", 9, 2, 1, 268 },  /* ja PER.SOCORRISTA */
    { "\346\225\221\346\200\245\350\273\212", 9, 2, 2, 29 },  /* ja EV.MEDICO */
    { "\346\200\226\343\201\204", 6, 2, 6, 1291 },  /* ja ST.COM_MEDO */
    { "\346\200\245\343\201\216", 6, 4, 0, 1 },  /* ja urgency ATENCAO */
    { "\346\204\237\350\254\235", 6, 2, 2, 15 },  /* ja EV.OBRIGADO */
    { "\346\210\273\343\202\213", 6, 2, 2, 55 },  /* ja EV.VOLTANDO */
    { "\346\225\205\351\232\234", 6, 2, 2, 43 },  /* ja EV.VEICULO_QUEBROU */
    { "\346\225\221\345\212\251", 6, 1, 0, 5 },  /* ja op SOCORRO */
    { "\346\230\216\346\227\245", 6, 2, 3, 774 },  /* ja TIME.AMANHA */
    { "\346\230\250\346\227\245", 6, 2, 3, 781 },  /* ja TIME.ONTEM */
    { "\346\232\221\343\201\204", 6, 2, 2, 35 },  /* ja EV.CALOR */
    { "\346\235\245\343\201\246", 6, 2, 2, 24 },  /* ja EV.VEM */
    { "\346\263\250\346\204\217", 6, 2, 2, 22 },  /* ja EV.CUIDADO */
    { "\346\234\235", 3, 2, 3, 779 },  /* ja TIME.MANHA */
    { "\346\257\215", 3, 2, 1, 262 },  /* ja PER.MAE */
    { "\346\260\264", 3, 2, 2, 31 },  /* ja EV.AGUA */
    { "\347\265\202\343\202\217\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 21, 8, 2, 9 },  /* ja EV.TERMINEI(neg) */
    { "\347\271\260\343\202\212\350\277\224\343\201\225\343\201\252\343\201\204\343\201\247", 21, 8, 2, 57 },  /* ja EV.REPETE(neg) */
    { "\347\201\253\344\272\213\343\201\247\343\201\257\343\201\252\343\201\204", 18, 8, 2, 27 },  /* ja EV.FOGO(neg) */
    { "\347\204\241\347\220\206\343\201\247\343\201\257\343\201\252\343\201\204", 18, 8, 2, 14 },  /* ja EV.IMPOSSIVEL(neg) */
    { "\347\226\262\343\202\214\343\201\246\343\201\204\343\201\252\343\201\204", 18, 8, 2, 36 },  /* ja EV.CANSADO(neg) */
    { "\347\235\200\343\201\204\343\201\246\343\201\204\343\201\252\343\201\204", 18, 8, 2, 1 },  /* ja EV.CHEGUEI(neg) */
    { "\347\242\272\350\252\215\343\201\252\343\201\227\343\201\247", 15, 7, 0, 0 },  /* ja authority */
    { "\347\204\241\350\246\226\343\201\227\343\201\246", 12, 7, 0, 0 },  /* ja authority */
    { "\347\225\260\345\270\270\343\201\252\343\201\227", 12, 8, 2, 59 },  /* ja EV.ALGO_ERRADO(neg) */
    { "\347\227\233\343\201\217\343\201\252\343\201\204", 12, 8, 2, 30 },  /* ja EV.DOR(neg) */
    { "\347\237\245\343\202\211\343\201\233\343\201\246", 12, 1, 0, 1 },  /* ja op COMUNICAR */
    { "\347\265\202\343\202\217\343\201\243\343\201\237", 12, 2, 2, 9 },  /* ja EV.TERMINEI */
    { "\347\226\262\343\202\214\343\201\237", 9, 2, 2, 36 },  /* ja EV.CANSADO */
    { "\347\231\273\345\261\261\351\201\223", 9, 2, 4, 517 },  /* ja LOC.TRILHA */
    { "\347\235\200\343\201\204\343\201\237", 9, 2, 2, 1 },  /* ja EV.CHEGUEI */
    { "\347\235\241\347\234\240\344\270\255", 9, 2, 2, 37 },  /* ja EV.DORMINDO */
    { "\347\247\273\345\213\225\344\270\255", 9, 2, 2, 8 },  /* ja EV.A_CAMINHO */
    { "\347\267\212\346\200\245\343\201\253", 9, 4, 0, 2 },  /* ja urgency URGENTE */
    { "\347\201\253\344\272\213", 6, 2, 2, 27 },  /* ja EV.FOGO */
    { "\347\204\241\347\220\206", 6, 2, 2, 14 },  /* ja EV.IMPOSSIVEL */
    { "\347\224\237\344\275\223", 6, 6, 0, 0 },  /* ja sensitive */
    { "\347\227\205\351\231\242", 6, 2, 4, 519 },  /* ja LOC.HOSPITAL */
    { "\347\227\233\343\201\204", 6, 2, 2, 30 },  /* ja EV.DOR */
    { "\347\227\233\343\201\277", 6, 2, 2, 30 },  /* ja EV.DOR */
    { "\347\242\272\350\252\215", 6, 1, 0, 6 },  /* ja op CONFIRMAR */
    { "\347\251\272\346\270\257", 6, 2, 4, 521 },  /* ja LOC.AEROPORTO */
    { "\347\251\272\350\205\271", 6, 2, 2, 32 },  /* ja EV.COMIDA */
    { "\347\267\212\346\200\245", 6, 1, 0, 5 },  /* ja op SOCORRO */
    { "\347\205\231", 3, 2, 2, 27 },  /* ja EV.FOGO */
    { "\347\210\266", 3, 2, 1, 263 },  /* ja PER.PAI */
    { "\347\247\201", 3, 0, 0, 0 },  /* ja stop */
    { "\350\246\213\343\201\244\343\201\213\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 24, 8, 2, 45 },  /* ja EV.ACHEI(neg) */
    { "\350\220\275\343\201\241\347\235\200\343\201\204\343\201\246\343\201\204\343\202\213", 21, 2, 6, 1290 },  /* ja ST.CALMO */
    { "\350\251\261\343\201\231\345\277\205\350\246\201\343\201\257\343\201\252\343\201\204", 21, 8, 2, 58 },  /* ja EV.PRECISO_FALAR(neg) */
    { "\350\262\267\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 18, 8, 2, 47 },  /* ja EV.COMPREI(neg) */
    { "\350\265\267\343\201\215\343\201\246\343\201\204\343\201\252\343\201\204", 18, 8, 2, 38 },  /* ja EV.ACORDEI(neg) */
    { "\350\277\267\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 18, 8, 2, 11 },  /* ja EV.PERDIDO(neg) */
    { "\350\255\246\345\257\237\343\201\257\344\270\215\350\246\201", 15, 8, 2, 28 },  /* ja EV.POLICIA(neg) */
    { "\350\273\212\343\201\214\345\243\212\343\202\214\343\201\237", 15, 2, 2, 43 },  /* ja EV.VEICULO_QUEBROU */
    { "\350\207\252\345\213\225\347\232\204\343\201\253", 12, 7, 0, 0 },  /* ja authority */
    { "\350\246\213\343\201\244\343\201\221\343\201\237", 12, 2, 2, 45 },  /* ja EV.ACHEI */
    { "\350\250\230\351\214\262\343\201\227\343\201\246", 12, 1, 0, 3 },  /* ja op LEMBRAR */
    { "\350\251\261\343\201\227\343\201\237\343\201\204", 12, 2, 2, 58 },  /* ja EV.PRECISO_FALAR */
    { "\350\251\261\343\201\233\343\201\252\343\201\204", 12, 2, 2, 42 },  /* ja EV.NAO_POSSO_FALAR */
    { "\350\201\236\343\201\204\343\201\246", 9, 1, 0, 2 },  /* ja op PERGUNTAR */
    { "\350\246\232\343\201\210\343\201\246", 9, 1, 0, 3 },  /* ja op LEMBRAR */
    { "\350\251\261\343\201\233\343\202\213", 9, 8, 2, 42 },  /* ja EV.NAO_POSSO_FALAR(neg) */
    { "\350\262\267\343\201\243\343\201\237", 9, 2, 2, 47 },  /* ja EV.COMPREI */
    { "\350\265\267\343\201\215\343\201\237", 9, 2, 2, 38 },  /* ja EV.ACORDEI */
    { "\350\201\267\345\240\264", 6, 2, 4, 513 },  /* ja LOC.TRABALHO */
    { "\350\255\246\345\257\237", 6, 2, 2, 28 },  /* ja EV.POLICIA */
    { "\350\277\221\346\211\200", 6, 2, 1, 269 },  /* ja PER.VIZINHO */
    { "\350\277\267\345\255\220", 6, 2, 2, 11 },  /* ja EV.PERDIDO */
    { "\350\252\260", 3, 5, 1, 0 },  /* ja qword QUEM */
    { "\350\273\212", 3, 2, 4, 523 },  /* ja LOC.VEICULO */
    { "\351\243\237\343\201\271\347\211\251\343\201\257\350\266\263\343\202\212\343\201\246\343\201\204\343\202\213", 27, 8, 2, 32 },  /* ja EV.COMIDA(neg) */
    { "\351\226\211\343\201\230\350\276\274\343\202\201\343\202\211\343\202\214\343\201\237", 21, 2, 6, 1289 },  /* ja ST.PRESO */
    { "\351\200\201\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", 18, 8, 2, 49 },  /* ja EV.ENVIEI(neg) */
    { "\351\201\277\351\233\243\346\211\200\343\201\257\344\270\215\350\246\201", 18, 8, 2, 33 },  /* ja EV.ABRIGO(neg) */
    { "\351\233\273\350\251\261\343\201\227\343\201\252\343\201\204\343\201\247", 18, 8, 2, 41 },  /* ja EV.LIGA_PRA_MIM(neg) */
    { "\351\201\223\343\201\253\350\277\267\343\201\243\343\201\237", 15, 2, 2, 11 },  /* ja EV.PERDIDO */
    { "\351\233\273\346\261\240\343\201\257\343\201\202\343\202\213", 15, 8, 2, 10 },  /* ja EV.SEM_BATERIA(neg) */
    { "\351\233\273\346\263\242\343\201\214\343\201\252\343\201\204", 15, 2, 2, 44 },  /* ja EV.SEM_SINAL */
    { "\351\233\273\346\263\242\343\201\257\343\201\202\343\202\213", 15, 8, 2, 44 },  /* ja EV.SEM_SINAL(neg) */
    { "\351\201\205\343\202\214\343\201\252\343\201\204", 12, 8, 2, 3 },  /* ja EV.ATRASADO(neg) */
    { "\351\233\206\345\220\210\345\240\264\346\211\200", 12, 2, 4, 528 },  /* ja LOC.PONTO_ENCONTRO */
    { "\351\233\273\346\261\240\345\210\207\343\202\214", 12, 2, 2, 10 },  /* ja EV.SEM_BATERIA */
    { "\351\233\273\350\251\261\343\201\227\343\201\246", 12, 2, 2, 41 },  /* ja EV.LIGA_PRA_MIM */
    { "\351\200\243\347\265\241\345\205\2101", 10, 2, 1, 270 },  /* ja PER.HANDLE_1 */
    { "\351\200\243\347\265\241\345\205\2102", 10, 2, 1, 271 },  /* ja PER.HANDLE_2 */
    { "\351\200\243\347\265\241\345\205\2103", 10, 2, 1, 272 },  /* ja PER.HANDLE_3 */
    { "\351\200\243\347\265\241\345\205\2104", 10, 2, 1, 273 },  /* ja PER.HANDLE_4 */
    { "\351\200\201\343\201\243\343\201\237", 9, 2, 2, 49 },  /* ja EV.ENVIEI */
    { "\351\201\205\343\202\214\343\202\213", 9, 2, 2, 3 },  /* ja EV.ATRASADO */
    { "\351\201\277\351\233\243\346\211\200", 9, 2, 2, 33 },  /* ja EV.ABRIGO */
    { "\351\243\237\343\201\271\347\211\251", 9, 2, 2, 32 },  /* ja EV.COMIDA */
    { "\351\247\220\350\273\212\345\240\264", 9, 2, 4, 530 },  /* ja LOC.ESTACIONAMENTO */
    { "\351\200\261\346\234\253", 6, 2, 3, 784 },  /* ja TIME.FIM_DE_SEMANA */
    { "\351\201\205\345\210\273", 6, 2, 2, 3 },  /* ja EV.ATRASADO */
    { "\351\201\223\350\267\257", 6, 2, 4, 516 },  /* ja LOC.ESTRADA */
    { "\351\207\215\347\227\207", 6, 2, 6, 1285 },  /* ja ST.GRAVE */
    { "\351\214\262\351\237\263", 6, 6, 0, 0 },  /* ja sensitive */
    { "\351\237\263\345\243\260", 6, 6, 0, 0 },  /* ja sensitive */
    { "\351\240\202\344\270\212", 6, 2, 4, 526 },  /* ja LOC.CUME */
    { "\351\215\265", 3, 6, 0, 0 },  /* ja sensitive */
    { "\351\247\205", 3, 2, 4, 522 },  /* ja LOC.ESTACAO */
    { "0", 1, 2, 5, 1024 },  /* zh QTY.N_0 */
    { "10\345\210\206\351\222\237", 8, 2, 3, 770 },  /* zh TIME.MIN_10 */
    { "15\345\210\206\351\222\237", 8, 2, 3, 777 },  /* zh TIME.MIN_15 */
    { "1\345\210\206\351\222\237", 7, 2, 3, 776 },  /* zh TIME.MIN_1 */
    { "1\345\260\217\346\227\266", 7, 2, 3, 772 },  /* zh TIME.HORA_1 */
    { "100", 3, 2, 5, 1033 },  /* zh QTY.N_100 */
    { "10", 2, 2, 5, 1030 },  /* zh QTY.N_10 */
    { "1", 1, 2, 5, 1025 },  /* zh QTY.N_1 */
    { "2\345\260\217\346\227\266", 7, 2, 3, 778 },  /* zh TIME.HORA_2 */
    { "20", 2, 2, 5, 1031 },  /* zh QTY.N_20 */
    { "2", 1, 2, 5, 1026 },  /* zh QTY.N_2 */
    { "30\345\210\206\351\222\237", 8, 2, 3, 771 },  /* zh TIME.MIN_30 */
    { "3", 1, 2, 5, 1027 },  /* zh QTY.N_3 */
    { "4", 1, 2, 5, 1028 },  /* zh QTY.N_4 */
    { "5\345\210\206\351\222\237", 7, 2, 3, 769 },  /* zh TIME.MIN_5 */
    { "50", 2, 2, 5, 1032 },  /* zh QTY.N_50 */
    { "5", 1, 2, 5, 1029 },  /* zh QTY.N_5 */
    { "\344\270\215\351\234\200\350\246\201\351\201\277\351\232\276\346\211\200", 18, 8, 2, 33 },  /* zh EV.ABRIGO(neg) */
    { "\344\270\215\347\224\250\346\211\223\347\224\265\350\257\235", 15, 8, 2, 41 },  /* zh EV.LIGA_PRA_MIM(neg) */
    { "\344\270\215\351\234\200\350\246\201\345\270\256\345\212\251", 15, 8, 2, 5 },  /* zh EV.AJUDA(neg) */
    { "\344\270\215\351\234\200\350\246\201\351\243\237\347\211\251", 15, 8, 2, 32 },  /* zh EV.COMIDA(neg) */
    { "\344\270\200\345\210\207\346\255\243\345\270\270", 12, 8, 2, 59 },  /* zh EV.ALGO_ERRADO(neg) */
    { "\344\270\215\346\230\257\344\272\213\346\225\205", 12, 8, 2, 26 },  /* zh EV.ACIDENTE(neg) */
    { "\344\270\215\347\224\250\345\214\273\347\224\237", 12, 8, 2, 29 },  /* zh EV.MEDICO(neg) */
    { "\344\270\215\347\224\250\347\241\256\350\256\244", 12, 7, 0, 0 },  /* zh authority */
    { "\344\270\215\347\224\250\350\255\246\345\257\237", 12, 8, 2, 28 },  /* zh EV.POLICIA(neg) */
    { "\344\270\215\347\224\250\351\207\215\345\244\215", 12, 8, 2, 57 },  /* zh EV.REPETE(neg) */
    { "\344\270\215\350\203\275\350\257\264\350\257\235", 12, 2, 2, 42 },  /* zh EV.NAO_POSSO_FALAR */
    { "\344\270\215\350\246\201\350\277\207\346\235\245", 12, 8, 2, 24 },  /* zh EV.VEM(neg) */
    { "\344\270\215\351\234\200\350\246\201\346\260\264", 12, 8, 2, 31 },  /* zh EV.AGUA(neg) */
    { "\344\273\200\344\271\210\346\227\266\345\200\231", 12, 5, 3, 0 },  /* zh qword QUANDO */
    { "\344\270\215\345\220\214\346\204\217", 9, 8, 2, 13 },  /* zh EV.SIM(neg) */
    { "\344\270\215\345\244\252\345\245\275", 9, 8, 2, 4 },  /* zh EV.TUDO_BEM(neg) */
    { "\344\270\215\345\255\246\344\271\240", 9, 8, 2, 12 },  /* zh EV.ESTUDAR(neg) */
    { "\344\270\215\345\257\271\345\212\262", 9, 2, 2, 59 },  /* zh EV.ALGO_ERRADO */
    { "\344\270\215\346\230\216\347\231\275", 9, 2, 2, 56 },  /* zh EV.NAO_ENTENDI */
    { "\344\270\215\347\224\250\347\255\211", 9, 8, 2, 25 },  /* zh EV.ESPERA(neg) */
    { "\344\270\215\347\224\250\350\260\210", 9, 8, 2, 58 },  /* zh EV.PRECISO_FALAR(neg) */
    { "\344\270\215\350\246\201\345\201\234", 9, 8, 2, 23 },  /* zh EV.PARE(neg) */
    { "\344\270\215\350\247\201\351\235\242", 9, 8, 2, 7 },  /* zh EV.ENCONTRAR(neg) */
    { "\344\270\215\350\277\237\345\210\260", 9, 8, 2, 3 },  /* zh EV.ATRASADO(neg) */
    { "\344\270\200\345\215\212", 6, 2, 5, 1036 },  /* zh QTY.METADE */
    { "\344\270\212\347\217\255", 6, 2, 2, 39 },  /* zh EV.TRABALHANDO */
    { "\344\270\213\345\215\210", 6, 2, 3, 780 },  /* zh TIME.TARDE_DIA */
    { "\344\270\215\345\206\267", 6, 8, 2, 34 },  /* zh EV.FRIO(neg) */
    { "\344\270\215\345\245\275", 6, 2, 6, 1281 },  /* zh ST.MAL */
    { "\344\270\215\347\203\255", 6, 8, 2, 35 },  /* zh EV.CALOR(neg) */
    { "\344\270\215\347\226\274", 6, 8, 2, 30 },  /* zh EV.DOR(neg) */
    { "\344\270\215\347\264\257", 6, 8, 2, 36 },  /* zh EV.CANSADO(neg) */
    { "\344\270\215\350\241\214", 6, 2, 2, 14 },  /* zh EV.IMPOSSIVEL */
    { "\344\270\245\351\207\215", 6, 2, 6, 1285 },  /* zh ST.GRAVE */
    { "\344\271\260\344\272\206", 6, 2, 2, 47 },  /* zh EV.COMPREI */
    { "\344\272\213\346\225\205", 6, 2, 2, 26 },  /* zh EV.ACIDENTE */
    { "\344\272\272\346\225\260", 6, 2, 2, 60 },  /* zh EV.CONTAGEM */
    { "\344\273\212\345\244\251", 6, 2, 3, 773 },  /* zh TIME.HOJE */
    { "\344\273\212\346\231\232", 6, 2, 3, 775 },  /* zh TIME.NOITE */
    { "\344\273\230\346\254\276", 6, 2, 2, 48 },  /* zh EV.PAGAMENTO */
    { "\344\273\245\345\220\216", 6, 2, 3, 782 },  /* zh TIME.DEPOIS */
    { "\344\274\232\350\256\256", 6, 2, 2, 40 },  /* zh EV.REUNIAO */
    { "\344\275\215\347\275\256", 6, 6, 0, 0 },  /* zh sensitive */
    { "\344\275\240\345\245\275", 6, 2, 2, 20 },  /* zh EV.BOM_DIA */
    { "\344\272\206", 3, 0, 0, 0 },  /* zh stop */
    { "\344\273\226", 3, 0, 0, 0 },  /* zh stop */
    { "\344\275\240", 3, 0, 0, 0 },  /* zh stop */
    { "\345\206\215\350\257\264\344\270\200\346\254\241", 12, 2, 2, 57 },  /* zh EV.REPETE */
    { "\345\207\206\345\244\207\345\245\275\344\272\206", 12, 2, 2, 51 },  /* zh EV.PRONTO */
    { "\345\217\257\344\273\245\350\257\264\350\257\235", 12, 8, 2, 42 },  /* zh EV.NAO_POSSO_FALAR(neg) */
    { "\345\201\232\344\270\215\345\210\260", 9, 2, 2, 14 },  /* zh EV.IMPOSSIVEL */
    { "\345\201\232\345\256\214\344\272\206", 9, 2, 2, 9 },  /* zh EV.TERMINEI */
    { "\345\201\234\350\275\246\345\234\272", 9, 2, 4, 530 },  /* zh LOC.ESTACIONAMENTO */
    { "\345\201\245\350\272\253\346\210\277", 9, 2, 4, 515 },  /* zh LOC.TREINO */
    { "\345\217\226\346\266\210\344\272\206", 9, 2, 2, 53 },  /* zh EV.CANCELADO */
    { "\345\234\250\345\267\245\344\275\234", 9, 2, 2, 39 },  /* zh EV.TRABALHANDO */
    { "\345\234\250\345\274\200\344\274\232", 9, 2, 2, 40 },  /* zh EV.REUNIAO */
    { "\345\234\250\350\267\257\344\270\212", 9, 2, 2, 8 },  /* zh EV.A_CAMINHO */
    { "\345\234\250\350\277\224\345\233\236", 9, 2, 2, 55 },  /* zh EV.VOLTANDO */
    { "\345\257\271\344\270\215\350\265\267", 9, 2, 2, 16 },  /* zh EV.DESCULPA */
    { "\345\267\262\345\257\204\345\207\272", 9, 2, 2, 49 },  /* zh EV.ENVIEI */
    { "\345\274\200\345\247\213\344\272\206", 9, 2, 2, 52 },  /* zh EV.COMECOU */
    { "\345\260\217\347\273\2041", 7, 2, 1, 274 },  /* zh PER.GRUPO_1 */
    { "\345\260\217\347\273\2042", 7, 2, 1, 275 },  /* zh PER.GRUPO_2 */
    { "\345\201\234\344\270\213", 6, 2, 2, 23 },  /* zh EV.PARE */
    { "\345\205\245\345\217\243", 6, 2, 4, 529 },  /* zh LOC.ENTRADA */
    { "\345\205\250\351\203\250", 6, 2, 5, 1037 },  /* zh QTY.TOTAL */
    { "\345\205\254\345\217\270", 6, 2, 4, 513 },  /* zh LOC.TRABALHO */
    { "\345\206\215\350\247\201", 6, 2, 2, 21 },  /* zh EV.BOA_NOITE */
    { "\345\207\272\345\217\221", 6, 2, 2, 2 },  /* zh EV.SAINDO */
    { "\345\210\260\344\272\206", 6, 2, 2, 1 },  /* zh EV.CHEGUEI */
    { "\345\214\227\351\227\250", 6, 2, 4, 514 },  /* zh LOC.PORTAO_N */
    { "\345\214\273\347\224\237", 6, 2, 2, 29 },  /* zh EV.MEDICO */
    { "\345\214\273\351\231\242", 6, 2, 4, 519 },  /* zh LOC.HOSPITAL */
    { "\345\215\241\345\217\267", 6, 6, 0, 0 },  /* zh sensitive */
    { "\345\215\261\351\231\251", 6, 2, 6, 1283 },  /* zh ST.EM_PERIGO */
    { "\345\217\221\344\272\206", 6, 2, 2, 49 },  /* zh EV.ENVIEI */
    { "\345\217\226\346\266\210", 6, 1, 0, 7 },  /* zh op CANCELAR */
    { "\345\217\227\344\274\244", 6, 2, 6, 1284 },  /* zh ST.FERIDO */
    { "\345\217\257\344\273\245", 6, 8, 2, 14 },  /* zh EV.IMPOSSIVEL(neg) */
    { "\345\220\214\346\204\217", 6, 2, 2, 13 },  /* zh EV.SIM */
    { "\345\221\212\350\257\211", 6, 1, 0, 1 },  /* zh op COMUNICAR */
    { "\345\221\250\346\234\253", 6, 2, 3, 784 },  /* zh TIME.FIM_DE_SEMANA */
    { "\345\223\252\351\207\214", 6, 5, 4, 0 },  /* zh qword ONDE */
    { "\345\225\206\345\272\227", 6, 2, 4, 520 },  /* zh LOC.MERCADO */
    { "\345\233\236\345\216\273", 6, 2, 2, 55 },  /* zh EV.VOLTANDO */
    { "\345\233\242\351\230\237", 6, 2, 1, 259 },  /* zh PER.EQUIPE */
    { "\345\234\250\346\211\276", 6, 2, 2, 46 },  /* zh EV.PROCURANDO */
    { "\345\234\250\347\235\241", 6, 2, 2, 37 },  /* zh EV.DORMINDO */
    { "\345\234\250\347\255\211", 6, 2, 2, 6 },  /* zh EV.ESPERANDO */
    { "\345\235\220\346\240\207", 6, 6, 0, 0 },  /* zh sensitive */
    { "\345\237\272\345\234\260", 6, 2, 4, 527 },  /* zh LOC.BASE */
    { "\345\244\232\345\260\221", 6, 5, 5, 0 },  /* zh qword QUANTO */
    { "\345\246\210\345\246\210", 6, 2, 1, 262 },  /* zh PER.MAE */
    { "\345\255\246\344\271\240", 6, 2, 2, 12 },  /* zh EV.ESTUDAR */
    { "\345\255\246\346\240\241", 6, 2, 4, 518 },  /* zh LOC.ESCOLA */
    { "\345\255\251\345\255\220", 6, 2, 1, 264 },  /* zh PER.FILHO */
    { "\345\256\211\345\205\250", 6, 2, 6, 1282 },  /* zh ST.SEGURO */
    { "\345\256\211\345\250\234", 6, 2, 1, 258 },  /* zh PER.ANA */
    { "\345\256\214\346\210\220", 6, 2, 2, 9 },  /* zh EV.TERMINEI */
    { "\345\256\263\346\200\225", 6, 2, 6, 1291 },  /* zh ST.COM_MEDO */
    { "\345\256\266\344\272\272", 6, 2, 1, 260 },  /* zh PER.FAMILIA */
    { "\345\256\266\351\207\214", 6, 2, 4, 512 },  /* zh LOC.CASA */
    { "\345\257\206\347\240\201", 6, 6, 0, 0 },  /* zh sensitive */
    { "\345\257\206\351\222\245", 6, 6, 0, 0 },  /* zh sensitive */
    { "\345\260\217\345\276\204", 6, 2, 4, 517 },  /* zh LOC.TRILHA */
    { "\345\260\217\345\277\203", 6, 2, 2, 22 },  /* zh EV.CUIDADO */
    { "\345\261\261\351\241\266", 6, 2, 4, 526 },  /* zh LOC.CUME */
    { "\345\267\262\345\210\260", 6, 2, 2, 1 },  /* zh EV.CHEGUEI */
    { "\345\270\256\345\277\231", 6, 2, 2, 5 },  /* zh EV.AJUDA */
    { "\345\271\263\351\235\231", 6, 2, 6, 1290 },  /* zh ST.CALMO */
    { "\345\275\225\351\237\263", 6, 6, 0, 0 },  /* zh sensitive */
    { "\345\276\210\345\244\232", 6, 2, 5, 1035 },  /* zh QTY.MUITO */
    { "\345\277\275\347\225\245", 6, 7, 0, 0 },  /* zh authority */
    { "\345\201\234", 3, 2, 2, 23 },  /* zh EV.PARE */
    { "\345\206\267", 3, 2, 2, 34 },  /* zh EV.FRIO */
    { "\345\220\227", 3, 0, 0, 0 },  /* zh stop */
    { "\345\222\214", 3, 0, 0, 0 },  /* zh stop */
    { "\345\245\271", 3, 0, 0, 0 },  /* zh stop */
    { "\345\257\271", 3, 0, 0, 0 },  /* zh stop */
    { "\345\260\221", 3, 2, 5, 1034 },  /* zh QTY.POUCO */
    { "\345\277\253", 3, 4, 0, 1 },  /* zh urgency ATENCAO */
    { "\346\211\223\347\224\265\350\257\235\347\273\231\346\210\221", 15, 2, 2, 41 },  /* zh EV.LIGA_PRA_MIM */
    { "\346\227\240\351\234\200\347\241\256\350\256\244", 12, 7, 0, 0 },  /* zh authority */
    { "\346\262\241\345\207\206\345\244\207\345\245\275", 12, 8, 2, 51 },  /* zh EV.PRONTO(neg) */
    { "\346\262\241\345\234\250\345\233\236\345\216\273", 12, 8, 2, 55 },  /* zh EV.VOLTANDO(neg) */
    { "\346\262\241\345\234\250\345\267\245\344\275\234", 12, 8, 2, 39 },  /* zh EV.TRABALHANDO(neg) */
    { "\346\262\241\345\234\250\350\267\257\344\270\212", 12, 8, 2, 8 },  /* zh EV.A_CAMINHO(neg) */
    { "\346\262\241\346\234\211\344\274\232\350\256\256", 12, 8, 2, 40 },  /* zh EV.REUNIAO(neg) */
    { "\346\210\221\347\210\261\344\275\240", 9, 2, 2, 17 },  /* zh EV.TE_AMO */
    { "\346\211\200\346\234\211\344\272\272", 9, 2, 1, 261 },  /* zh PER.TODOS */
    { "\346\211\276\345\210\260\344\272\206", 9, 2, 2, 45 },  /* zh EV.ACHEI */
    { "\346\220\234\347\264\242\344\270\255", 9, 2, 2, 46 },  /* zh EV.PROCURANDO */
    { "\346\224\266\345\210\260\344\272\206", 9, 2, 2, 50 },  /* zh EV.RECEBI */
    { "\346\225\221\346\212\244\350\275\246", 9, 2, 2, 29 },  /* zh EV.MEDICO */
    { "\346\225\221\346\217\264\351\230\237", 9, 2, 1, 268 },  /* zh PER.SOCORRISTA */
    { "\346\227\240\347\275\221\347\273\234", 9, 2, 2, 44 },  /* zh EV.SEM_SINAL */
    { "\346\227\251\344\270\212\345\245\275", 9, 2, 2, 20 },  /* zh EV.BOM_DIA */
    { "\346\230\216\347\231\275\344\272\206", 9, 8, 2, 56 },  /* zh EV.NAO_ENTENDI(neg) */
    { "\346\234\211\344\272\272\351\231\252", 9, 2, 6, 1288 },  /* zh ST.ACOMPANHADO */
    { "\346\234\211\344\277\241\345\217\267", 9, 8, 2, 44 },  /* zh EV.SEM_SINAL(neg) */
    { "\346\234\211\351\227\256\351\242\230", 9, 2, 2, 59 },  /* zh EV.ALGO_ERRADO */
    { "\346\235\245\350\277\231\351\207\214", 9, 2, 2, 24 },  /* zh EV.VEM */
    { "\346\262\241\344\273\230\346\254\276", 9, 8, 2, 48 },  /* zh EV.PAGAMENTO(neg) */
    { "\346\262\241\344\277\241\345\217\267", 9, 2, 2, 44 },  /* zh EV.SEM_SINAL */
    { "\346\262\241\345\217\226\346\266\210", 9, 8, 2, 53 },  /* zh EV.CANCELADO(neg) */
    { "\346\262\241\345\234\250\346\211\276", 9, 8, 2, 46 },  /* zh EV.PROCURANDO(neg) */
    { "\346\262\241\345\234\250\347\235\241", 9, 8, 2, 37 },  /* zh EV.DORMINDO(neg) */
    { "\346\262\241\345\234\250\347\255\211", 9, 8, 2, 6 },  /* zh EV.ESPERANDO(neg) */
    { "\346\262\241\345\256\214\346\210\220", 9, 8, 2, 9 },  /* zh EV.TERMINEI(neg) */
    { "\346\262\241\345\274\200\345\247\213", 9, 8, 2, 52 },  /* zh EV.COMECOU(neg) */
    { "\346\262\241\346\211\276\345\210\260", 9, 8, 2, 45 },  /* zh EV.ACHEI(neg) */
    { "\346\262\241\346\224\266\345\210\260", 9, 8, 2, 50 },  /* zh EV.RECEBI(neg) */
    { "\346\262\241\346\234\211\347\201\253", 9, 8, 2, 27 },  /* zh EV.FOGO(neg) */
    { "\346\262\241\347\224\265\344\272\206", 9, 2, 2, 10 },  /* zh EV.SEM_BATERIA */
    { "\346\262\241\350\277\267\350\267\257", 9, 8, 2, 11 },  /* zh EV.PERDIDO(neg) */
    { "\346\262\241\351\227\256\351\242\230", 9, 2, 2, 4 },  /* zh EV.TUDO_BEM */
    { "\346\200\235\345\277\265", 6, 2, 2, 18 },  /* zh EV.SAUDADE */
    { "\346\201\255\345\226\234", 6, 2, 2, 19 },  /* zh EV.PARABENS */
    { "\346\203\263\344\275\240", 6, 2, 2, 18 },  /* zh EV.SAUDADE */
    { "\346\204\237\350\260\242", 6, 2, 2, 15 },  /* zh EV.OBRIGADO */
    { "\346\212\261\346\255\211", 6, 2, 2, 16 },  /* zh EV.DESCULPA */
    { "\346\222\236\350\275\246", 6, 2, 2, 26 },  /* zh EV.ACIDENTE */
    { "\346\223\205\350\207\252", 6, 7, 0, 0 },  /* zh authority */
    { "\346\224\257\344\273\230", 6, 2, 2, 48 },  /* zh EV.PAGAMENTO */
    { "\346\225\205\351\232\234", 6, 2, 2, 43 },  /* zh EV.VEICULO_QUEBROU */
    { "\346\227\251\344\270\212", 6, 2, 3, 779 },  /* zh TIME.MANHA */
    { "\346\230\216\345\244\251", 6, 2, 3, 774 },  /* zh TIME.AMANHA */
    { "\346\230\250\345\244\251", 6, 2, 3, 781 },  /* zh TIME.ONTEM */
    { "\346\231\232\345\256\211", 6, 2, 2, 21 },  /* zh EV.BOA_NOITE */
    { "\346\234\213\345\217\213", 6, 2, 1, 265 },  /* zh PER.AMIGO */
    { "\346\234\272\345\234\272", 6, 2, 4, 521 },  /* zh LOC.AEROPORTO */
    { "\346\261\202\346\225\221", 6, 1, 0, 5 },  /* zh op SOCORRO */
    { "\346\262\241\344\271\260", 6, 8, 2, 47 },  /* zh EV.COMPREI(neg) */
    { "\346\262\241\345\210\260", 6, 8, 2, 1 },  /* zh EV.CHEGUEI(neg) */
    { "\346\262\241\345\217\221", 6, 8, 2, 49 },  /* zh EV.ENVIEI(neg) */
    { "\346\262\241\351\206\222", 6, 8, 2, 38 },  /* zh EV.ACORDEI(neg) */
    { "\346\263\250\346\204\217", 6, 2, 2, 22 },  /* zh EV.CUIDADO */
    { "\346\270\205\347\202\271", 6, 2, 2, 60 },  /* zh EV.CONTAGEM */
    { "\346\210\221", 3, 0, 0, 0 },  /* zh stop */
    { "\346\212\212", 3, 0, 0, 0 },  /* zh stop */
    { "\346\230\257", 3, 0, 0, 0 },  /* zh stop */
    { "\346\260\264", 3, 2, 2, 31 },  /* zh EV.AGUA */
    { "\346\262\241", 3, 3, 0, 0 },  /* zh negation */
    { "\346\262\263", 3, 2, 4, 525 },  /* zh LOC.RIO */
    { "\347\224\237\347\211\251\347\211\271\345\276\201", 12, 6, 0, 0 },  /* zh sensitive */
    { "\347\216\233\344\270\275\344\272\232", 9, 2, 1, 257 },  /* zh PER.MARIA */
    { "\347\224\265\351\207\217\344\275\216", 9, 2, 2, 10 },  /* zh EV.SEM_BATERIA */
    { "\347\264\247\346\200\245\347\232\204", 9, 4, 0, 2 },  /* zh urgency URGENTE */
    { "\347\201\253\347\201\276", 6, 2, 2, 27 },  /* zh EV.FOGO */
    { "\347\210\261\344\275\240", 6, 2, 2, 17 },  /* zh EV.TE_AMO */
    { "\347\210\270\347\210\270", 6, 2, 1, 263 },  /* zh PER.PAI */
    { "\347\213\254\350\207\252", 6, 2, 6, 1287 },  /* zh ST.SOZINHO_ST */
    { "\347\216\260\345\234\250", 6, 2, 3, 768 },  /* zh TIME.AGORA */
    { "\347\226\262\346\203\253", 6, 2, 2, 36 },  /* zh EV.CANSADO */
    { "\347\226\274\347\227\233", 6, 2, 2, 30 },  /* zh EV.DOR */
    { "\347\235\200\347\201\253", 6, 2, 2, 27 },  /* zh EV.FOGO */
    { "\347\235\241\350\247\211", 6, 2, 2, 37 },  /* zh EV.DORMINDO */
    { "\347\241\256\345\256\232", 6, 2, 2, 13 },  /* zh EV.SIM */
    { "\347\241\256\350\256\244", 6, 1, 0, 6 },  /* zh op CONFIRMAR */
    { "\347\242\260\345\244\264", 6, 2, 2, 7 },  /* zh EV.ENCONTRAR */
    { "\347\245\235\350\264\272", 6, 2, 2, 19 },  /* zh EV.PARABENS */
    { "\347\250\215\347\255\211", 6, 2, 2, 25 },  /* zh EV.ESPERA */
    { "\347\250\263\345\256\232", 6, 2, 6, 1286 },  /* zh ST.ESTAVEL */
    { "\347\255\211\346\210\221", 6, 2, 2, 25 },  /* zh EV.ESPERA */
    { "\347\255\211\347\235\200", 6, 2, 2, 6 },  /* zh EV.ESPERANDO */
    { "\347\264\247\346\200\245", 6, 1, 0, 5 },  /* zh op SOCORRO */
    { "\347\264\257\344\272\206", 6, 2, 2, 36 },  /* zh EV.CANSADO */
    { "\347\203\255", 3, 2, 2, 35 },  /* zh EV.CALOR */
    { "\347\226\274", 3, 2, 2, 30 },  /* zh EV.DOR */
    { "\347\232\204", 3, 0, 0, 0 },  /* zh stop */
    { "\347\273\231", 3, 0, 0, 0 },  /* zh stop */
    { "\350\256\241\345\210\222\345\217\230\344\272\206", 12, 2, 2, 54 },  /* zh EV.PLANO_MUDOU */
    { "\350\256\241\345\210\222\346\262\241\345\217\230", 12, 8, 2, 54 },  /* zh EV.PLANO_MUDOU(neg) */
    { "\350\201\224\347\263\273\344\272\2721", 10, 2, 1, 270 },  /* zh PER.HANDLE_1 */
    { "\350\201\224\347\263\273\344\272\2722", 10, 2, 1, 271 },  /* zh PER.HANDLE_2 */
    { "\350\201\224\347\263\273\344\272\2723", 10, 2, 1, 272 },  /* zh PER.HANDLE_3 */
    { "\350\201\224\347\263\273\344\272\2724", 10, 2, 1, 273 },  /* zh PER.HANDLE_4 */
    { "\350\246\201\346\231\232\344\272\206", 9, 2, 2, 3 },  /* zh EV.ATRASADO */
    { "\350\246\201\350\260\210\350\260\210", 9, 2, 2, 58 },  /* zh EV.PRECISO_FALAR */
    { "\350\246\201\350\265\260\344\272\206", 9, 2, 2, 2 },  /* zh EV.SAINDO */
    { "\350\265\260\344\270\242\344\272\206", 9, 2, 2, 11 },  /* zh EV.PERDIDO */
    { "\350\275\246\345\235\217\344\272\206", 9, 2, 2, 43 },  /* zh EV.VEICULO_QUEBROU */
    { "\350\275\246\346\262\241\345\235\217", 9, 8, 2, 43 },  /* zh EV.VEICULO_QUEBROU(neg) */
    { "\350\277\230\346\234\211\347\224\265", 9, 8, 2, 10 },  /* zh EV.SEM_BATERIA(neg) */
    { "\350\277\230\346\262\241\350\265\260", 9, 8, 2, 2 },  /* zh EV.SAINDO(neg) */
    { "\350\200\201\346\235\277", 6, 2, 1, 266 },  /* zh PER.CHEFE */
    { "\350\207\252\345\212\250", 6, 7, 0, 0 },  /* zh authority */
    { "\350\211\257\345\245\275", 6, 2, 6, 1280 },  /* zh ST.BEM */
    { "\350\213\245\346\230\202", 6, 2, 1, 256 },  /* zh PER.JOAO */
    { "\350\220\245\345\234\260", 6, 2, 4, 524 },  /* zh LOC.ACAMPAMENTO */
    { "\350\242\253\345\233\260", 6, 2, 6, 1289 },  /* zh ST.PRESO */
    { "\350\247\201\351\235\242", 6, 2, 2, 7 },  /* zh EV.ENCONTRAR */
    { "\350\255\246\345\257\237", 6, 2, 2, 28 },  /* zh EV.POLICIA */
    { "\350\256\241\345\210\222", 6, 1, 0, 4 },  /* zh op PLANEJAR */
    { "\350\256\260\344\270\213", 6, 1, 0, 3 },  /* zh op LEMBRAR */
    { "\350\256\260\344\275\217", 6, 1, 0, 3 },  /* zh op LEMBRAR */
    { "\350\257\273\344\271\246", 6, 2, 2, 12 },  /* zh EV.ESTUDAR */
    { "\350\260\242\350\260\242", 6, 2, 2, 15 },  /* zh EV.OBRIGADO */
    { "\350\267\257\344\270\212", 6, 2, 4, 516 },  /* zh LOC.ESTRADA */
    { "\350\275\246\344\270\212", 6, 2, 4, 523 },  /* zh LOC.VEICULO */
    { "\350\275\246\347\253\231", 6, 2, 4, 522 },  /* zh LOC.ESTACAO */
    { "\350\277\207\346\235\245", 6, 2, 2, 24 },  /* zh EV.VEM */
    { "\350\277\231\345\221\250", 6, 2, 3, 783 },  /* zh TIME.SEMANA */
    { "\350\277\237\345\210\260", 6, 2, 2, 3 },  /* zh EV.ATRASADO */
    { "\350\277\267\350\267\257", 6, 2, 2, 11 },  /* zh EV.PERDIDO */
    { "\350\246\201", 3, 0, 0, 0 },  /* zh stop */
    { "\350\257\267", 3, 0, 0, 0 },  /* zh stop */
    { "\350\260\201", 3, 5, 1, 0 },  /* zh qword QUEM */
    { "\351\234\200\350\246\201\345\270\256\345\212\251", 12, 2, 2, 5 },  /* zh EV.AJUDA */
    { "\351\201\277\351\232\276\346\211\200", 9, 2, 2, 33 },  /* zh EV.ABRIGO */
    { "\351\233\206\345\220\210\347\202\271", 9, 2, 4, 528 },  /* zh LOC.PONTO_ENCONTRO */
    { "\351\234\200\350\246\201\346\260\264", 9, 2, 2, 31 },  /* zh EV.AGUA */
    { "\351\200\232\347\237\245", 6, 1, 0, 1 },  /* zh op COMUNICAR */
    { "\351\202\273\345\261\205", 6, 2, 1, 269 },  /* zh PER.VIZINHO */
    { "\351\203\275\345\245\275", 6, 2, 2, 4 },  /* zh EV.TUDO_BEM */
    { "\351\206\222\344\272\206", 6, 2, 2, 38 },  /* zh EV.ACORDEI */
    { "\351\242\206\351\230\237", 6, 2, 1, 267 },  /* zh PER.GUIA */
    { "\351\243\237\347\211\251", 6, 2, 2, 32 },  /* zh EV.COMIDA */
    { "\351\245\277\344\272\206", 6, 2, 2, 32 },  /* zh EV.COMIDA */
    { "\351\251\254\344\270\212", 6, 2, 3, 785 },  /* zh TIME.EM_BREVE */
    { "\351\227\256", 3, 1, 0, 2 },  /* zh op PERGUNTAR */
};

const loom_lang_t LOOM_LANG[LOOM_LANG_COUNT] = {
    { "pt", "Portugu\303\252s", 1, 0, 0, 378, 17, 260 },
    { "en", "English", 1, 0, 378, 354, 20, 1500 },
    { "es", "Espa\303\261ol", 1, 0, 732, 277, 18, 560 },
    { "fr", "Fran\303\247ais", 1, 0, 1009, 252, 23, 310 },
    { "it", "Italiano", 1, 0, 1261, 252, 19, 65 },
    { "de", "Deutsch", 1, 0, 1513, 260, 23, 135 },
    { "ja", "\346\227\245\346\234\254\350\252\236", 0, 0, 1773, 276, 27, 125 },
    { "zh", "\344\270\255\346\226\207", 0, 0, 2049, 279, 18, 1350 },
};

const loom_sym_t LOOM_SYM[LOOM_SYM_COUNT] = {
    { 1, 2, { "cheguei", "arrived", "llegue", "arrive", "arrivato", "angekommen", "\347\235\200\343\201\204\343\201\237", "\345\210\260\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\347\235\200\343\201\204\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\345\210\260" } },  /* EV.CHEGUEI */
    { 2, 2, { "saindo", "leaving", "saliendo", "je part", "esco", "ich gehe", "\345\207\272\347\231\272", "\345\207\272\345\217\221" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\207\272\347\231\272\343\201\227\343\201\252\343\201\204", "\350\277\230\346\262\241\350\265\260" } },  /* EV.SAINDO */
    { 3, 2, { "atrasado", "late", "retrasado", "en retard", "in ritardo", "verspatet", "\351\201\205\343\202\214\343\202\213", "\350\277\237\345\210\260" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\351\201\205\343\202\214\343\201\252\343\201\204", "\344\270\215\350\277\237\345\210\260" } },  /* EV.ATRASADO */
    { 4, 2, { "tudo bem", "all good", "todo bien", "tout va bien", "tutto bene", "alles gut", "\345\244\247\344\270\210\345\244\253", "\351\203\275\345\245\275" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\244\247\344\270\210\345\244\253\343\201\230\343\202\203\343\201\252\343\201\204", "\344\270\215\345\244\252\345\245\275" } },  /* EV.TUDO_BEM */
    { 5, 2, { "preciso de ajuda", "help", "ayuda", "aide", "aiuto", "hilfe", "\345\212\251\343\201\221\343\201\246", "\345\270\256\345\277\231" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\212\251\343\201\221\343\201\257\343\201\204\343\202\211\343\201\252\343\201\204", "\344\270\215\351\234\200\350\246\201\345\270\256\345\212\251" } },  /* EV.AJUDA */
    { 6, 2, { "esperando", "waiting", "esperando", "jattends", "aspetto", "warte", "\345\276\205\343\201\243\343\201\246\343\201\204\343\202\213", "\345\234\250\347\255\211" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\276\205\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\345\234\250\347\255\211" } },  /* EV.ESPERANDO */
    { 7, 2, { "encontrar", "meet", "encontrarnos", "rencontre", "incontro", "treffen", "\344\274\232\343\201\206", "\350\247\201\351\235\242" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\344\274\232\343\201\210\343\201\252\343\201\204", "\344\270\215\350\247\201\351\235\242" } },  /* EV.ENCONTRAR */
    { 8, 2, { "a caminho", "on the way", "en camino", "en route", "in arrivo", "auf dem weg", "\345\220\221\343\201\213\343\201\243\343\201\246\343\201\204\343\202\213", "\345\234\250\350\267\257\344\270\212" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\220\221\343\201\213\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\345\234\250\350\267\257\344\270\212" } },  /* EV.A_CAMINHO */
    { 9, 2, { "terminei", "done", "termine", "termine", "finito", "fertig", "\347\265\202\343\202\217\343\201\243\343\201\237", "\345\256\214\346\210\220" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\347\265\202\343\202\217\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\345\256\214\346\210\220" } },  /* EV.TERMINEI */
    { 10, 2, { "sem bateria", "low battery", "sin bateria", "batterie faible", "batteria scarica", "akku leer", "\351\233\273\346\261\240\345\210\207\343\202\214", "\346\262\241\347\224\265\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\351\233\273\346\261\240\343\201\257\343\201\202\343\202\213", "\350\277\230\346\234\211\347\224\265" } },  /* EV.SEM_BATERIA */
    { 11, 2, { "perdido", "lost", "perdido", "perdu", "perso", "verlaufen", "\351\201\223\343\201\253\350\277\267\343\201\243\343\201\237", "\350\277\267\350\267\257" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\350\277\267\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\350\277\267\350\267\257" } },  /* EV.PERDIDO */
    { 12, 2, { "estudar", "study", "estudiar", "etudier", "studiare", "lernen", "\345\213\211\345\274\267", "\345\255\246\344\271\240" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\213\211\345\274\267\343\201\227\343\201\252\343\201\204", "\344\270\215\345\255\246\344\271\240" } },  /* EV.ESTUDAR */
    { 13, 2, { "sim", "yes", "si", "oui", "si", "ja", "\343\201\257\343\201\204", "\345\220\214\346\204\217" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\220\214\346\204\217\343\201\227\343\201\252\343\201\204", "\344\270\215\345\220\214\346\204\217" } },  /* EV.SIM */
    { 14, 2, { "impossivel", "unable", "imposible", "impossible", "impossibile", "unmoglich", "\347\204\241\347\220\206", "\344\270\215\350\241\214" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\347\204\241\347\220\206\343\201\247\343\201\257\343\201\252\343\201\204", "\345\217\257\344\273\245" } },  /* EV.IMPOSSIVEL */
    { 15, 2, { "obrigado", "thanks", "gracias", "merci", "grazie", "danke", "\343\201\202\343\202\212\343\201\214\343\201\250\343\201\206", "\350\260\242\350\260\242" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* EV.OBRIGADO */
    { 16, 2, { "desculpa", "sorry", "perdon", "pardon", "scusa", "entschuldigung", "\343\201\224\343\202\201\343\202\223", "\345\257\271\344\270\215\350\265\267" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* EV.DESCULPA */
    { 17, 2, { "te amo", "love you", "te quiero", "je taime", "ti amo", "ich liebe dich", "\346\204\233\343\201\227\343\201\246\343\201\204\343\202\213", "\346\210\221\347\210\261\344\275\240" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* EV.TE_AMO */
    { 18, 2, { "saudade", "miss you", "te echo de menos", "tu me manque", "mi manchi", "ich vermisse dich", "\344\274\232\343\201\204\343\201\237\343\201\204", "\346\203\263\344\275\240" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* EV.SAUDADE */
    { 19, 2, { "parabens", "congrats", "felicidades", "felicitation", "congratulazioni", "gratulation", "\343\201\212\343\202\201\343\201\247\343\201\250\343\201\206", "\346\201\255\345\226\234" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* EV.PARABENS */
    { 20, 2, { "bom dia", "good morning", "buenos dias", "bonjour", "buongiorno", "guten morgen", "\343\201\212\343\201\257\343\202\210\343\201\206", "\346\227\251\344\270\212\345\245\275" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* EV.BOM_DIA */
    { 21, 2, { "boa noite", "good night", "buenas noches", "bonne nuit", "buonanotte", "gute nacht", "\343\201\212\343\202\204\343\201\231\343\201\277", "\346\231\232\345\256\211" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* EV.BOA_NOITE */
    { 22, 2, { "cuidado", "careful", "cuidado", "attention", "attenzione", "achtung", "\346\263\250\346\204\217", "\345\260\217\345\277\203" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* EV.CUIDADO */
    { 23, 2, { "pare", "stop", "detente", "arrete", "fermati", "stopp", "\346\255\242\343\201\276\343\201\243\343\201\246", "\345\201\234" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\346\255\242\343\201\276\343\202\211\343\201\252\343\201\204", "\344\270\215\350\246\201\345\201\234" } },  /* EV.PARE */
    { 24, 2, { "vem", "come", "ven", "viens", "vieni", "komm", "\346\235\245\343\201\246", "\350\277\207\346\235\245" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\346\235\245\343\201\252\343\201\204\343\201\247", "\344\270\215\350\246\201\350\277\207\346\235\245" } },  /* EV.VEM */
    { 25, 2, { "espera", "wait", "espera", "attends", "aspetta", "warte auf mich", "\345\276\205\343\201\243\343\201\246", "\347\255\211\346\210\221" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\276\205\343\201\237\343\201\252\343\201\204\343\201\247", "\344\270\215\347\224\250\347\255\211" } },  /* EV.ESPERA */
    { 26, 2, { "acidente", "accident", "accidente", "accident", "incidente", "unfall", "\344\272\213\346\225\205", "\344\272\213\346\225\205" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\344\272\213\346\225\205\343\201\247\343\201\257\343\201\252\343\201\204", "\344\270\215\346\230\257\344\272\213\346\225\205" } },  /* EV.ACIDENTE */
    { 27, 2, { "fogo", "fire", "fuego", "feu", "fuoco", "feuer", "\347\201\253\344\272\213", "\347\201\253\347\201\276" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\347\201\253\344\272\213\343\201\247\343\201\257\343\201\252\343\201\204", "\346\262\241\346\234\211\347\201\253" } },  /* EV.FOGO */
    { 28, 2, { "policia", "police", "policia", "police", "polizia", "polizei", "\350\255\246\345\257\237", "\350\255\246\345\257\237" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\350\255\246\345\257\237\343\201\257\344\270\215\350\246\201", "\344\270\215\347\224\250\350\255\246\345\257\237" } },  /* EV.POLICIA */
    { 29, 2, { "medico", "doctor", "medico", "medecin", "medico", "arzt", "\345\214\273\350\200\205", "\345\214\273\347\224\237" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\214\273\350\200\205\343\201\257\344\270\215\350\246\201", "\344\270\215\347\224\250\345\214\273\347\224\237" } },  /* EV.MEDICO */
    { 30, 2, { "dor", "pain", "dolor", "douleur", "dolore", "schmerz", "\347\227\233\343\201\204", "\347\226\274" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\347\227\233\343\201\217\343\201\252\343\201\204", "\344\270\215\347\226\274" } },  /* EV.DOR */
    { 31, 2, { "agua", "water", "agua", "eau", "acqua", "wasser", "\346\260\264", "\346\260\264" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\346\260\264\343\201\257\350\266\263\343\202\212\343\201\246\343\201\204\343\202\213", "\344\270\215\351\234\200\350\246\201\346\260\264" } },  /* EV.AGUA */
    { 32, 2, { "comida", "food", "comida", "nourriture", "cibo", "essen", "\351\243\237\343\201\271\347\211\251", "\351\243\237\347\211\251" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\351\243\237\343\201\271\347\211\251\343\201\257\350\266\263\343\202\212\343\201\246\343\201\204\343\202\213", "\344\270\215\351\234\200\350\246\201\351\243\237\347\211\251" } },  /* EV.COMIDA */
    { 33, 2, { "abrigo", "shelter", "refugio", "abri", "riparo", "unterkunft", "\351\201\277\351\233\243\346\211\200", "\351\201\277\351\232\276\346\211\200" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\351\201\277\351\233\243\346\211\200\343\201\257\344\270\215\350\246\201", "\344\270\215\351\234\200\350\246\201\351\201\277\351\232\276\346\211\200" } },  /* EV.ABRIGO */
    { 34, 2, { "frio", "cold", "frio", "froid", "freddo", "kalt", "\345\257\222\343\201\204", "\345\206\267" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\257\222\343\201\217\343\201\252\343\201\204", "\344\270\215\345\206\267" } },  /* EV.FRIO */
    { 35, 2, { "calor", "hot", "calor", "chaud", "caldo", "heiss", "\346\232\221\343\201\204", "\347\203\255" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\346\232\221\343\201\217\343\201\252\343\201\204", "\344\270\215\347\203\255" } },  /* EV.CALOR */
    { 36, 2, { "cansado", "tired", "cansado", "fatigue", "stanco", "mude", "\347\226\262\343\202\214\343\201\237", "\347\264\257\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\347\226\262\343\202\214\343\201\246\343\201\204\343\201\252\343\201\204", "\344\270\215\347\264\257" } },  /* EV.CANSADO */
    { 37, 2, { "dormindo", "sleeping", "durmiendo", "je dor", "dormo", "schlafe", "\345\257\235\343\202\213", "\347\235\241\350\247\211" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\257\235\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\345\234\250\347\235\241" } },  /* EV.DORMINDO */
    { 38, 2, { "acordei", "awake", "despierto", "reveille", "svegliato", "wach", "\350\265\267\343\201\215\343\201\237", "\351\206\222\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\350\265\267\343\201\215\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\351\206\222" } },  /* EV.ACORDEI */
    { 39, 2, { "trabalhando", "working", "trabajando", "au travail", "lavorando", "arbeite", "\344\273\225\344\272\213\344\270\255", "\345\234\250\345\267\245\344\275\234" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\344\273\225\344\272\213\344\270\255\343\201\247\343\201\257\343\201\252\343\201\204", "\346\262\241\345\234\250\345\267\245\344\275\234" } },  /* EV.TRABALHANDO */
    { 40, 2, { "reuniao", "meeting", "reunion", "reunion", "riunione", "besprechung", "\344\274\232\350\255\260", "\344\274\232\350\256\256" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\344\274\232\350\255\260\343\201\257\343\201\252\343\201\204", "\346\262\241\346\234\211\344\274\232\350\256\256" } },  /* EV.REUNIAO */
    { 41, 2, { "me liga", "call me", "llamame", "appelle moi", "chiamami", "ruf mich an", "\351\233\273\350\251\261\343\201\227\343\201\246", "\346\211\223\347\224\265\350\257\235\347\273\231\346\210\221" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\351\233\273\350\251\261\343\201\227\343\201\252\343\201\204\343\201\247", "\344\270\215\347\224\250\346\211\223\347\224\265\350\257\235" } },  /* EV.LIGA_PRA_MIM */
    { 42, 2, { "nao posso falar", "cannot talk", "no puedo hablar", "je peux pas parler", "non posso parlare", "kann nicht sprechen", "\350\251\261\343\201\233\343\201\252\343\201\204", "\344\270\215\350\203\275\350\257\264\350\257\235" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\350\251\261\343\201\233\343\202\213", "\345\217\257\344\273\245\350\257\264\350\257\235" } },  /* EV.NAO_POSSO_FALAR */
    { 43, 2, { "carro quebrou", "broke down", "se averio", "en panne", "guasto", "panne", "\346\225\205\351\232\234", "\350\275\246\345\235\217\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\346\225\205\351\232\234\343\201\227\343\201\246\343\201\204\343\201\252\343\201\204", "\350\275\246\346\262\241\345\235\217" } },  /* EV.VEICULO_QUEBROU */
    { 44, 2, { "sem sinal", "no signal", "sin senal", "pas de reseau", "senza segnale", "kein signal", "\351\233\273\346\263\242\343\201\214\343\201\252\343\201\204", "\346\262\241\344\277\241\345\217\267" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\351\233\273\346\263\242\343\201\257\343\201\202\343\202\213", "\346\234\211\344\277\241\345\217\267" } },  /* EV.SEM_SINAL */
    { 45, 2, { "achei", "found", "encontrado", "trouve", "trovato", "gefunden", "\350\246\213\343\201\244\343\201\221\343\201\237", "\346\211\276\345\210\260\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\350\246\213\343\201\244\343\201\213\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\346\211\276\345\210\260" } },  /* EV.ACHEI */
    { 46, 2, { "procurando", "searching", "buscando", "je cherche", "cercando", "suche", "\346\216\242\343\201\227\343\201\246\343\201\204\343\202\213", "\345\234\250\346\211\276" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\346\216\242\343\201\227\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\345\234\250\346\211\276" } },  /* EV.PROCURANDO */
    { 47, 2, { "comprei", "bought", "comprado", "achete", "comprato", "gekauft", "\350\262\267\343\201\243\343\201\237", "\344\271\260\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\350\262\267\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\344\271\260" } },  /* EV.COMPREI */
    { 48, 2, { "pagamento", "payment", "pago", "paiement", "pagamento", "zahlung", "\346\224\257\346\211\225\343\201\204", "\344\273\230\346\254\276" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\346\224\257\346\211\225\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\344\273\230\346\254\276" } },  /* EV.PAGAMENTO */
    { 49, 2, { "enviei", "sent", "enviado", "envoye", "inviato", "gesendet", "\351\200\201\343\201\243\343\201\237", "\345\217\221\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\351\200\201\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\345\217\221" } },  /* EV.ENVIEI */
    { 50, 2, { "recebi", "received", "recibido", "recu", "ricevuto", "erhalten", "\345\217\227\343\201\221\345\217\226\343\201\243\343\201\237", "\346\224\266\345\210\260\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\217\227\343\201\221\345\217\226\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\346\224\266\345\210\260" } },  /* EV.RECEBI */
    { 51, 2, { "pronto", "ready", "listo", "pret", "pronto", "bereit", "\346\272\226\345\202\231\343\201\247\343\201\215\343\201\237", "\345\207\206\345\244\207\345\245\275\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\346\272\226\345\202\231\343\201\247\343\201\215\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\345\207\206\345\244\207\345\245\275" } },  /* EV.PRONTO */
    { 52, 2, { "comecou", "started", "empezo", "commence", "iniziato", "begonnen", "\345\247\213\343\201\276\343\201\243\343\201\237", "\345\274\200\345\247\213\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\345\247\213\343\201\276\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\345\274\200\345\247\213" } },  /* EV.COMECOU */
    { 53, 2, { "cancelado", "cancelled", "cancelado", "annulation", "annullato", "abgesagt", "\344\270\255\346\255\242", "\345\217\226\346\266\210\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\344\270\255\346\255\242\343\201\247\343\201\257\343\201\252\343\201\204", "\346\262\241\345\217\226\346\266\210" } },  /* EV.CANCELADO */
    { 54, 2, { "mudou o plano", "plan changed", "cambio el plan", "plan change", "piano cambiato", "plan geandert", "\344\272\210\345\256\232\345\244\211\346\233\264", "\350\256\241\345\210\222\345\217\230\344\272\206" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\344\272\210\345\256\232\345\244\211\346\233\264\343\201\252\343\201\227", "\350\256\241\345\210\222\346\262\241\345\217\230" } },  /* EV.PLANO_MUDOU */
    { 55, 2, { "voltando", "heading back", "volviendo", "je reviens", "torno", "auf dem ruckweg", "\346\210\273\343\202\213", "\345\233\236\345\216\273" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\346\210\273\343\201\243\343\201\246\343\201\204\343\201\252\343\201\204", "\346\262\241\345\234\250\345\233\236\345\216\273" } },  /* EV.VOLTANDO */
    { 56, 2, { "nao entendi", "unclear", "no entiendo", "pas compris", "non ho capito", "nicht verstanden", "\343\202\217\343\201\213\343\202\211\343\201\252\343\201\204", "\344\270\215\346\230\216\347\231\275" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\343\202\217\343\201\213\343\201\243\343\201\237", "\346\230\216\347\231\275\344\272\206" } },  /* EV.NAO_ENTENDI */
    { 57, 2, { "repete", "repeat", "repite", "repete", "ripeti", "wiederhole", "\343\202\202\343\201\206\344\270\200\345\272\246", "\345\206\215\350\257\264\344\270\200\346\254\241" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\347\271\260\343\202\212\350\277\224\343\201\225\343\201\252\343\201\204\343\201\247", "\344\270\215\347\224\250\351\207\215\345\244\215" } },  /* EV.REPETE */
    { 58, 2, { "preciso falar", "need to talk", "necesito hablar", "il faut parler", "devo parlarti", "muss reden", "\350\251\261\343\201\227\343\201\237\343\201\204", "\350\246\201\350\260\210\350\260\210" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\350\251\261\343\201\231\345\277\205\350\246\201\343\201\257\343\201\252\343\201\204", "\344\270\215\347\224\250\350\260\210" } },  /* EV.PRECISO_FALAR */
    { 59, 2, { "algo errado", "something wrong", "algo mal", "quelque chose ne va pas", "qualcosa non va", "etwas stimmt nicht", "\344\275\225\343\201\213\343\201\212\343\201\213\343\201\227\343\201\204", "\346\234\211\351\227\256\351\242\230" }, { NULL, NULL, NULL, NULL, NULL, NULL, "\347\225\260\345\270\270\343\201\252\343\201\227", "\344\270\200\345\210\207\346\255\243\345\270\270" } },  /* EV.ALGO_ERRADO */
    { 60, 2, { "contagem", "headcount", "recuento", "effectif", "conteggio", "anzahl", "\344\272\272\346\225\260", "\344\272\272\346\225\260" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* EV.CONTAGEM */
    { 256, 1, { "joao", "joao", "joao", "joao", "joao", "joao", "\343\202\270\343\203\247\343\202\242\343\203\263", "\350\213\245\346\230\202" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.JOAO */
    { 257, 1, { "maria", "maria", "maria", "maria", "maria", "maria", "\343\203\236\343\203\252\343\202\242", "\347\216\233\344\270\275\344\272\232" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.MARIA */
    { 258, 1, { "ana", "ana", "ana", "ana", "ana", "ana", "\343\202\242\343\203\212", "\345\256\211\345\250\234" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.ANA */
    { 259, 1, { "equipe", "team", "equipo", "equipe", "squadra", "team", "\343\203\201\343\203\274\343\203\240", "\345\233\242\351\230\237" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.EQUIPE */
    { 260, 1, { "familia", "family", "familia", "famille", "famiglia", "familie", "\345\256\266\346\227\217", "\345\256\266\344\272\272" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.FAMILIA */
    { 261, 1, { "todos", "everyone", "todos", "tous", "tutti", "alle", "\345\205\250\345\223\241", "\346\211\200\346\234\211\344\272\272" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.TODOS */
    { 262, 1, { "mae", "mother", "madre", "mere", "madre", "mutter", "\346\257\215", "\345\246\210\345\246\210" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.MAE */
    { 263, 1, { "pai", "father", "padre", "pere", "padre", "vater", "\347\210\266", "\347\210\270\347\210\270" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.PAI */
    { 264, 1, { "filho", "child", "hijo", "enfant", "figlio", "kind", "\345\255\220\344\276\233", "\345\255\251\345\255\220" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.FILHO */
    { 265, 1, { "amigo", "friend", "amigo", "ami", "amico", "freund", "\345\217\213\351\201\224", "\346\234\213\345\217\213" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.AMIGO */
    { 266, 1, { "chefe", "boss", "jefe", "chef", "capo", "chef", "\344\270\212\345\217\270", "\350\200\201\346\235\277" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.CHEFE */
    { 267, 1, { "guia", "guide", "guia", "guide", "guida", "fuhrer", "\343\202\254\343\202\244\343\203\211", "\351\242\206\351\230\237" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.GUIA */
    { 268, 1, { "socorrista", "rescue", "rescate", "secouriste", "soccorritore", "retter", "\346\225\221\345\212\251\351\232\212", "\346\225\221\346\217\264\351\230\237" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.SOCORRISTA */
    { 269, 1, { "vizinho", "neighbour", "vecino", "voisin", "vicino", "nachbar", "\350\277\221\346\211\200", "\351\202\273\345\261\205" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.VIZINHO */
    { 270, 1, { "contato 1", "contact 1", "contacto 1", "contact 1", "contatto 1", "kontakt 1", "\351\200\243\347\265\241\345\205\2101", "\350\201\224\347\263\273\344\272\2721" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.HANDLE_1 */
    { 271, 1, { "contato 2", "contact 2", "contacto 2", "contact 2", "contatto 2", "kontakt 2", "\351\200\243\347\265\241\345\205\2102", "\350\201\224\347\263\273\344\272\2722" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.HANDLE_2 */
    { 272, 1, { "contato 3", "contact 3", "contacto 3", "contact 3", "contatto 3", "kontakt 3", "\351\200\243\347\265\241\345\205\2103", "\350\201\224\347\263\273\344\272\2723" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.HANDLE_3 */
    { 273, 1, { "contato 4", "contact 4", "contacto 4", "contact 4", "contatto 4", "kontakt 4", "\351\200\243\347\265\241\345\205\2104", "\350\201\224\347\263\273\344\272\2724" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.HANDLE_4 */
    { 274, 1, { "grupo 1", "group 1", "grupo 1", "groupe 1", "gruppo 1", "gruppe 1", "\343\202\260\343\203\253\343\203\274\343\203\2271", "\345\260\217\347\273\2041" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.GRUPO_1 */
    { 275, 1, { "grupo 2", "group 2", "grupo 2", "groupe 2", "gruppo 2", "gruppe 2", "\343\202\260\343\203\253\343\203\274\343\203\2272", "\345\260\217\347\273\2042" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* PER.GRUPO_2 */
    { 512, 4, { "casa", "home", "casa", "maison", "casa", "zuhause", "\345\256\266", "\345\256\266\351\207\214" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.CASA */
    { 513, 4, { "trabalho", "work", "trabajo", "bureau", "ufficio", "buro", "\350\201\267\345\240\264", "\345\205\254\345\217\270" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.TRABALHO */
    { 514, 4, { "portao norte", "north gate", "puerta norte", "porte nord", "porta nord", "nordtor", "\345\214\227\351\226\200", "\345\214\227\351\227\250" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.PORTAO_N */
    { 515, 4, { "treino", "gym", "gimnasio", "salle de sport", "palestra", "fitnessstudio", "\343\202\270\343\203\240", "\345\201\245\350\272\253\346\210\277" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.TREINO */
    { 516, 4, { "estrada", "road", "carretera", "route", "strada", "strasse", "\351\201\223\350\267\257", "\350\267\257\344\270\212" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.ESTRADA */
    { 517, 4, { "trilha", "trail", "sendero", "sentier", "sentiero", "wanderweg", "\347\231\273\345\261\261\351\201\223", "\345\260\217\345\276\204" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.TRILHA */
    { 518, 4, { "escola", "school", "escuela", "ecole", "scuola", "schule", "\345\255\246\346\240\241", "\345\255\246\346\240\241" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.ESCOLA */
    { 519, 4, { "hospital", "hospital", "hospital", "hopital", "ospedale", "krankenhaus", "\347\227\205\351\231\242", "\345\214\273\351\231\242" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.HOSPITAL */
    { 520, 4, { "mercado", "market", "mercado", "marche", "mercato", "markt", "\345\272\227", "\345\225\206\345\272\227" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.MERCADO */
    { 521, 4, { "aeroporto", "airport", "aeropuerto", "aeroport", "aeroporto", "flughafen", "\347\251\272\346\270\257", "\346\234\272\345\234\272" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.AEROPORTO */
    { 522, 4, { "estacao", "station", "estacion", "gare", "stazione", "bahnhof", "\351\247\205", "\350\275\246\347\253\231" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.ESTACAO */
    { 523, 4, { "carro", "vehicle", "coche", "voiture", "auto", "auto", "\350\273\212", "\350\275\246\344\270\212" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.VEICULO */
    { 524, 4, { "acampamento", "camp", "campamento", "campement", "campo", "lager", "\343\202\255\343\203\243\343\203\263\343\203\227", "\350\220\245\345\234\260" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.ACAMPAMENTO */
    { 525, 4, { "rio", "river", "rio", "riviere", "fiume", "fluss", "\345\267\235", "\346\262\263" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.RIO */
    { 526, 4, { "cume", "summit", "cumbre", "sommet", "cima", "gipfel", "\351\240\202\344\270\212", "\345\261\261\351\241\266" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.CUME */
    { 527, 4, { "base", "base", "base", "base", "base", "basis", "\345\237\272\345\234\260", "\345\237\272\345\234\260" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.BASE */
    { 528, 4, { "ponto de encontro", "meeting point", "punto de encuentro", "point de rencontre", "punto di incontro", "treffpunkt", "\351\233\206\345\220\210\345\240\264\346\211\200", "\351\233\206\345\220\210\347\202\271" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.PONTO_ENCONTRO */
    { 529, 4, { "entrada", "entrance", "entrada", "entree", "ingresso", "eingang", "\345\205\245\345\217\243", "\345\205\245\345\217\243" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.ENTRADA */
    { 530, 4, { "estacionamento", "parking", "estacionamiento", "parking", "parcheggio", "parkplatz", "\351\247\220\350\273\212\345\240\264", "\345\201\234\350\275\246\345\234\272" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* LOC.ESTACIONAMENTO */
    { 768, 3, { "agora", "now", "ahora", "maintenant", "adesso", "jetzt", "\344\273\212", "\347\216\260\345\234\250" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.AGORA */
    { 769, 3, { "5 minutos", "5 minutes", "5 minutos", "5 minutes", "5 minuti", "5 minuten", "5\345\210\206", "5\345\210\206\351\222\237" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.MIN_5 */
    { 770, 3, { "10 minutos", "10 minutes", "10 minutos", "10 minutes", "10 minuti", "10 minuten", "10\345\210\206", "10\345\210\206\351\222\237" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.MIN_10 */
    { 771, 3, { "30 minutos", "30 minutes", "30 minutos", "30 minutes", "30 minuti", "30 minuten", "30\345\210\206", "30\345\210\206\351\222\237" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.MIN_30 */
    { 772, 3, { "uma hora", "1 hour", "1 hora", "1 heure", "1 ora", "1 stunde", "1\346\231\202\351\226\223", "1\345\260\217\346\227\266" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.HORA_1 */
    { 773, 3, { "hoje", "today", "hoy", "aujourdhui", "oggi", "heute", "\344\273\212\346\227\245", "\344\273\212\345\244\251" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.HOJE */
    { 774, 3, { "amanha", "tomorrow", "manana", "demain", "domani", "morgen", "\346\230\216\346\227\245", "\346\230\216\345\244\251" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.AMANHA */
    { 775, 3, { "hoje a noite", "tonight", "esta noche", "ce soir", "stasera", "heute abend", "\344\273\212\345\244\234", "\344\273\212\346\231\232" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.NOITE */
    { 776, 3, { "1 minuto", "1 minute", "1 minuto", "1 minute", "1 minuto", "1 minute", "1\345\210\206", "1\345\210\206\351\222\237" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.MIN_1 */
    { 777, 3, { "15 minutos", "15 minutes", "15 minutos", "15 minutes", "15 minuti", "15 minuten", "15\345\210\206", "15\345\210\206\351\222\237" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.MIN_15 */
    { 778, 3, { "2 horas", "2 hours", "2 horas", "2 heures", "2 ore", "2 stunden", "2\346\231\202\351\226\223", "2\345\260\217\346\227\266" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.HORA_2 */
    { 779, 3, { "manha", "morning", "por la manana", "le matin", "mattina", "vormittag", "\346\234\235", "\346\227\251\344\270\212" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.MANHA */
    { 780, 3, { "de tarde", "afternoon", "por la tarde", "apres midi", "pomeriggio", "nachmittag", "\345\215\210\345\276\214", "\344\270\213\345\215\210" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.TARDE_DIA */
    { 781, 3, { "ontem", "yesterday", "ayer", "hier", "ieri", "gestern", "\346\230\250\346\227\245", "\346\230\250\345\244\251" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.ONTEM */
    { 782, 3, { "depois", "later", "despues", "plus tard", "dopo", "spater", "\345\276\214\343\201\247", "\344\273\245\345\220\216" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.DEPOIS */
    { 783, 3, { "esta semana", "this week", "esta semana", "cette semaine", "questa settimana", "diese woche", "\344\273\212\351\200\261", "\350\277\231\345\221\250" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.SEMANA */
    { 784, 3, { "fim de semana", "weekend", "fin de semana", "week end", "fine settimana", "wochenende", "\351\200\261\346\234\253", "\345\221\250\346\234\253" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.FIM_DE_SEMANA */
    { 785, 3, { "em breve", "shortly", "en breve", "bientot", "a breve", "bald", "\343\201\231\343\201\220\343\201\253", "\351\251\254\344\270\212" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* TIME.EM_BREVE */
    { 1024, 5, { "0", "0", "0", "0", "0", "0", "0", "0" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.N_0 */
    { 1025, 5, { "1", "1", "1", "1", "1", "1", "1", "1" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.N_1 */
    { 1026, 5, { "2", "2", "2", "2", "2", "2", "2", "2" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.N_2 */
    { 1027, 5, { "3", "3", "3", "3", "3", "3", "3", "3" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.N_3 */
    { 1028, 5, { "4", "4", "4", "4", "4", "4", "4", "4" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.N_4 */
    { 1029, 5, { "5", "5", "5", "5", "5", "5", "5", "5" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.N_5 */
    { 1030, 5, { "10", "10", "10", "10", "10", "10", "10", "10" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.N_10 */
    { 1031, 5, { "20", "20", "20", "20", "20", "20", "20", "20" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.N_20 */
    { 1032, 5, { "50", "50", "50", "50", "50", "50", "50", "50" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.N_50 */
    { 1033, 5, { "100", "100", "100", "100", "100", "100", "100", "100" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.N_100 */
    { 1034, 5, { "pouco", "few", "poco", "peu", "poco", "wenig", "\345\260\221\343\201\227", "\345\260\221" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.POUCO */
    { 1035, 5, { "muito", "many", "mucho", "beaucoup", "molto", "viel", "\343\201\237\343\201\217\343\201\225\343\202\223", "\345\276\210\345\244\232" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.MUITO */
    { 1036, 5, { "metade", "half", "mitad", "moitie", "meta", "halfte", "\345\215\212\345\210\206", "\344\270\200\345\215\212" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.METADE */
    { 1037, 5, { "total", "total", "total", "total", "totale", "gesamt", "\345\205\250\351\203\250", "\345\205\250\351\203\250" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* QTY.TOTAL */
    { 1280, 6, { "bem", "well", "bien", "bien", "bene", "gut", "\345\205\203\346\260\227", "\350\211\257\345\245\275" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.BEM */
    { 1281, 6, { "mal", "unwell", "mal", "mal", "male", "schlecht", "\344\275\223\350\252\277\343\201\214\346\202\252\343\201\204", "\344\270\215\345\245\275" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.MAL */
    { 1282, 6, { "seguro", "safe", "a salvo", "en securite", "al sicuro", "sicher", "\345\256\211\345\205\250", "\345\256\211\345\205\250" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.SEGURO */
    { 1283, 6, { "em perigo", "in danger", "en peligro", "en danger", "in pericolo", "in gefahr", "\345\215\261\351\231\272", "\345\215\261\351\231\251" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.EM_PERIGO */
    { 1284, 6, { "ferido", "injured", "herido", "blesse", "ferito", "verletzt", "\343\201\221\343\201\214", "\345\217\227\344\274\244" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.FERIDO */
    { 1285, 6, { "grave", "critical", "grave", "grave", "grave", "kritisch", "\351\207\215\347\227\207", "\344\270\245\351\207\215" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.GRAVE */
    { 1286, 6, { "estavel", "stable", "estable", "stable", "stabile", "stabil", "\345\256\211\345\256\232", "\347\250\263\345\256\232" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.ESTAVEL */
    { 1287, 6, { "so", "alone", "solo", "seul", "solo", "allein", "\344\270\200\344\272\272", "\347\213\254\350\207\252" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.SOZINHO_ST */
    { 1288, 6, { "acompanhado", "accompanied", "acompanado", "accompagne", "accompagnato", "begleitet", "\345\220\214\350\241\214\343\201\202\343\202\212", "\346\234\211\344\272\272\351\231\252" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.ACOMPANHADO */
    { 1289, 6, { "preso", "trapped", "atrapado", "coince", "bloccato", "eingeschlossen", "\351\226\211\343\201\230\350\276\274\343\202\201\343\202\211\343\202\214\343\201\237", "\350\242\253\345\233\260" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.PRESO */
    { 1290, 6, { "calmo", "calm", "tranquilo", "calme", "calmo", "ruhig", "\350\220\275\343\201\241\347\235\200\343\201\204\343\201\246\343\201\204\343\202\213", "\345\271\263\351\235\231" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.CALMO */
    { 1291, 6, { "com medo", "afraid", "con miedo", "effraye", "spaventato", "angst", "\346\200\226\343\201\204", "\345\256\263\346\200\225" }, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },  /* ST.COM_MEDO */
};

const loom_tmpl_t LOOM_TMPL[LOOM_LANG_COUNT][LOOM_OP_COUNT] = {
    { /* pt */
        { 0, { 0 } },
        { 9, { "%OP%", "%QUEM%", "%URG%", "%NEG%", "%O_QUE%", "em %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%QVAR%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "em %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "em %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%", "%QUEM%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "em %ONDE%", "%QUANDO%", "%QUANTO%", "%QUEM%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "em %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "em %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "em %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
    },
    { /* en */
        { 0, { 0 } },
        { 9, { "%OP%", "%QUEM%", "%URG%", "%NEG%", "%O_QUE%", "at %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%QVAR%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "at %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "at %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%", "%QUEM%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "at %ONDE%", "%QUANDO%", "%QUANTO%", "%QUEM%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "at %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "at %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "at %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
    },
    { /* es */
        { 0, { 0 } },
        { 9, { "%OP%", "%QUEM%", "%URG%", "%NEG%", "%O_QUE%", "en %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%QVAR%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "en %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "en %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%", "%QUEM%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "en %ONDE%", "%QUANDO%", "%QUANTO%", "%QUEM%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "en %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "en %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "en %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
    },
    { /* fr */
        { 0, { 0 } },
        { 9, { "%OP%", "a %QUEM%", "%URG%", "%NEG%", "%O_QUE%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%QVAR%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%", "%QUEM%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%QUEM%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
    },
    { /* it */
        { 0, { 0 } },
        { 9, { "%OP%", "%QUEM%", "%URG%", "%NEG%", "%O_QUE%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%QVAR%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%", "%QUEM%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%QUEM%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "a %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
    },
    { /* de */
        { 0, { 0 } },
        { 9, { "%OP%", "%QUEM%", "%URG%", "%NEG%", "%O_QUE%", "in %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%QVAR%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "in %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "in %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%", "%QUEM%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "in %ONDE%", "%QUANDO%", "%QUANTO%", "%QUEM%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%QUEM%", "%NEG%", "%O_QUE%", "in %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "in %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
        { 9, { "%OP%", "%URG%", "%NEG%", "%O_QUE%", "%QUEM%", "in %ONDE%", "%QUANDO%", "%QUANTO%", "%ESTADO%" } },
    },
    { /* ja */
        { 0, { 0 } },
        { 10, { "%QUEM%\343\201\253", "%QUANDO%", "%ONDE%\343\201\247", "%QUANTO%", "%ESTADO%", "%URG%", "%O_QUE%", "%NEG%", "\343\201\250", "%OP%" } },
        { 11, { "%QUEM%\343\201\257", "%QUANDO%", "%ONDE%\343\201\247", "%QUANTO%", "%ESTADO%", "%URG%", "%NEG%", "%O_QUE%", "%QVAR%", "\343\201\213", "%OP%" } },
        { 10, { "%QUANDO%", "%ONDE%\343\201\247", "%QUANTO%", "%ESTADO%", "%URG%", "%O_QUE%", "%NEG%", "%QUEM%", "\343\202\222", "%OP%" } },
        { 10, { "%QUANDO%", "%ONDE%\343\201\247", "%QUANTO%", "%QUEM%", "%ESTADO%", "%URG%", "%NEG%", "%O_QUE%", "\343\201\256", "%OP%" } },
        { 9, { "%QUEM%\343\201\253", "%QUANDO%", "%ONDE%\343\201\247", "%QUANTO%", "%ESTADO%", "%URG%", "%NEG%", "%O_QUE%", "%OP%" } },
        { 10, { "%QUEM%\343\201\253", "%QUANDO%", "%ONDE%\343\201\247", "%QUANTO%", "%ESTADO%", "%URG%", "%O_QUE%", "%NEG%", "\343\202\222", "%OP%" } },
        { 10, { "%QUEM%\343\201\253", "%QUANDO%", "%ONDE%\343\201\247", "%QUANTO%", "%ESTADO%", "%URG%", "%NEG%", "%O_QUE%", "\343\201\256", "%OP%" } },
    },
    { /* zh */
        { 0, { 0 } },
        { 9, { "%OP%", "%QUEM%", "%URG%", "%QUANDO%", "%ONDE%", "%QUANTO%", "%ESTADO%", "%NEG%", "%O_QUE%" } },
        { 10, { "%OP%", "%QUEM%", "%URG%", "%QUANDO%", "%ONDE%", "%QUANTO%", "%ESTADO%", "%NEG%", "%O_QUE%", "%QVAR%" } },
        { 9, { "%OP%", "%QUANDO%", "%ONDE%", "%QUANTO%", "%ESTADO%", "%NEG%", "%O_QUE%", "%QUEM%", "%URG%" } },
        { 9, { "%OP%", "%QUANDO%", "%ONDE%", "%QUANTO%", "%QUEM%", "%URG%", "%ESTADO%", "%NEG%", "%O_QUE%" } },
        { 9, { "%OP%", "%QUEM%", "%URG%", "%QUANDO%", "%ONDE%", "%QUANTO%", "%ESTADO%", "%NEG%", "%O_QUE%" } },
        { 9, { "%OP%", "%QUEM%", "%URG%", "%QUANDO%", "%ONDE%", "%QUANTO%", "%ESTADO%", "%NEG%", "%O_QUE%" } },
        { 9, { "%OP%", "%QUEM%", "%URG%", "%QUANDO%", "%ONDE%", "%QUANTO%", "%ESTADO%", "%NEG%", "%O_QUE%" } },
    },
};

const char *const LOOM_OP_RENDER[LOOM_LANG_COUNT][LOOM_OP_COUNT] = {
    { NULL, "avisa", "pergunta", "lembra", "planeja", "socorro", "confirma", "cancela" },  /* pt */
    { NULL, "tell", "ask", "remember", "plan", "sos", "confirm", "cancel" },  /* en */
    { NULL, "avisa", "pregunta", "recuerda", "planea", "socorro", "confirma", "cancela" },  /* es */
    { NULL, "dis", "demande", "rappelle", "planifie", "secours", "confirme", "annule" },  /* fr */
    { NULL, "avvisa", "chiedi", "ricorda", "pianifica", "soccorso", "conferma", "annulla" },  /* it */
    { NULL, "sag", "frage", "erinnere", "plane", "notruf", "bestatige", "storniere" },  /* de */
    { NULL, "\344\274\235\343\201\210\343\201\246", "\350\201\236\343\201\204\343\201\246", "\350\246\232\343\201\210\343\201\246", "\344\272\210\345\256\232", "\346\225\221\345\212\251", "\347\242\272\350\252\215", "\345\217\226\343\202\212\346\266\210\343\201\227" },  /* ja */
    { NULL, "\345\221\212\350\257\211", "\351\227\256", "\350\256\260\344\275\217", "\350\256\241\345\210\222", "\346\261\202\346\225\221", "\347\241\256\350\256\244", "\345\217\226\346\266\210" },  /* zh */
};

const char *const LOOM_NEG_RENDER[LOOM_LANG_COUNT] = {
    "nao", "not", "no", "pas", "non", "nicht", "\343\201\252\343\201\204", "\346\262\241"
};

const char *const LOOM_URG_RENDER[LOOM_LANG_COUNT][4] = {
    { "", "rapido", "urgente", "" },  /* pt */
    { "", "quick", "urgent", "" },  /* en */
    { "", "rapido", "urgente", "" },  /* es */
    { "", "vite", "urgent", "" },  /* fr */
    { "", "presto", "urgente", "" },  /* it */
    { "", "schnell", "dringend", "" },  /* de */
    { "", "\346\200\245\343\201\216", "\347\267\212\346\200\245\343\201\253", "" },  /* ja */
    { "", "\345\277\253", "\347\264\247\346\200\245\347\232\204", "" },  /* zh */
};

const char *const LOOM_QW_RENDER[LOOM_LANG_COUNT][LOOM_ROLE_COUNT] = {
    { "", "quem", "", "quando", "onde", "quantos", "" },  /* pt */
    { "", "who", "", "when", "where", "how many", "" },  /* en */
    { "", "quien", "", "cuando", "donde", "cuantos", "" },  /* es */
    { "", "qui", "", "quand", "ou", "combien", "" },  /* fr */
    { "", "chi", "", "quando", "dove", "quanti", "" },  /* it */
    { "", "wer", "", "wann", "wo", "wie viele", "" },  /* de */
    { "", "\343\201\240\343\202\214", "", "\343\201\204\343\201\244", "\343\201\251\343\201\223", "\344\275\225\344\272\272", "" },  /* ja */
    { "", "\350\260\201", "", "\344\273\200\344\271\210\346\227\266\345\200\231", "\345\223\252\351\207\214", "\345\244\232\345\260\221", "" },  /* zh */
};

const loom_bucket_t LOOM_BUCKET[LOOM_LANG_COUNT][256] = {
    { /* pt */
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,1}, {1,7}, {8,3}, {11,2}, {13,1}, {14,3}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {17,40}, {57,8}, {65,45}, {110,20}, {130,35}, {165,17}, {182,11},
        {193,4}, {197,7}, {204,3}, {0,0}, {207,7}, {214,23}, {237,9}, {246,8},
        {254,36}, {290,9}, {299,7}, {306,22}, {328,30}, {358,5}, {363,15}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
    },
    { /* en */
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {378,1}, {379,7}, {386,3}, {389,2}, {391,1}, {392,3}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {395,28}, {423,11}, {434,34}, {468,5}, {473,6}, {479,17}, {496,9},
        {505,24}, {529,25}, {554,3}, {557,2}, {559,10}, {569,17}, {586,18}, {604,10},
        {614,13}, {627,1}, {628,14}, {642,31}, {673,26}, {699,5}, {704,2}, {706,23},
        {0,0}, {729,3}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
    },
    { /* es */
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {732,1}, {733,7}, {740,3}, {743,2}, {745,1}, {746,3}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {749,28}, {777,6}, {783,37}, {820,13}, {833,31}, {864,6}, {870,9},
        {879,7}, {886,4}, {890,2}, {0,0}, {892,6}, {898,16}, {914,9}, {923,1},
        {924,21}, {945,3}, {948,12}, {960,15}, {975,16}, {991,4}, {995,9}, {0,0},
        {0,0}, {1004,5}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
    },
    { /* fr */
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {1009,1}, {1010,7}, {1017,3}, {1020,2}, {1022,1}, {1023,3}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {1026,29}, {1055,13}, {1068,23}, {1091,13}, {1104,21}, {1125,6}, {1131,7},
        {1138,2}, {1140,4}, {1144,17}, {0,0}, {1161,5}, {1166,16}, {1182,3}, {1185,3},
        {1188,24}, {1212,5}, {1217,10}, {1227,13}, {1240,9}, {1249,4}, {1253,7}, {1260,1},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
    },
    { /* it */
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {1261,1}, {1262,7}, {1269,3}, {1272,2}, {1274,1}, {1275,3}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {1278,27}, {1305,9}, {1314,30}, {1344,14}, {1358,4}, {1362,12}, {1374,9},
        {1383,2}, {1385,17}, {1402,1}, {0,0}, {1403,4}, {1407,15}, {1422,8}, {1430,3},
        {1433,21}, {1454,5}, {1459,6}, {1465,28}, {1493,9}, {1502,5}, {1507,6}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
    },
    { /* de */
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {1513,1}, {1514,7}, {1521,3}, {1524,2}, {1526,1}, {1527,3}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {1530,23}, {1553,19}, {1572,1}, {1573,9}, {1582,14}, {1596,12}, {1608,15},
        {1623,12}, {1635,13}, {1648,3}, {1651,24}, {1675,3}, {1678,14}, {1692,8}, {1700,2},
        {1702,8}, {0,0}, {1710,3}, {1713,19}, {1732,5}, {1737,6}, {1743,10}, {1753,14},
        {0,0}, {0,0}, {1767,6}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
    },
    { /* ja */
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {1773,1}, {1774,7}, {1781,3}, {1784,2}, {1786,1}, {1787,3}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {1790,49}, {1839,28}, {1867,64}, {1931,32}, {1963,31},
        {1994,24}, {2018,31}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
    },
    { /* zh */
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {2049,1}, {2050,7}, {2057,3}, {2060,2}, {2062,1}, {2063,3}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {2066,47}, {2113,75}, {2188,63}, {2251,27},
        {2278,37}, {2315,13}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
        {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
    },
};

int loom_lang_index(const char *tag)
{
    uint8_t i;
    if (!tag) return -1;
    for (i = 0; i < LOOM_LANG_COUNT; i++) {
        if (strcmp(LOOM_LANG[i].tag, tag) == 0) return (int)i;
    }
    return -1;
}

static const loom_sym_t *loom_find(uint16_t sym)
{
    uint16_t lo = 0, hi = LOOM_SYM_COUNT;
    while (lo < hi) {                      /* LOOM_SYM esta ordenado por sym */
        uint16_t mid = (uint16_t)(lo + (hi - lo) / 2u);
        if (LOOM_SYM[mid].sym == sym) return &LOOM_SYM[mid];
        if (LOOM_SYM[mid].sym < sym) lo = (uint16_t)(mid + 1u);
        else hi = mid;
    }
    return NULL;
}

const char *loom_sym_render(uint16_t sym, uint8_t lang)
{
    const loom_sym_t *s;
    if (lang >= LOOM_LANG_COUNT) return NULL;
    s = loom_find(sym);
    return s ? s->render[lang] : NULL;
}

const char *loom_sym_neg(uint16_t sym, uint8_t lang)
{
    const loom_sym_t *s;
    if (lang >= LOOM_LANG_COUNT) return NULL;
    s = loom_find(sym);
    return s ? s->neg[lang] : NULL;
}
