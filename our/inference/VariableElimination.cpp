//
// Created by LucaS on 09/08/2021.
//

#include "VariableElimination.h"

void VariableElimination::inferVariableProbability(Graph &g){

    std::vector<Arc> arcs = g.getArcs();
    std::vector<Node> nodes = g.getNodes();

    std::map<Node, std::vector<Node>> factors;

    for(auto& a : arcs){
        auto src = a.getSource();
        auto dest = a.getDestination();

        if(factors.find(dest) != factors.end()){
            factors.find(dest)->second.push_back(src);
        }
        else{
            factors.insert(dest, new std::vector<Node>(src);
        }
    }

    for(Node& n: nodes){
        if(!n.getCPT()->hasDependence)
            continue;
    }

};