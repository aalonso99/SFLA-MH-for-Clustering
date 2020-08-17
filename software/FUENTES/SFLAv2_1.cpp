#include<iostream>
#include<cmath>
#include<cstring>
#include<vector>
#include<utility>
#include<set>
#include<iterator>  //std::advance
#include<chrono>
#include<cfloat>
#include<climits>
#include<unistd.h>
#include"general.h"
#include"SFLA.h"
using namespace std;

void sfla_searchv2_1(const string& dataset, 
                  const string& constraints, 
                  const int seed, const unsigned n_clust, const unsigned sub_size){

    const unsigned N_REINICIOS = 5;
    const unsigned N_MEMEPLEXES = 5;
    const unsigned FROGS_PER_MEMEPLEX = 30;
    const unsigned FROGS_PER_SUBMEMEPLEX = 15;
    const unsigned ITERS_IN_MEMEPLEX = 30;

    auto start = chrono::high_resolution_clock::now();

    unsigned size = 0;
    unsigned dim = 0;
    const unsigned N_FROGS = N_MEMEPLEXES*FROGS_PER_MEMEPLEX;
    
    vector<vector<float> > dat, c_matrix;
    read_matrix_from_txt( dataset, dat, size, dim );
    read_matrix_from_txt( constraints, c_matrix, size, size );

    if( sub_size>0 && sub_size<size )   size = sub_size;

    vector<vector<unsigned> > frogs;
    for(int i=0; i<N_FROGS; i++){
        frogs.push_back( vector<unsigned> (size) );
    }

    vector<vector<int> > lista_rest;
    for(int i=0; i<size; i++){
        for(int j=i+1; j<size; j++){
            if(c_matrix[i][j] != 0){
                vector<int> rest;
                rest.push_back(i); rest.push_back(j); rest.push_back(c_matrix[i][j]);
                lista_rest.push_back(rest);
            }
        }
    }
    
    //Calculamos el factor que gradúa la infeasibility: lambda=Distancia_máxima/Número_Restricciones
    float L = 0;
    for( vector<vector<float> >::const_iterator i = dat.begin(); i!=dat.end(); i++ ){
        for( vector<vector<float> >::const_iterator j = dat.begin(); j!=dat.end(); j++ ){
            L = max(L, distancia((*i).data(), (*j).data(), dim));
        }
    }
        
    L = ceil(L)/lista_rest.size();

    //Generación de centroides iniciales
    vector<float> vec_max = maximo_por_columnas(dat, size, dim);
    vector<float> vec_min = minimo_por_columnas(dat, size, dim);

    float*** centroides = new float** [N_FROGS];
    for(int k=0; k<N_FROGS; k++){
        centroides[k] = new float* [n_clust];
        init_rand(seed);
        for(int i=0; i<n_clust; i++){
            centroides[k][i] = new float [dim];
            for(int j=0; j<dim; j++){
                centroides[k][i][j] = random_float(0,1)*(vec_max[j]-vec_min[j])+vec_min[j];
            }
        }
    }

    //Selección inicial del cluster asignado a cada elemento de cada rana (se toma el centroide más cercano)
    for(int j=0; j<N_FROGS; j++){
        for(int i=0; i<size; i++){
            float dist = FLT_MAX;
            int mejor = 0;
            for(int k=0; k<n_clust; k++){
                float dist_cluster_k = distancia( dat[i].data(), centroides[j][k], dim );
                if( dist_cluster_k < dist ){
                    mejor = k;
                    dist = dist_cluster_k;
                } 
            } 
                
            frogs[j][i] = mejor;
        }
    }

    struct compare_frogs{
        bool operator()(const pair<unsigned, float> &frog1, const pair<unsigned, float> &frog2) const {
            if(frog1.second == frog2.second){
                return frog1.first < frog2.first ;
            }else{
                return frog1.second < frog2.second ;
            }
        }
    };

    //Conjunto con el índice y los costes de cada rana (se ordena de menor a mayor automáticamente)
    set<pair<unsigned,float>, compare_frogs> loss;

    for(int j=0; j<N_FROGS; j++){
        if( any_empty_cluster(n_clust, frogs[j].data(), size) ){
            generar_solucion_aleatoria_valida(n_clust, frogs[j].data(), size);
        }
        recalcular_centroides(centroides[j], n_clust, frogs[j].data(), size, dat, dim);
        //Insertamos la pareja indice-coste en el conjunto de costes
        loss.insert(
            pair<unsigned, float> ( j, f_objetivo(frogs[j].data(), dat, centroides[j], n_clust, size, dim, L, lista_rest) ) 
        );
    }

    float min_loss = loss.begin()->second;
    unsigned min_loss_idx = loss.begin()->first;
    vector<set<pair<unsigned,float>, compare_frogs> > memeplexes;
    f_objetivo_evals = 0;
    //Bucle principal
    for(unsigned rep=0; rep<N_REINICIOS; rep++){

        do{
            memeplexes.clear();
            for(unsigned i=0; i<N_MEMEPLEXES; i++){
                memeplexes.push_back( set<pair<unsigned,float>, compare_frogs> () );
            }
            //Repartir las frogs en memeplexes    
            unsigned cont=0;
            for(auto it=loss.begin(); it!=loss.end(); it++, cont++){
                memeplexes[cont%N_MEMEPLEXES].insert(*it);
            }
            
            ///////////// Evolución memética
            float f_m = FROGS_PER_MEMEPLEX;     //Para mejor legibilidad
            set<pair<unsigned,float>, compare_frogs> submemeplex;
            for(unsigned i=0; i<N_MEMEPLEXES; i++){     //Dentro de cada memeplex...
                for(unsigned j=0; j<ITERS_IN_MEMEPLEX; j++){     //Para cada iteración en cada memeplex...
                    //Construimos un submemeplex y lo llenamos con frogs, favoreciendo las mejores
                    submemeplex.clear();
                    while( submemeplex.size()<FROGS_PER_SUBMEMEPLEX ){  
                        unsigned k=0;   
                        for(auto it=memeplexes[i].begin(); it!=memeplexes[i].end(); k++, it++){
                            if( random_float(0,1) < 2*(f_m-k)/(f_m*(f_m+1)) ){
                                submemeplex.insert(*it);
                            }
                        }
                    }

                    //Modificamos la peor rana del submemeplex, aproximándola a la mejor.
                    //(Por si acaso el resultado no es factible, guardamos la modificación en otro vector (rana_modificada))
                    unsigned peor_idx = submemeplex.rbegin()->first;
                    unsigned mejor_idx = submemeplex.begin()->first;
                    vector<unsigned> rana_modificada;
                    for(unsigned k=0; k<size; k++){
                        if( frogs[peor_idx][k]<frogs[mejor_idx][k] ){
                            rana_modificada.push_back( random_int(frogs[peor_idx][k], frogs[mejor_idx][k]) );
                        }else{
                            rana_modificada.push_back( random_int(frogs[mejor_idx][k], frogs[peor_idx][k]) );
                        }
                    }

                    //Evaluamos la rana modificada
                    bool mejor = false;
                    float loss_rana_modificada;
                    if( !any_empty_cluster(n_clust, rana_modificada.data(), size) ){
                        //La peor rana será siempre sustituida, así que podemos sobreescribir su centroide
                        recalcular_centroides(centroides[peor_idx], n_clust, rana_modificada.data(), size, dat, dim);
                        loss_rana_modificada = f_objetivo(rana_modificada.data(), dat, 
                                                                centroides[peor_idx], n_clust, size, dim, L, lista_rest);
                        //(Si la evaluación realizada es mejor que la de la peor rana en el submemeplex...)
                        if( loss_rana_modificada < submemeplex.rbegin()->second ){
                            mejor = true;
                            frogs[peor_idx] = rana_modificada;      
                        }
                    }

                    if( !mejor ){
                        for(unsigned k=0; k<size; k++){
                            if( frogs[peor_idx][k]<frogs[min_loss_idx][k] ){
                                rana_modificada[k] = random_int( frogs[peor_idx][k], frogs[min_loss_idx][k] );
                            }else{
                                rana_modificada[k] = random_int( frogs[min_loss_idx][k], frogs[peor_idx][k] );
                            }
                        }

                        if( !any_empty_cluster(n_clust, rana_modificada.data(), size) ){
                            //La peor rana será siempre sustituida, así que podemos sobreescribir su centroide
                            recalcular_centroides(centroides[peor_idx], n_clust, rana_modificada.data(), size, dat, dim);
                            loss_rana_modificada = f_objetivo(rana_modificada.data(), dat, centroides[peor_idx], n_clust,
                                                                    size, dim, L, lista_rest);
                            //(Si la evaluación realizada es mejor que la de la peor rana en el submemeplex...)
                            if( loss_rana_modificada < submemeplex.rbegin()->second ){
                                mejor = true;
                                frogs[peor_idx] = rana_modificada;      
                            }

                        }
                    }
                    
                    if( !mejor ){
                        generar_solucion_aleatoria_valida(n_clust, frogs[peor_idx].data(), size);

                        //La peor rana será siempre sustituida, así que podemos sobreescribir su centroide
                        recalcular_centroides(centroides[peor_idx], n_clust, frogs[peor_idx].data(), size, dat, dim);
                        loss_rana_modificada = f_objetivo(frogs[peor_idx].data(), dat, centroides[peor_idx], 
                                                        n_clust, size, dim, L, lista_rest);
                        //(Si la evaluación realizada es mejor que la de la peor rana en el submemeplex...)
                        if( loss_rana_modificada < submemeplex.rbegin()->second ){
                            mejor = true;
                        }
                    }

                    //Eliminamos en el memeplex la peor rana de este submemeplex y añadimos su nueva versión
                    memeplexes[i].erase( *(submemeplex.rbegin()) );
                    //Como el set ordena internamente sus elementos, asumo que esto debe modificar el orden en el memeplex
                    memeplexes[i].insert( pair<unsigned, float> (submemeplex.rbegin()->first, loss_rana_modificada) );  
                    if( loss_rana_modificada < min_loss ){
                        min_loss = loss_rana_modificada;
                        min_loss_idx = peor_idx;
                    }
                }
            }
            ////////////////////////////////

            //Mezclar todas las ranas
            loss.clear();
            for(unsigned j=0; j<N_MEMEPLEXES; j++){
                loss.insert(memeplexes[j].begin(), memeplexes[j].end());
            }

        }while( f_objetivo_evals < (rep+1)*MAX_EVALS/N_REINICIOS );

        if( rep<(N_REINICIOS-1) ){
            auto loss_it = loss.begin();
            advance(loss_it, 2*(rep+1));
            set<pair<unsigned,float>, compare_frogs> nuevo_loss(loss.begin(), loss_it);
            //Generar población virtual y ordenarla en función de su función objetivo
            while( loss_it!=loss.end() ){
                unsigned idx = loss_it->first;
                generar_solucion_aleatoria_valida(n_clust, frogs[idx].data(), size);
                recalcular_centroides(centroides[idx], n_clust, frogs[idx].data(), size, dat, dim);
                //Insertamos la pareja indice-coste en el nuevo  conjunto de costes
                //loss_it = loss.erase(loss_it);
                nuevo_loss.insert(
                    pair<unsigned, float> ( idx, f_objetivo(frogs[idx].data(), dat, 
                                            centroides[idx], n_clust, size, dim, L, lista_rest) ) 
                );
                loss_it++;
            }

            loss = nuevo_loss;
        }
        
    }

    cout<<"Evaluaciones de la funcion objetivo: "<<f_objetivo_evals<<endl;
    min_loss_idx = loss.begin()->first;
    float infeas = infeasibility_total( lista_rest, frogs[min_loss_idx].data() );
    recalcular_centroides(centroides[min_loss_idx], n_clust, frogs[min_loss_idx].data(), size, dat, dim);
    float d_media = desviacion_media(frogs[min_loss_idx].data(), dat, centroides[min_loss_idx], n_clust, size, dim);
    cout<<"Infeasibility: "<<infeas<<endl;
    cout<<"Desviacion media: "<<d_media<<endl;
    cout<<"Función objetivo: "<<d_media+L*infeas<<endl;
    //cout<<"Lambda: "<<L<<endl;

    //Liberación de memoria
    for(int j=0; j<N_FROGS; j++){
        for(int i=0; i<n_clust; i++){
            delete [] centroides[j][i];
        }
        delete [] centroides[j];
    }
    delete [] centroides;

    auto end = chrono::high_resolution_clock::now();
    cout << "Tiempo total: " 
		<< chrono::duration_cast<chrono::milliseconds>(end - start).count()/1000.0f
		<< " segundos"<<endl;

}
