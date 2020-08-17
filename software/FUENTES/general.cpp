#include<iostream>
#include<cmath>
#include<cstring>
//#include<string>
#include<vector>
#include<algorithm>
#include<random>
#include<fstream>
#include<chrono>
#include<cfloat>
#include<climits>
#include<unistd.h>
#include"general.h"
using namespace std;

default_random_engine generator;
unsigned f_objetivo_evals = 0;

//El tamaño de solucion debe ser mayor o igual a sub_size
bool any_empty_cluster(const int n_clust, const unsigned* solucion, const int sub_size){
    unsigned cardinal_cluster[n_clust] = {0};
    for(int i=0; i<sub_size; i++){
        cardinal_cluster[solucion[i]]++;
    }
    for(int k=0; k<n_clust; k++){
        if(cardinal_cluster[k] == 0){
            return true;
        }
    }
    return false;
}

void generar_solucion_aleatoria_valida(const int n_clust, unsigned* solucion, const int sub_size){
    bool valida = false;
    do{

        valida = true;
        unsigned cardinal_cluster[n_clust] = {0};

        for(int i=0; i<sub_size; i++){
            solucion[i] = random_int(0, INT_MAX)%n_clust;
            cardinal_cluster[solucion[i]]++;
        }

        int k=0;
        while(valida && k<n_clust){
            if(cardinal_cluster[k] == 0){
                valida = false;
            }
            k++;
        }
    
    }while( !valida );

}

//p1 y p2 deben ser arrays de tamaño 3
float distancia(const float* p_1, const float* p_2, const unsigned dim){
    float sum = 0.0f;
    for(int i=0; i<dim; i++){
        sum += p_1[i]*p_1[i]+p_2[i]*p_2[i]-2*p_1[i]*p_2[i];
    }
    return sqrt(sum);
}

float media(const float* array, const int array_size){
    float sum = 0.0;
    for(int i=0; i<array_size; i++){
        sum += array[i];
    }
    return sum/array_size;
}

void sumar(const float* v_1, const float* v_2, float* resultado, unsigned dim){
    for(int i=0; i<dim; i++){
        resultado[i] = v_1[i]+v_2[i];
    }
}

vector<float> maximo_por_columnas(vector<vector<float> >& matrix, 
                                  const unsigned size, const unsigned dim){
    vector<float> vec_max;
    for(int j=0; j<dim; j++){
        float maximo = -FLT_MAX;
        for(int i=0; i<size; i++){
            maximo = max(maximo, matrix[i][j]);
        }
        vec_max.push_back(maximo);
    }
    return vec_max;
}

vector<float> minimo_por_columnas(vector<vector<float> >& matrix, 
                                  const unsigned size, const unsigned dim){
    vector<float> vec_min;
    for(int j=0; j<dim; j++){
        float minimo = FLT_MAX;
        for(int i=0; i<size; i++){
            minimo = min(minimo, matrix[i][j]);
        }
        vec_min.push_back(minimo);
    }
    return vec_min;
}



float desviacion_media(const unsigned* solucion, const vector<vector<float> >& dat,
                       const float* const* centroides, const unsigned n_clust,
                       const unsigned sub_size, const unsigned dim){
    float desviaciones[n_clust] = {0};
    unsigned cardinal_cluster[n_clust] = {0};
    for(int i=0; i<sub_size; i++){
        int k = solucion[i];
        desviaciones[k] += distancia(centroides[k], dat[i].data(), dim);
        cardinal_cluster[k]++;
    }
        
    for(int k=0; k<n_clust; k++){
        desviaciones[k] /= cardinal_cluster[k];
    }
    return media(desviaciones, n_clust);
}

unsigned infeasibility_total(const vector<vector<int> > &lista_rest, const unsigned* solucion){
    unsigned infeasibility = 0;
    //Recorremos la lista de restricciones
    for(vector<vector<int> >::const_iterator r=lista_rest.begin(); r!=lista_rest.end(); r++){
        //Si se incumple alguna restricción aumentamos la infeasibility
        if( (solucion[(*r)[0]]!=solucion[(*r)[1]] && (*r)[2]==1) 
            || (solucion[(*r)[0]]==solucion[(*r)[1]] && (*r)[2]==-1)){
                infeasibility += 1;
            }        
    }        
    return infeasibility;
}

bool misma_solucion(unsigned* sol1, unsigned* sol2, unsigned size){
    for(int i=0; i<size; i++){
        if(sol1[i]!=sol2[i])    return false;
    }
    return true;
}

void recalcular_centroides(float** centroides, const int n_clust, 
                           const unsigned* solucion, const unsigned size, 
                           const vector<vector<float> >& dat, const unsigned dim){
    for(int k=0; k<n_clust; k++){
        fill(centroides[k], centroides[k]+dim, 0);
    }
    
    unsigned cardinal_cluster[n_clust] = {0};
    for(int i=0; i<size; i++){
        sumar(centroides[solucion[i]], dat[i].data(), centroides[solucion[i]], dim);
        cardinal_cluster[solucion[i]]++;
    }
    
    for(int k=0; k<n_clust; k++){
        for(int j=0; j<dim; j++){
            centroides[k][j] /= cardinal_cluster[k];
        }
    }

}

void read_matrix_from_txt(const string& path, vector<vector<float> >& matrix, unsigned &dim_1, unsigned &dim_2){

    ifstream file(path);
    string line;

    int i=0, j=0;
    while( getline(file,line) ){
        j = 0;
        vector<float> fila;
        do{
            int coma_pos = line.find(",");
            string element = line.substr(0, coma_pos);
            fila.push_back(stof(element));
            if(coma_pos==string::npos){
                line.clear();
            }else{
                line = line.substr(coma_pos+1);
            } 
            j++;
        }while(!line.empty());
        matrix.push_back(fila);
        i++;
    }

    file.close();

    dim_1 = i;
    dim_2 = j;

}