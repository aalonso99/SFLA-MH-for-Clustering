
RUTA = "../FUENTES/"
FLAGS = -std=c++17 -fopenmp -O3
FUENTES = $(RUTA)general.cpp $(RUTA)SFLAv1.cpp $(RUTA)SFLAv2_1.cpp $(RUTA)SFLAv2_2.cpp $(RUTA)SFLAv2_2parallel.cpp $(RUTA)main.cpp

all: main

main:
	g++ $(FLAGS) $(FUENTES) -o main
	
clean: 
	rm ./main
