//
// Created by S290225 on 12/08/2021.
//

#ifndef OUR_VARIABLEELIMINATION_H
#define OUR_VARIABLEELIMINATION_H
#include "../graph/Graph.h"
#include <memory>

class VariableElimination {
    static float computeStatusProbability(std::shared_ptr<Graph> g, Node& n,Status& s,std::map<Node, std::vector<Node>>& factors);
    static void recursiveFunction(std::vector<Node> nodes, Node& node, Status s, std::map <Node, std::vector<Node>> &factors, std::map<Node, Status> variables, float* p);
    static float computeProbability(std::map <Node, std::vector<Node>> &factors, Status s, std::map<Node, Status> variables, Node node);
public:
    static Graph inferVariableProbability(std::shared_ptr<Graph> g);
};


#endif //OUR_VARIABLEELIMINATION_H