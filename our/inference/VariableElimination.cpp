//
// Created by LucaS on 09/08/2021.
//
#include "VariableElimination.h"

Graph VariableElimination::inferVariableProbability(Graph& g){

    std::vector<Arc> arcs = g.getArcs();
    std::vector<Node> nodes = g.getNodes();

    std::map<Node, std::vector<Node>> factors;

    for(auto& a : arcs){
        Node src = a.getSource();
        Node dest = a.getDestination();

        auto nodeAlreadyPresent = factors.find(dest);
        if(nodeAlreadyPresent != factors.end()){
            nodeAlreadyPresent->second.push_back(src);
        }
        else{
            std::vector<Node> toInsert;
            toInsert.push_back(src);
            factors.insert({dest, toInsert});
        }
    }

    Graph output;

    for(Node& n: nodes){
        if(!n.getCpt()->isHasDependence()) {
            output.addNode(n);
            /* si può migliorare facendo che graph abbia ptr a nodi e archi, in questo modo il ptr
            può essere condiviso e occupare meno memoria*/
            continue;
        }

        std::vector<Status> statuses = n.getStatuses();
        for(auto& status: statuses){
            float probability = computeStatusProbability(g, n, status, factors);
            auto vi = std::make_shared<VariableInformations>(std::map<NodeId, Status>(), status);
            ConditionalProbability cp(vi,probability);

            n.getCpt()->addProbability(cp);

        }
    }

    return output;
}

float VariableElimination::computeStatusProbability(Graph &g, Node &node, Status &s,
                                                    std::map <Node, std::vector<Node>> &factors) {
    auto nodes = g.getNodes();
    std::vector<Node> rightNodes;

    for(auto& n:nodes) {
        if(n != node)
            rightNodes.push_back(n);
    }
    std::map<Node, Status> variables;
    float prob = 0;
    recursiveFunction(rightNodes, node, s, factors, variables, &prob);
    return prob;
}

void VariableElimination::recursiveFunction(std::vector<Node>& nodes, Node node, Status s, std::map <Node, std::vector<Node>> &factors, std::map<Node, Status> variables, float* p){

    if(nodes.empty()){
        *p += computeProbability(factors, s, variables, node);
        return;
    }

    auto nIt = nodes.erase(nodes.begin());
    nodes.pop_back();
    for(auto& status: nIt->getStatuses()){
        variables.insert({*nIt,status});
        recursiveFunction(nodes, node, s, factors, variables, p);
        variables.erase(*nIt);
    }
};

float VariableElimination::computeProbability(std::map <Node, std::vector<Node>> &factors, Status s, std::map<Node, Status> variables, Node node){
    float result = 1;

    std::map<NodeId, Status> toBeChecked;
    for(auto& it : variables){
        toBeChecked.insert(std::pair<NodeId, Status>(it.first.getId(),it.second));
    }
    for(auto& f : factors){
        std::vector<ConditionalProbability> cpt = f.first.getCpt()->getCPTTable();
        for(auto& c : cpt){
            if(c.checkParentVectors(toBeChecked, s))
                result *= c.getProbability();
        }
    }
    return result;
}