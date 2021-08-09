//
// Created by S290225 on 09/08/2021.
//

#include "Graph.h"

#include <utility>

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

Graph::~Graph() = default;
