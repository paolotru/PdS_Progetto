//
// Created by LucaS on 09/08/2021.
//

#include "VariableElimination.h"

void VariableElimination::inferVariableProbability(Graph& g){

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

    Graph output{};

    for(Node& n: nodes){
        if(!n.getCPT()->hasDependence) {
            output.addNode(n);
            /* si può migliorare facendo che graph abbia ptr a nodi e archi, in questo modo il ptr
            può essere condiviso e occupare meno memoria*/
            continue;
        }

        std::vector<Status> statuses = n.getStatuses();
        for(auto& status: statuses){
            probability = computeStatusProbability(g, n, status, factors);
            //aggiungi alla cpt con status=status e prob=probability senza parents

        }
    }
}

float VariableElimination::computeStatusProbability(Graph &g, Node &node, Status &s,
                                                    std::map <Node, std::vector<Node>> &factors) {
    auto nodes = g.getNodes();
    std::vector<Node> rightNodes;

    for(auto& n:nodes) {
        if(n !== node)
            rightNode.push_back(n);
    }
    std::vector<Node, Status> variables;
    float prob = 0;
    recursiveFunction(rightNodes, node, s, factors, variables, &prob);
    return prob;
}

void recursiveFunction(std::vector<Node>& nodes, Node node, Status s, std::map <Node, std::vector<Node>> &factors, std::map<Node, Status> variables, float* p){

    if(nodes.empty()){
        *p += computeProbability(factors, s, variables, node);
        return;
    }

    Node n = nodes.erase();
    nodes.pop_back();
    for(auto& status: n.getStatuses()){
        variables.insert(n,status);
        recursiveFunction(nodes, node, s, factors, variables);
        variables.erase(n);
    }
};

float computeProbability(std::map <Node, std::vector<Node>> &factors, Status s, std::map<Node, Status> variables, Node node){
    float result = 1;
    for(auto& f : factors){
        std::vector<ConditionalProbability> cpt = f.first.cpt->getCPTTable();
        for(auto& c : cpt){
            if(checkParentVectors(c,variables)) // da implementare funzione
                result *= c.getProbability();
        }
    }
    return result;
}