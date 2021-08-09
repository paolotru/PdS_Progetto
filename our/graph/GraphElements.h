//
// Created by S290225 on 09/08/2021.
//
#ifndef OUR_GRAPHELEMENTS_H
#define OUR_GRAPHELEMENTS_H
#include <string>

typedef int NodeId;

class Node{
private:
    std::string name;
    NodeId id;
    int n_states;
    //cpt e vettore padri

public:
    Node(std::string& name,NodeId id);

    Node(const Node& n);

    ~Node();
};

class Arc{
private:
    NodeId n_start,n_destination;
public:
    Arc(NodeId n1,NodeId n2);

    Arc(const Arc& a);

    ~Arc();
};

class Graph {

};


#endif //OUR_GRAPHELEMENTS_H
