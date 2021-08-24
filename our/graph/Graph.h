//
// Created by S290225 on 09/08/2021.
//
#ifndef OUR_GRAPH_H
#define OUR_GRAPH_H
#include "GraphElements.h"
#include "vector"


class Graph {
    //std::vector<Node> nodes;
    std::map<NodeId,Node> nodes;
    std::vector<Arc> arcs;
public:
    Graph();
    Graph(std::map<NodeId, Node> n, std::vector<Arc> a);

    virtual ~Graph();

    std::vector<Node> getNodes();

    Node getNodeById(NodeId id);

    std::vector<Arc> getArcs() const;

    void addArc(const Arc& a);

    void addNode(Node n);

    Graph(const Graph& g){
        nodes = g.getMaps();
        arcs = g.getArcs();
    }

    Graph& operator=(const Graph& g){
        if(this != &g){
            this->arcs.clear();
            this->arcs = g.getArcs();
            this->nodes.clear();
            this->nodes = g.getMaps();
        }
        return *this;
    }

    std::map<NodeId, Node> getMaps() const{
        return nodes;
    }
};


#endif //OUR_GRAPH_H
