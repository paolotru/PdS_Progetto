cmake_minimum_required (VERSION 3.8)

target_sources(
	Bayesian-Network
	PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/rapidxml.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/rapidxml_iterators.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/rapidxml_print.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/rapidxml_utils.hpp"
)

target_include_directories(Bayesian-Network PRIVATE ${CMAKE_CURRENT_LIST_DIR})