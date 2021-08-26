//
// Created by LucaS on 09/08/2021.
//
#include "VariableElimination.h"

Graph VariableElimination::inferVariableProbability(std::shared_ptr<Graph> g){

    std::vector<Arc> arcs = g->getArcs();
    std::vector<Node> nodes = g->getNodes();

    std::map<Node, std::vector<Node>> factors;

    for(auto& n : nodes){
        if(!n.getCpt()->isHasDependence()){
            std::vector<Node> toInsert;
            factors.insert({n, toInsert});
        }

    }
    for(auto& a : arcs){
        Node src = a.getSource();
        Node dest = a.getDestination();

        auto nodeAlreadyPresent = factors.find(dest);
        if(nodeAlreadyPresent != factors.end()){
            nodeAlreadyPresent->second.push_back(src);
            std::cout << "push in vettore già presente nodo " << dest.getName() << std::endl;
        }
        else{
            std::vector<Node> toInsert;
            toInsert.push_back(src);
            factors.insert({dest, toInsert});
            std::cout << "inserisco nodo " << dest.getName() << " e inserisco nel vettore " << src.getName() << std::endl;
        }
    }

    std::cout << "I FATTORI SONO " << factors.size() << std::endl;
    Graph output;
    for(Node n: nodes){
        if(!n.getCpt()->isHasDependence()) {
            Node newNode = n;
            std::cout << "aggiungo all'output nodo " << newNode.getName() << std::endl;


            output.addNode(newNode);
            /* si può migliorare facendo che graph abbia ptr a nodi e archi, in questo modo il ptr
            può essere condiviso e occupare meno memoria*/
            continue;
        }

        std::vector<Status> statuses = n.getStatuses();
        Node newNode = n;
        newNode.resetCPT();
        std::cout << "CPT HAS DEPENDENCE : " << newNode.getCpt()->isHasDependence() << std::endl;
        std::cout << "NODO " << newNode.getName() << std::endl;
        for(auto status: statuses){
            std::cout << "STATUS " << status << std::endl;
            float probability = computeStatusProbability(g, n, status, factors);
            std::cout << "PROBABILITA' CALCOLATA " << probability << std::endl;
            auto vi = std::make_shared<VariableInformations>(std::map<NodeId, Status>(), status);
            ConditionalProbability cp(vi,probability);
            newNode.getCpt()->addProbability(cp);
        }
        output.addNode(newNode);
    }
    return output;
}

float VariableElimination::computeStatusProbability(std::shared_ptr<Graph> g, Node &node, Status &s,
                                                    std::map <Node, std::vector<Node>> &factors) {
    auto nodes = g->getNodes();
    std::vector<Node> rightNodes;

    for(auto n : nodes) {
        if(n != node)
            rightNodes.push_back(n);
    }
    std::map<Node, Status> variables;
    float prob = 0;
    recursiveFunction(rightNodes, node, s, factors, variables, &prob);
    return prob;
}

void VariableElimination::recursiveFunction(std::vector<Node> nodes, Node& node, Status s, std::map <Node, std::vector<Node>> &factors, std::map<Node, Status> variables, float* p){

    if(nodes.empty()){
        /*std::cout << "AGGIUNGO PROB node " << node.getName() << " e stato " << s << std::endl;
        for(auto& a :variables)
            std::cout << a.first.getId() << " e stato " << a.second << std::endl;*/

        *p += computeProbability(factors, s, variables, node);
        return;
    }

    /*for(auto& n : nodes)
        std::cout << n.getName();

    std::cout << std::endl;*/
    auto nIt = nodes.back();
    //std::cout << "Erase del nodo " << nIt.getName() << std::endl;
    nodes.pop_back();
    for(auto& status: nIt.getStatuses()){
        //std::cout << "considero stato " << status << " del nodo " << nIt.getName() << std::endl;
        variables.insert({nIt,status});
        recursiveFunction(nodes, node, s, factors, variables, p);
        variables.erase(nIt);
    }
};

float VariableElimination::computeProbability(std::map <Node, std::vector<Node>> &factors, Status s, std::map<Node, Status> variables, Node node){
    float result = 1;

    std::map<NodeId, Status> toBeChecked;
    std::cout << "VARIABLES" << std::endl;
    for(auto& it : variables){
        std::cout << it.first.getId() << " : " << it.second << std::endl;
        toBeChecked.insert(std::pair<NodeId, Status>(it.first.getId(),it.second));
    }

    for(auto& f : factors){
        std::vector<ConditionalProbability> cpt = f.first.getCpt()->getCPTTable();
        for(auto& c : cpt){
            if(c.checkParentVectors(toBeChecked, node.getId(), s, f.first.getId())){
                result *= c.getProbability();
                break;
            }

        }
    }
    std::cout << "AGGIUNGO PROB " << result << std::endl;
    return result;
}