cmake_minimum_required (VERSION 3.8)

add_subdirectory(base)

target_sources(
	Bayesian-Network
	PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/VariableNodeMap.h"
	"${CMAKE_CURRENT_LIST_DIR}/VariableNodeMap.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ThreadPoolManager.h"
	"${CMAKE_CURRENT_LIST_DIR}/ThreadPoolManager.cpp"
)

target_include_directories(Bayesian-Network PRIVATE ${CMAKE_CURRENT_LIST_DIR})