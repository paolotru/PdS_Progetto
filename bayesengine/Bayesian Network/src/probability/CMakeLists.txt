cmake_minimum_required (VERSION 3.8)

target_sources(
	Bayesian-Network
	PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/CPT.h"
	"${CMAKE_CURRENT_LIST_DIR}/Potential.h"
)

target_include_directories(Bayesian-Network PRIVATE ${CMAKE_CURRENT_LIST_DIR})