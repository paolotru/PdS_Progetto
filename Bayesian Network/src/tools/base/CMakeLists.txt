cmake_minimum_required (VERSION 3.8)

target_sources(
	Bayesian-Network
	PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/COW.h"
	"${CMAKE_CURRENT_LIST_DIR}/COW.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/thread_pool.hpp"
)

target_include_directories(Bayesian-Network PRIVATE ${CMAKE_CURRENT_LIST_DIR})