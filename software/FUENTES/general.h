#ifndef GENERAL_H
#define GENERAL_H

#include<cstring>
#include<string>
#include<optional>
#include<random>
#include<vector>
#include<algorithm>
using namespace std;

#define MAX_EVALS 100000

extern default_random_engine generator;
extern unsigned f_objetivo_evals;

inline void init_rand(int seed){
    generator.seed(seed);
}

inline float random_float(float min, float max){
    uniform_real_distribution<float> distribution(min,max);
    return distribution(generator);
}

inline int random_int(int min, int max){
    uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}

inline void barajar(int* principio, int* final){
    shuffle (principio, final, generator);
}

//El tamaño de solucion debe ser mayor o igual a sub_size
bool any_empty_cluster(const int n_clust, const unsigned* solucion, const int sub_size);

void generar_solucion_aleatoria_valida(const int n_clust, unsigned* solucion, const int sub_size);

//p1 y p2 deben ser arrays de tamaño 3
float distancia(const float* p_1, const float* p_2, const unsigned dim);
float media(const float* array, const int array_size);
void sumar(const float* v_1, const float* v_2, float* resultado, unsigned dim);
vector<float> maximo_por_columnas(vector<vector<float> >& matrix, const unsigned size, const unsigned dim);
vector<float> minimo_por_columnas(vector<vector<float> >& matrix, const unsigned size, const unsigned dim);

float desviacion_media(const unsigned* solucion, const vector<vector<float> >& dat,
                       const float* const* centroides, const unsigned n_clust,
                       const unsigned sub_size, const unsigned dim);
unsigned infeasibility_total(const vector<vector<int> > &lista_rest, const unsigned* solucion);
inline float f_objetivo( const unsigned* solucion, const vector<vector<float> >& dat,
                  const float* const* centroides, const unsigned n_clust,
                  const unsigned sub_size, const unsigned dim, 
                  const float peso_infeas, const vector<vector<int> > &lista_rest){

    f_objetivo_evals++;
    return ( desviacion_media(solucion, dat, centroides, n_clust, sub_size, dim) + peso_infeas*infeasibility_total(lista_rest, solucion) );
}

bool misma_solucion(unsigned* sol1, unsigned* sol2, unsigned size);
inline void mutar(unsigned* solucion, unsigned size, unsigned n_clust){

    unsigned n_muts = size*0.1f;
    unsigned primer_elemento = random_int(0, size-1);
    for(unsigned i=0; i<n_muts; i++){
        unsigned indice = (primer_elemento+i) % size;
        solucion[indice] = random_int(0, n_clust-1);
    }

}
void recalcular_centroides(float** centroides, const int n_clust, 
                           const unsigned* solucion, const unsigned sub_size, 
                           const vector<vector<float> >& dat, const unsigned dim);

void read_matrix_from_txt(const string& path, vector<vector<float> >& matrix, unsigned &dim_1, unsigned &dim_2);

#endif