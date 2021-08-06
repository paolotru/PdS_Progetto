cmake_minimum_required (VERSION 3.8)

target_sources(
	Bayesian-Network
	PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/ArcSet.h" 
	"${CMAKE_CURRENT_LIST_DIR}/ArcSet.cpp" 
	"${CMAKE_CURRENT_LIST_DIR}/Clique.h" 
	"${CMAKE_CURRENT_LIST_DIR}/Clique.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/EdgeSet.h"
	"${CMAKE_CURRENT_LIST_DIR}/EdgeSet.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/GraphElements.h" 
	"${CMAKE_CURRENT_LIST_DIR}/GraphElements.cpp"
)

target_include_directories(Bayesian-Network PRIVATE ${CMAKE_CURRENT_LIST_DIR})