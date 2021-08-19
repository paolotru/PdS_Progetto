//
// Created by S290225 on 12/08/2021.
//

#ifndef OUR_VARIABLEELIMINATION_H
#define OUR_VARIABLEELIMINATION_H
#include "../graph/Graph.h"
#include "../graph/GraphElements.h"
#include <memory>

class VariableElimination {
    float computeStatusProbability(Graph& g, Node& n,Status& s,std::map<Node, std::vector<Node>>& factors);
public:
    static void inferVariableProbability(Graph& g);
};


#endif //OUR_VARIABLEELIMINATION_H
