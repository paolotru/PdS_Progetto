//
// Created by S290225 on 09/08/2021.
//
#ifndef OUR_GRAPHELEMENTS_H
#define OUR_GRAPHELEMENTS_H
#include <string>
#include "../probability/CPT.h"

typedef int NodeId;
typedef std::string Status;

class Node{
private:
    std::string name;
    NodeId id;
    int n_states;
    std::vector<Status> statuses;
    std::shared_ptr<CPT> cpt;

public:
    Node(std::string& name,NodeId id);

    Node(const Node& n);

    ~Node();

    std::shared_ptr<CPT> getCPT(){
        return this->cpt;
    };

    std::vector<Status> getStatuses(){
        return this->statuses;
    }
};

class Arc{
private:
    Node source, destination;
public:
    Arc(Node n1,Node n2);

    Arc(const Arc& a);

    ~Arc();

    Node& getSource(){
        return source;
    }

    Node& getDestination(){
        return destination;
    }
};


#endif //OUR_GRAPHELEMENTS_H
