/*------------------------------------------------------
                    .*TRABALHO DE SO*.
                 ALUNAS: ISABELLA E LUDMILA
                            2022
--------------------------------------------------------*/


#pragma comment(lib,"pthreadVC2.lib")

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

//------------------------------------------
//Definição do tamanho da matriz 
#define COL_MATRIZ 20000
#define LIN_MATRIZ 10000
//------------------------------------------
//Definição do tamanho dos macroblocos
#define COL_MACRO_BLOCO 100
#define LIN_MACRO_BLOCO 100
//------------------------------------------
//Definição do número de threads
#define NUM_THREADS 2
//------------------------------------------
#define QUANT_MACRO_BLOCO (COL_MATRIZ * LIN_MATRIZ) / (COL_MACRO_BLOCO * LIN_MACRO_BLOCO)
#define SEED 1

//Declaração de variáveis globais
int cont_primos; // Conta total de primos na busca sequencial
int t; // contador utilizado em buscaMacroBloco()
int estado[QUANT_MACRO_BLOCO]; // Vetor com valores 0 para macrobloco livre e 1 pra ocupado
int(*matriz)[COL_MATRIZ]; 
int pv_threads; // Pivô de threads
int conta_primos_threads; // Conta total de primos na busca paralela

pthread_mutex_t mutex;
pthread_mutex_t mutex_estado;


//Função que verifica se o número é primo
bool ehPrimo(int num) {
    int cont = 1;
    int i = 0;

    if (num == 0 || num == 1) return false;

    for (i = 2; i <= (sqrt(num)); i++) {
        if (num % i == 0) {
            cont = 0;
            break;
        }
    }

    if (cont == 1) return true;
    else return false;

}

//Struct construída para armazenaar dados do macrobloco
typedef struct {

    int index_inicial[2];

} macroBloco;

macroBloco* mb;

//Função que salva os dados dos macroblocos da matriz
int buscaMacroBloco() {
    int lin;
    int col;
    int pl = 0;
    int pc = 0;

    mb = malloc(sizeof(macroBloco) * QUANT_MACRO_BLOCO);
    t = 0;

    for (int i = 0; i < LIN_MATRIZ; i += LIN_MACRO_BLOCO) {
        pc = 0;
        for (int j = 0; j < COL_MATRIZ; j += COL_MACRO_BLOCO) {
            for (lin = 0; lin < LIN_MACRO_BLOCO; lin++) {
                for (col = 0; col < COL_MACRO_BLOCO; col++) {


                    if (col == 0 && lin == 0) {
                        mb[t].index_inicial[0] = pl + lin;
                        mb[t].index_inicial[1] = pc + col;
                    }

                }
            }
            pc += COL_MACRO_BLOCO;
            t++;
        }
        pl += LIN_MACRO_BLOCO;
    }


}

//Função que é atribuida as threads para calcular o número de primos nos macroblocos
void* ehPrimoThreads(void* param) {

    int contador_primos = 0;
    int i = 0;


    while (pv_threads < QUANT_MACRO_BLOCO) {

        i = 0;

        pthread_mutex_lock(&mutex_estado);
        if (pv_threads < NUM_THREADS) {
            i = pv_threads;
        }
        else {
            while (estado[i] != 0) {
                i++;
            }
        }
        pv_threads++;
        estado[i] = 1;
        pthread_mutex_unlock(&mutex_estado);

        
        for (int lin = 0; lin < LIN_MACRO_BLOCO; lin++) {
            for (int col = 0; col < COL_MACRO_BLOCO; col++) {
                
                if (ehPrimo(matriz[mb[i].index_inicial[0] + lin][mb[i].index_inicial[1] + col]) == true) {
                    pthread_mutex_lock(&mutex);
                    conta_primos_threads++;
                    pthread_mutex_unlock(&mutex);

                }
            }
        }

    }

}

//Função principal
int main() {

    //Declaração e inicialização de variáveis
    srand(SEED);

    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_estado, NULL);

    pv_threads = 0;
    conta_primos_threads = 0;
    clock_t comeco_seq = 0;
    clock_t final_seq = 0;
    double total_gasto_seq = 0.0;
    clock_t comeco_para = 0;
    clock_t final_para = 0;
    double total_gasto_para = 0.0;



    //Função que cria a matriz
    matriz = malloc(sizeof * matriz * LIN_MATRIZ);

    for (int i = 0; i < LIN_MATRIZ; i++) {
        for (int j = 0; j < COL_MATRIZ; j++)
            matriz[i][j] = rand() % 320000;
    }


    //Calculo dos primos de forma sequencial
    comeco_seq = clock(); //guarda a contagem de tempo inicial

    for (int i = 0; i < LIN_MATRIZ; i++) {
        for (int j = 0; j < COL_MATRIZ; j++) {
            if (ehPrimo(matriz[i][j]) == true) cont_primos++;
        }
    }

    final_seq = clock(); //guarda a contagem de tempo final
    //calculo do tempo gasto
    total_gasto_seq = (double)(final_seq - comeco_seq) / CLOCKS_PER_SEC;
    
    //Print dos dados já obtidos
    printf("Total gasto na busca sequencial: %f\n", total_gasto_seq);
    printf("Primos encontrados na busca sequencial: %d\n", cont_primos);
    printf("Quantidade de macroblocos %dx%d da matriz: %d\n", COL_MACRO_BLOCO, LIN_MACRO_BLOCO, QUANT_MACRO_BLOCO);
    buscaMacroBloco();

    //Criação de threds e atribuição de tarefas 
    for (int k = 0; k < NUM_THREADS; k++) {
        pthread_attr_init(&attr);
        pthread_create(&threads[k], &attr, ehPrimoThreads, NULL);
    }

    comeco_para = clock();

    for (int l = 0; l < NUM_THREADS; l++) {
        pthread_join(threads[l], NULL);
    }

    //Print dos dados obtidos
    printf("Total primos busca paralela: %d\n", conta_primos_threads);
    final_para = clock();
    total_gasto_para = (double)(final_para - comeco_para) / CLOCKS_PER_SEC; //Calculo do tempo gasto
    printf("Total gasto na busca paralela: %f\n", total_gasto_para);
    


    free(matriz);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_estado);

    system("pause");
    return 0;
}