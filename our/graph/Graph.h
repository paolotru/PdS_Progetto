//
// Created by S290225 on 09/08/2021.
//
#ifndef OUR_GRAPH_H
#define OUR_GRAPH_H

#include "GraphElements.h"


class Graph {
    std::vector<Node> nodes;
    std::vector<Arc> arcs;
public:
    std::vector<Node> getNodes(){
        return this->nodes;
    }

    std::vector<Arc> getArcs(){
        return this->arcs;
    }

};


#endif //OUR_GRAPH_H
