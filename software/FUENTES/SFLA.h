#ifndef SFLA_H
#define SFLA_H

#include<cstring>
#include<string>
using namespace std;

void sfla_searchv1(const string& dataset, 
                 const string& constraints, 
                 const int seed, const unsigned n_clust, const unsigned sub_size);

void sfla_searchv2_1(const string& dataset, 
                 const string& constraints, 
                 const int seed, const unsigned n_clust, const unsigned sub_size);

void sfla_searchv2_2(const string& dataset, 
                 const string& constraints, 
                 const int seed, const unsigned n_clust, const unsigned sub_size);

void sfla_searchv2_2parallel(const string& dataset, 
                 const string& constraints, 
                 const int seed, const unsigned n_clust, const unsigned sub_size);

#endif