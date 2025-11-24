#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- Constantes e Definições ---
#define TAMANHO_HASH 10
#define PISTAS_MINIMAS 2

// --- Estrutura da Pista (Usada na BST e na Tabela Hash) ---
typedef struct PistaNode {
    char nome[50];
    char suspeito[50];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// --- Estrutura do Cômodo (Usada na Árvore Binária do Mapa) ---
typedef struct ComodoNode {
    char nome[50];
    char pista_estatica[50]; // Pista que será coletada neste cômodo
    struct ComodoNode *esquerda;
    struct ComodoNode *direita;
} ComodoNode;

// --- Estrutura da Tabela Hash (Usada para armazenar Pista -> Suspeito) ---
// Usaremos encadeamento simples para lidar com colisões
typedef struct HashEntry {
    char pista[50];
    char suspeito[50];
    struct HashEntry *proximo;
} HashEntry;

HashEntry *tabelaHash[TAMANHO_HASH]; // Tabela hash global

// --- Protótipos das Funções ---

// Funções do Mapa (Árvore Binária)
ComodoNode *criarSala(const char *nome, const char *pista);
void explorarSalas(ComodoNode *salaAtual, PistaNode **raizPistas);

// Funções da BST de Pistas
PistaNode *criarPista(const char *nomePista, const char *nomeSuspeito);
PistaNode *inserirPista(PistaNode *raiz, PistaNode *novaPista);
void listarPistasColetadas(PistaNode *raiz); // Travessia InOrder

// Funções da Tabela Hash
int gerarHash(const char *chave);
void inserirNaHash(const char *pista, const char *suspeito);
char *encontrarSuspeito(const char *pista);

// Funções de Gerenciamento do Jogo
char *coletarPista(const char *nomeComodo);
int verificarSuspeitoFinal(PistaNode *raizPistas, const char *acusado);

// Funções de Liberação de Memória
void liberarBST(PistaNode *raiz);
void liberarMapa(ComodoNode *raiz);
void liberarHash();

// --- Implementação das Funções do Mapa (Árvore Binária) ---

/**
 * @brief Cria dinamicamente um novo cômodo para a mansão.
 * @param nome Nome identificador do cômodo.
 * @param pista Pista estática associada a este cômodo.
 * @return ComodoNode* O ponteiro para o novo nó alocado.
 */
ComodoNode *criarSala(const char *nome, const char *pista) {
    ComodoNode *novo = (ComodoNode *)malloc(sizeof(ComodoNode));
    if (novo == NULL) {
        perror("Erro ao alocar memoria para Comodo");
        exit(EXIT_FAILURE);
    }
    strncpy(novo->nome, nome, 49);
    strncpy(novo->pista_estatica, pista, 49);
    novo->esquerda = NULL;
    novo->direita = NULL;
    return novo;
}

/**
 * @brief Navega pela árvore do mapa e coleta pistas.
 * @param salaAtual O cômodo atual na exploração.
 * @param raizPistas O ponteiro para a raiz da BST de pistas.
 */
void explorarSalas(ComodoNode *salaAtual, PistaNode **raizPistas) {
    char acao;

    if (salaAtual == NULL) return;

    printf("\n[Exploracao] Voce esta em: **%s**\n", salaAtual->nome);
    printf("---------------------------------------------------\n");

    // Coleta automática da pista
    if (strlen(salaAtual->pista_estatica) > 0) {
        char *nomePista = salaAtual->pista_estatica;
        char *nomeSuspeito = encontrarSuspeito(nomePista);

        if (nomeSuspeito != NULL) {
            // Se a pista já foi inserida, não a inserimos novamente
            // A simplificação aqui é que verificamos na hash, mas a BST
            // evitará a inserção duplicada pelo nome da pista (chave).
            
            PistaNode *novaPista = criarPista(nomePista, nomeSuspeito);
            *raizPistas = inserirPista(*raizPistas, novaPista);

            printf("PISTA ENCONTRADA: '%s' que aponta para: %s\n", nomePista, nomeSuspeito);
            // Zera a pista estática para evitar coleta duplicada
            salaAtual->pista_estatica[0] = '\0'; 
        } else {
             printf("[INFO] Este comodo nao possui mais pistas a serem coletadas.\n");
        }
    } else {
        printf("[INFO] Nenhuma pista a ser coletada neste comodo.\n");
    }
    printf("---------------------------------------------------\n");

    do {
        printf("Acao (e) ir ESQUERDA, (d) ir DIREITA, (s) SAIR e julgar: ");
        // Leitura segura do primeiro caractere (ignora o resto da linha)
        if (scanf(" %c", &acao) != 1) acao = ' ';
        while (getchar() != '\n'); // Limpa o buffer
        
        acao = tolower(acao);

        if (acao == 'e' || acao == 'd') {
            if (acao == 'e' && salaAtual->esquerda) {
                explorarSalas(salaAtual->esquerda, raizPistas);
                return; // Retorna após a exploração do ramo esquerdo
            } else if (acao == 'd' && salaAtual->direita) {
                explorarSalas(salaAtual->direita, raizPistas);
                return; // Retorna após a exploração do ramo direito
            } else {
                printf("[AVISO] Nao ha comodo nessa direcao. Escolha outra acao.\n");
            }
        } else if (acao == 's') {
            printf("\nMissao de exploracao encerrada. Indo para a fase de julgamento...\n");
            return;
        } else {
            printf("[ERRO] Acao invalida.\n");
        }
    } while (1); // Loop interno para garantir uma ação válida
}

// --- Implementação das Funções da BST de Pistas ---

/**
 * @brief Cria um novo nó para a BST de pistas.
 * @param nomePista Nome da pista (chave da BST).
 * @param nomeSuspeito Suspeito associado.
 * @return PistaNode* O ponteiro para o novo nó.
 */
PistaNode *criarPista(const char *nomePista, const char *nomeSuspeito) {
    PistaNode *novo = (PistaNode *)malloc(sizeof(PistaNode));
    if (novo == NULL) {
        perror("Erro ao alocar memoria para PistaNode");
        exit(EXIT_FAILURE);
    }
    strncpy(novo->nome, nomePista, 49);
    strncpy(novo->suspeito, nomeSuspeito, 49);
    novo->esquerda = novo->direita = NULL;
    return novo;
}

/**
 * @brief Insere uma pista na Árvore de Busca Binária (BST).
 * @param raiz Raiz atual da BST.
 * @param novaPista O nó de pista a ser inserido.
 * @return PistaNode* A nova raiz da subárvore.
 */
PistaNode *inserirPista(PistaNode *raiz, PistaNode *novaPista) {
    if (raiz == NULL) {
        return novaPista; // Encontrado local de inserção
    }

    int comparacao = strcmp(novaPista->nome, raiz->nome);

    if (comparacao < 0) {
        raiz->esquerda = inserirPista(raiz->esquerda, novaPista);
    } else if (comparacao > 0) {
        raiz->direita = inserirPista(raiz->direita, novaPista);
    } else {
        // Pista duplicada, não faz nada e libera a memória do nó duplicado
        free(novaPista); 
    }
    return raiz;
}

/**
 * @brief Lista as pistas coletadas em ordem alfabética (Travessia InOrder).
 * @param raiz Raiz da BST.
 */
void listarPistasColetadas(PistaNode *raiz) {
    if (raiz != NULL) {
        listarPistasColetadas(raiz->esquerda);
        printf("  - %-25s (Aponta para: %s)\n", raiz->nome, raiz->suspeito);
        listarPistasColetadas(raiz->direita);
    }
}

// --- Implementação das Funções da Tabela Hash ---

/**
 * @brief Função de hash simples para strings.
 * @param chave A string (nome da pista) a ser hasheada.
 * @return int O índice na tabela hash.
 */
int gerarHash(const char *chave) {
    unsigned long hash = 5381;
    int c;
    while ((c = *chave++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % TAMANHO_HASH;
}

/**
 * @brief Insere a associação pista/suspeito na tabela hash.
 * @param pista A chave (nome da pista).
 * @param suspeito O valor (nome do suspeito).
 */
void inserirNaHash(const char *pista, const char *suspeito) {
    int indice = gerarHash(pista);

    // Cria nova entrada
    HashEntry *novaEntrada = (HashEntry *)malloc(sizeof(HashEntry));
    if (novaEntrada == NULL) {
        perror("Erro ao alocar memoria para HashEntry");
        exit(EXIT_FAILURE);
    }
    strncpy(novaEntrada->pista, pista, 49);
    strncpy(novaEntrada->suspeito, suspeito, 49);
    
    // Adiciona no início da lista encadeada (encadeamento simples)
    novaEntrada->proximo = tabelaHash[indice];
    tabelaHash[indice] = novaEntrada;
}

/**
 * @brief Consulta o suspeito correspondente a uma pista na tabela hash.
 * @param pista O nome da pista a ser procurada.
 * @return char* O nome do suspeito ou NULL se não encontrado.
 */
char *encontrarSuspeito(const char *pista) {
    int indice = gerarHash(pista);
    HashEntry *atual = tabelaHash[indice];

    while (atual != NULL) {
        if (strcmp(atual->pista, pista) == 0) {
            return atual->suspeito; // Suspeito encontrado
        }
        atual = atual->proximo;
    }
    return NULL; // Pista não encontrada na hash
}

// --- Implementação das Funções de Gerenciamento do Jogo ---

/**
 * @brief Conta recursivamente as pistas que apontam para o suspeito acusado.
 * @param raiz Raiz da BST de pistas coletadas.
 * @param acusado O nome do suspeito acusado.
 * @return int O número de pistas que ligam ao acusado.
 */
int contarPistasPorSuspeito(PistaNode *raiz, const char *acusado) {
    if (raiz == NULL) {
        return 0;
    }

    int count = 0;
    if (strcmp(raiz->suspeito, acusado) == 0) {
        count = 1; // Esta pista aponta para o acusado
    }

    // Soma a contagem dos sub-ramos
    count += contarPistasPorSuspeito(raiz->esquerda, acusado);
    count += contarPistasPorSuspeito(raiz->direita, acusado);

    return count;
}

/**
 * @brief Conduz a fase final do julgamento e verifica a evidência.
 * @param raizPistas Raiz da BST de pistas coletadas.
 * @return int 1 se a acusação foi válida, 0 caso contrário.
 */
int verificarSuspeitoFinal(PistaNode *raizPistas, const char *acusado) {
    int totalPistas = contarPistasPorSuspeito(raizPistas, acusado);
    
    printf("\n--- VERIFICACAO FINAL DA EVIDENCIA ---\n");
    printf("Suspeito Acusado: %s\n", acusado);
    printf("Total de pistas encontradas contra o acusado: %d\n", totalPistas);

    if (totalPistas >= PISTAS_MINIMAS) {
        printf("\n*** CASO ENCERRADO! ***\n");
        printf("A acusacao contra %s e sustentada por %d evidencias. Vitoria do Detective Quest!\n", 
               acusado, totalPistas);
        return 1;
    } else {
        printf("\n!!! ACUSACAO FRACASSADA !!!\n");
        printf("E necessario pelo menos %d pistas para sustentar a acusacao. O culpado escapou!\n", PISTAS_MINIMAS);
        return 0;
    }
}

// --- Funções de Liberação de Memória ---

void liberarBST(PistaNode *raiz) {
    if (raiz != NULL) {
        liberarBST(raiz->esquerda);
        liberarBST(raiz->direita);
        free(raiz);
    }
}

void liberarMapa(ComodoNode *raiz) {
    if (raiz != NULL) {
        liberarMapa(raiz->esquerda);
        liberarMapa(raiz->direita);
        free(raiz);
    }
}

void liberarHash() {
    for (int i = 0; i < TAMANHO_HASH; i++) {
        HashEntry *atual = tabelaHash[i];
        HashEntry *temp;
        while (atual != NULL) {
            temp = atual;
            atual = atual->proximo;
            free(temp);
        }
        tabelaHash[i] = NULL;
    }
}

// --- Função Principal ---

int main() {
    // Inicialização da Tabela Hash
    for (int i = 0; i < TAMANHO_HASH; i++) {
        tabelaHash[i] = NULL;
    }

    // 1. Definição das Pistas Estáticas e Suspeitos (Regras Codificadas)
    // Inserindo na Hash: Pista (Chave) -> Suspeito (Valor)
    inserirNaHash("Fio de seda vermelho", "Dona Agatha");
    inserirNaHash("Residuo de graxa", "Mordomo James");
    inserirNaHash("Luva de couro preta", "Dona Agatha");
    inserirNaHash("Embalagem de remedio", "Cozinheira Marie");
    inserirNaHash("Cachimbo quebrado", "Mordomo James");
    inserirNaHash("Mancha de po de cafe", "Cozinheira Marie");
    inserirNaHash("Bilhete com coordenadas", "Mordomo James");

    // 2. Construção do Mapa da Mansão (Árvore Binária Fixa)
    /*
             Hall Central
             /        \
         Biblioteca   Sala de Jantar
         /  \          /   \
    Quarto P. Cozinha  Escritorio Sala de Estar
    */
    ComodoNode *raizMapa = criarSala("Hall Central", "Fio de seda vermelho");
    raizMapa->esquerda = criarSala("Biblioteca", "Residuo de graxa");
    raizMapa->direita = criarSala("Sala de Jantar", "Luva de couro preta");
    raizMapa->esquerda->esquerda = criarSala("Quarto Principal", "Embalagem de remedio");
    raizMapa->esquerda->direita = criarSala("Cozinha", "Mancha de po de cafe");
    raizMapa->direita->esquerda = criarSala("Escritorio", "Cachimbo quebrado");
    raizMapa->direita->direita = criarSala("Sala de Estar", "Bilhete com coordenadas");
    
    // Raiz da BST de Pistas (Inicialmente NULL)
    PistaNode *raizPistas = NULL;
    char acusado[50];
    
    printf("====================================================\n");
    printf("         BEM-VINDO AO DETECTIVE QUEST: A MANSÃO\n");
    printf("         Colete 2 pistas para acusar o culpado.\n");
    printf("====================================================\n");

    // 3. Exploração Interativa
    // Passa a raiz da BST por referência (**raizPistas) para que as pistas coletadas sejam salvas
    explorarSalas(raizMapa, &raizPistas);

    // 4. Fase de Acusação
    printf("\n\n=============== FASE DE JULGAMENTO ===============\n");
    printf("Pistas Coletadas (em ordem alfabetica):\n");
    listarPistasColetadas(raizPistas);
    printf("---------------------------------------------------\n");

    printf("Quem voce acusa? (Dona Agatha, Mordomo James, Cozinheira Marie): ");
    if (fgets(acusado, 50, stdin) == NULL) {
        strncpy(acusado, "Ninguem", 49);
    }
    // Remove o newline
    acusado[strcspn(acusado, "\n")] = 0;

    // 5. Verificação Final
    // A função retorna 1 se a acusação for válida (>= 2 pistas)
    verificarSuspeitoFinal(raizPistas, acusado);

    // 6. Liberação de Memória
    liberarMapa(raizMapa);
    liberarBST(raizPistas);
    liberarHash();
    
    printf("\nMemoria liberada. Programa encerrado.\n");

    return 0;
}