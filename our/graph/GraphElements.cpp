//
// Created by S290225 on 09/08/2021.
//

#include "GraphElements.h"

Node::Node(std::string &name, NodeId id) : name(name),id(id) {

}

Node::Node(const Node &n) {
    name=n.name;
    id=n.id;
    n_states=n.n_states;
}

Node::~Node() {

}

Arc::Arc(Node n1, Node n2) : start(n1),destination(n2){

}

Arc::~Arc() {

}

Arc::Arc(const Arc &a) {
    destination=a.destination;
    start=a.start;
}
