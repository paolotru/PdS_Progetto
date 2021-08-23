//
// Created by S290225 on 09/08/2021.
//

#include "GraphElements.h"

Node::Node(std::string &name, NodeId id,int ns) : name(name),id(id),n_states(ns) {

}

Node::Node(const Node &n) {
    name=n.name;
    id=n.id;
    n_states=n.n_states;
}

Node::~Node() = default;

bool Node::operator==(const Node &rhs) const {
    return id == rhs.id;
}

bool Node::operator!=(const Node &rhs) const {
    return id != rhs.id;
}

std::vector<Status> Node::getStatuses() {
    return statuses;
}

const std::shared_ptr<CPT> &Node::getCpt() const {
    return cpt;
}



Arc::Arc(Node n1, Node n2) : source(n1),destination(n2){

}

Arc::Arc(const Arc &a) : source(a.source), destination(a.destination) {}

Arc::~Arc() = default;

Node & Arc::getSource() {
    return source;
}
Node & Arc::getDestination() {
    return destination;
}


Node::Node(std::string &name, std::vector<Status> states, NodeId id, std::vector<float> probabilities, std::map<NodeId,std::vector<Status>> parents) {
    this->name=std::move(name);
    this->id=id;
    n_states=states.size();
    statuses=std::move(states);
    this->cpt=std::make_shared<CPT>(probabilities,parents,states);
}

NodeId Node::getId() const {
    return id;
}

bool Node::operator<(const Node &rhs) const {
    return id < rhs.id;
}

bool Node::operator>(const Node &rhs) const {
    return rhs < *this;
}

bool Node::operator<=(const Node &rhs) const {
    return !(rhs < *this);
}

bool Node::operator>=(const Node &rhs) const {
    return !(*this < rhs);
}

std::string Node::getName() {
    return name;
}

void Node::setId(NodeId id) {
    Node::id = id;
}

