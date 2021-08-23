//
// Created by S290225 on 09/08/2021.
//
#include "Graph.h"

#include <utility>

std::vector<Arc> Graph::getArcs() {
    return this->arcs;
}
std::vector<Node> Graph::getNodes() {
    std::vector<Node> v;
    for(auto& n : nodes)
        v.push_back(n.second);
    return v;
}

Graph::Graph(std::map<NodeId , Node> n, std::vector<Arc> a) {
    nodes=std::move(n);
    arcs=std::move(a);
}

void Graph::addArc(const Arc& a) {
    arcs.push_back(a);
}
void Graph::addNode(Node& n) {
    //nodes.push_back(n);
    nodes.insert(std::pair<NodeId,Node>(n.getId(),n));

    std::cout << "SONO IN ADD NODE DI GRAPHH " << n.getStatuses().size() << std::endl;
}

Node Graph::getNodeById(NodeId id) {
    return nodes.find(id)->second;
}

Graph::Graph() = default;;

Graph::~Graph() = default;
