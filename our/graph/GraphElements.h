//
// Created by S290225 on 09/08/2021.
//
#ifndef OUR_GRAPHELEMENTS_H
#define OUR_GRAPHELEMENTS_H
#include <string>
#include "CPT.h"

typedef int NodeId;

class Node{
private:
    std::string name;
    NodeId id;
    int n_states;
    std::shared_ptr<CPT> cpt;

public:
    Node(std::string& name,NodeId id);

    Node(const Node& n);

    ~Node();
};

class Arc{
private:
    Node n_start,n_destination;
public:
    Arc(Node n1,Node n2);

    Arc(const Arc& a);

    ~Arc();
};


#endif //OUR_GRAPHELEMENTS_H
