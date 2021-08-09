# CMakeList.txt: progetto CMake per Bayesian Network. Includere l'origine e definire
# qui la logica specifica del progetto.
#
cmake_minimum_required (VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)

add_library(
	Bayesian-Network
	"src/networks/BayesianNet.h"
	"src/networks/BayesianNet.cpp"
)

add_subdirectory(src)

# Aggiungere l'origine all'eseguibile di questo progetto.
add_executable (BNTarget "main.cpp")

target_link_libraries(BNTarget PRIVATE Bayesian-Network)

#target_compile_features(BNTarget PUBLIC cxx_std_17)
#set_property(TARGET BNTarget PROPERTY CXX_STANDARD 17)
#set_property(TARGET BNTarget PROPERTY CXX_STANDARD_REQUIRED ON)

#target_link_libraries(CMakeTarget PRIVATE tools)

# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
