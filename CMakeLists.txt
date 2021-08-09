cmake_minimum_required(VERSION 3.19)
project(PdS_Progetto)

set(CMAKE_CXX_STANDARD 14)

include_directories("bayesengine/Bayesian Network/src/graphs")
include_directories("bayesengine/Bayesian Network/src/graphs/graphStructure")
include_directories("bayesengine/Bayesian Network/src/inference")
include_directories("bayesengine/Bayesian Network/src/learning")
include_directories("bayesengine/Bayesian Network/src/learning/io")
include_directories("bayesengine/Bayesian Network/src/learning/io/base")
include_directories("bayesengine/Bayesian Network/src/networks")
include_directories("bayesengine/Bayesian Network/src/networks/io")
include_directories("bayesengine/Bayesian Network/src/networks/io/base")
include_directories("bayesengine/Bayesian Network/src/probability")
include_directories("bayesengine/Bayesian Network/src/tools")
include_directories("bayesengine/Bayesian Network/src/tools/base")
include_directories(our/graph)
include_directories(our/probability)

add_executable(PdS_Progetto
        "bayesengine/Bayesian Network/src/graphs/graphStructure/ArcSet.cpp"
        "bayesengine/Bayesian Network/src/graphs/graphStructure/ArcSet.h"
        "bayesengine/Bayesian Network/src/graphs/graphStructure/Clique.cpp"
        "bayesengine/Bayesian Network/src/graphs/graphStructure/Clique.h"
        "bayesengine/Bayesian Network/src/graphs/graphStructure/EdgeSet.cpp"
        "bayesengine/Bayesian Network/src/graphs/graphStructure/EdgeSet.h"
        "bayesengine/Bayesian Network/src/graphs/graphStructure/GraphElements.cpp"
        "bayesengine/Bayesian Network/src/graphs/graphStructure/GraphElements.h"
        "bayesengine/Bayesian Network/src/graphs/DAG.cpp"
        "bayesengine/Bayesian Network/src/graphs/DAG.h"
        "bayesengine/Bayesian Network/src/graphs/JunctionTree.cpp"
        "bayesengine/Bayesian Network/src/graphs/JunctionTree.h"
        "bayesengine/Bayesian Network/src/graphs/UndirectedGraph.cpp"
        "bayesengine/Bayesian Network/src/graphs/UndirectedGraph.h"
        "bayesengine/Bayesian Network/src/inference/LazyPropagation.cpp"
        "bayesengine/Bayesian Network/src/inference/LazyPropagation.h"
        "bayesengine/Bayesian Network/src/learning/io/base/rapidcsv.h"
        "bayesengine/Bayesian Network/src/learning/io/CSVDataStructure.h"
        "bayesengine/Bayesian Network/src/learning/io/CSVReader.h"
        "bayesengine/Bayesian Network/src/networks/io/base/rapidxml.hpp"
        "bayesengine/Bayesian Network/src/networks/io/base/rapidxml_iterators.hpp"
        "bayesengine/Bayesian Network/src/networks/io/base/rapidxml_print.hpp"
        "bayesengine/Bayesian Network/src/networks/io/base/rapidxml_utils.hpp"
        "bayesengine/Bayesian Network/src/networks/io/BNReader.cpp"
        "bayesengine/Bayesian Network/src/networks/io/BNReader.h"
        "bayesengine/Bayesian Network/src/networks/io/BNWriter.cpp"
        "bayesengine/Bayesian Network/src/networks/io/BNWriter.h"
        "bayesengine/Bayesian Network/src/networks/BayesianNet.cpp"
        "bayesengine/Bayesian Network/src/networks/BayesianNet.h"
        "bayesengine/Bayesian Network/src/probability/CPT.h"
        "bayesengine/Bayesian Network/src/probability/Potential.h"
        "bayesengine/Bayesian Network/src/tools/base/COW.cpp"
        "bayesengine/Bayesian Network/src/tools/base/COW.h"
        "bayesengine/Bayesian Network/src/tools/base/thread_pool.hpp"
        "bayesengine/Bayesian Network/src/tools/ThreadPoolManager.cpp"
        "bayesengine/Bayesian Network/src/tools/ThreadPoolManager.h"
        "bayesengine/Bayesian Network/src/tools/VariableNodeMap.cpp"
        "bayesengine/Bayesian Network/src/tools/VariableNodeMap.h"
        "bayesengine/Bayesian Network/main.cpp"
        our/graph/Graph.cpp
        our/graph/Graph.h
        our/graph/GraphElements.cpp
        our/graph/GraphElements.h
        our/probability/CPT.cpp
        our/probability/CPT.h
        our/main.cpp)
