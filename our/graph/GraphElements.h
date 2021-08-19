#ifndef OUR_GRAPHELEMENTS_H
#define OUR_GRAPHELEMENTS_H
#include <string>
#include <memory>
#include <vector>
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
    const std::shared_ptr<CPT> &getCpt() const;
    bool operator==(const Node &rhs) const;
    bool operator!=(const Node &rhs) const;

    Node(std::string& name,NodeId id,int ns);
    Node(std::string& name,std::vector<std::string> states,NodeId id,std::vector<float> probabilities,std::vector<std::string> parents);

    Node(const Node& n);

    Node();

    ~Node();

    NodeId getNodeId() const;

    std::vector<Status> getStatuses();
};

class Arc{
private:
    Node source, destination;
public:
    Arc(Node n1,Node n2);

    Arc(const Arc& a);

    ~Arc();

    Node& getSource();

    Node& getDestination();
};


#endif //OUR_GRAPHELEMENTS_H
