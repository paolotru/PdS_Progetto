cmake_minimum_required (VERSION 3.8)

add_subdirectory(graphStructure)

target_sources(
	Bayesian-Network
	PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/DAG.h"
	"${CMAKE_CURRENT_LIST_DIR}/DAG.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/JunctionTree.h"
	"${CMAKE_CURRENT_LIST_DIR}/JunctionTree.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/UndirectedGraph.h"
	"${CMAKE_CURRENT_LIST_DIR}/UndirectedGraph.cpp"
)

target_include_directories(Bayesian-Network PRIVATE ${CMAKE_CURRENT_LIST_DIR})