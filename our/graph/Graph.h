//
// Created by S290225 on 09/08/2021.
//
#ifndef OUR_GRAPH_H
#define OUR_GRAPH_H
#include "GraphElements.h"
#include "vector"


class Graph {
    std::vector<Node> nodes;
    std::vector<Arc> arcs;
public:
    Graph(std::vector<Node> n,std::vector<Arc> a);

    virtual ~Graph();

    std::vector<Node> getNodes();

    std::vector<Arc> getArcs();

    void addArc(NodeId n1, NodeId n2);

};


#endif //OUR_GRAPH_H
