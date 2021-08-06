cmake_minimum_required (VERSION 3.8)

add_subdirectory(base)

target_sources(
	Bayesian-Network
	PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/BNReader.h"
	"${CMAKE_CURRENT_LIST_DIR}/BNReader.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/BNWriter.h"
	"${CMAKE_CURRENT_LIST_DIR}/BNWriter.cpp"
)

target_include_directories(Bayesian-Network PRIVATE ${CMAKE_CURRENT_LIST_DIR})