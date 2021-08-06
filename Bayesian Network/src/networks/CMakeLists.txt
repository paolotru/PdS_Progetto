cmake_minimum_required (VERSION 3.8)

add_subdirectory(io)

target_sources(
	Bayesian-Network
	PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/BayesianNet.h"
	"${CMAKE_CURRENT_LIST_DIR}/BayesianNet.cpp"
)

target_include_directories(Bayesian-Network PRIVATE ${CMAKE_CURRENT_LIST_DIR})