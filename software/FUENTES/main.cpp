#include<iostream>
#include<functional>
#include<stdlib.h>
#include<string>
#include"SFLA.h"
using namespace std;

int main(int narg, char** arg){

    unsigned n_alg = 4;
    function<void(const string&, const string&, const int, const unsigned, 
                  const unsigned)> algorithms[] = { sfla_searchv1, sfla_searchv2_1, sfla_searchv2_2, sfla_searchv2_2parallel };
    string alg_names[] = { "SFLAv1", "SFLAv2_1", "SFLAv2_2", "SFLAv2_2parallel" };
    unsigned n_seeds = 5;
    int seeds [] = {31,41,71,109,163};
    unsigned n_data = 4;
    string datasets [] = {"iris_set.dat", "ecoli_set.dat", "newthyroid_set.dat", "rand_set.dat"};
    int n_clust[] = {3,8,3,3};
    string constraints10 [] = {"iris_set_const_10.const", "ecoli_set_const_10.const", 
                                "newthyroid_set_const_10.const", "rand_set_const_10.const"};
    string constraints20 [] = {"iris_set_const_20.const", "ecoli_set_const_20.const", 
                                 "newthyroid_set_const_20.const", "rand_set_const_20.const"};                       
    string path = "./nuevos conjuntos de datos PAR 2019-20/";

    if( narg==1 ){

        cout<<"Escriba \"main all\" para ejecutar todos los algoritmos para todos los datasets, semillas y conjuntos de restricciones."<<endl;
        cout<<"Si se desea ejecutar un algoritmo en particular, repita el comando con los siguientes argumentos:"<<endl<<endl;
        cout<<"main [INDICE_ALGORITMO] [DATASET] [RESTRICCIONES] [SEMILLA] [N_CLUSTERS]"<<endl<<endl;
        cout<<"El indice correspondiente a cada algoritmo es:"<<endl;
        cout<<"\t0 -> SFLAv1\n\t1 -> SFLAv2_1\n\t2 -> SFLAv2_2\n\t3 -> SFLAv2_2parallel"<<endl<<endl;

    }else if( narg==2 && strcmp(arg[1],"all")==0 ){

        for(int i=0; i<n_alg; i++){
            cout<<alg_names[i]<<endl;
            for(int k=0; k<n_data; k++){
                cout<<"Dataset: "<<datasets[k]<<endl;
                for(int j=0; j<n_seeds; j++){
                    cout<<"Semilla: "<<seeds[j]<<endl;
                    cout<<"Constraints: "<<constraints10[k]<<endl;
                    algorithms[i](path+datasets[k], path+constraints10[k], seeds[j], n_clust[k], NULL);
                    cout<<endl;
                }
            }
        }

        for(int i=0; i<n_alg; i++){
            cout<<alg_names[i]<<endl;
            for(int k=0; k<n_data; k++){
                cout<<"Dataset: "<<datasets[k]<<endl;
                for(int j=0; j<n_seeds; j++){
                    cout<<"Semilla: "<<seeds[j]<<endl;
                    cout<<"Constraints: "<<constraints20[k]<<endl;
                    algorithms[i](path+datasets[k], path+constraints20[k], seeds[j], n_clust[k], NULL);
                    cout<<endl;
                }
            }
        }
    }else if( narg==6 ){
        algorithms[atoi(arg[1])](arg[2], arg[3], seeds[atoi(arg[4])], atoi(arg[5]), NULL);
    }

    return 0;
}