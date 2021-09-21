//
// Created by S290225 on 09/08/2021.
//
#ifndef OUR_GRAPH_H
#define OUR_GRAPH_H
#include "GraphElements.h"
#include "vector"

template <class T>
class Graph {
    //std::vector<Node> nodes;
    std::map<NodeId,Node<T>> nodes;
    std::vector<Arc<T>> arcs;
public:
    Graph(){};

    Graph(std::map<NodeId , Node<T>> n, std::vector<Arc<T>> a) {
        nodes=std::move(n);
        arcs=std::move(a);
    }

    std::vector<Node<T>> getNodes() const{
        std::vector<Node<T>> v;
        for(auto n : nodes)
            v.push_back(n.second);
        return v;
    }

    Node<T> getNodeById(NodeId id) {
        return nodes.find(id)->second;
    };

    std::vector<Arc<T>> getArcs() const{
        return this->arcs;
    }

    void addArc(const Arc<T>& a) {
        arcs.push_back(a);
    };

    void addNode(Node<T> n) {
        nodes.insert(std::pair<NodeId, Node<T>>(n.getId(),n));
    }

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

    std::map<NodeId, Node<T>> getMaps() const{
        return nodes;
    }
};


#endif //OUR_GRAPH_H
