cmake_minimum_required(VERSION 3.19)
project(our)

set(CMAKE_CXX_STANDARD 20)

add_executable(our main.cpp graph/GraphElements.cpp graph/GraphElements.h graph/Graph.cpp graph/Graph.h probability/CPT.cpp probability/CPT.h network/reader.cpp network/reader.h network/BayesianNetwork.cpp network/BayesianNetwork.h inference/VariableElimination.cpp inference/VariableElimination.h)