//
// Created by S290225 on 09/08/2021.
//
#include "Graph.h"

std::vector<Arc> Graph::getArcs() {
    return this->arcs;
}
std::vector<Node> Graph::getNodes() {
    return this->nodes;
}

Graph::Graph(std::vector<Node> n, std::vector<Arc> a) {
    nodes=std::move(n);
    arcs=std::move(a);
}

void Graph::addArc(NodeId n1, NodeId n2) {
    //nodes.push_back(n1)
    //arcs.push_back(new Arc(n1,n2));
}
void Graph::addNode(const Node& n) {
    nodes.push_back(n);
}

Graph::Graph() = default;;

Graph::~Graph() = default;
