#ifndef OUR_GRAPHELEMENTS_H
#define OUR_GRAPHELEMENTS_H
#include <string>
#include <memory>
#include <vector>
#include "../probability/CPT.h"

typedef int NodeId;
typedef std::string Status;

template <class T>
class Node{
private:
    std::string name;
    NodeId id;
    int n_states;
    std::vector<Status> statuses;
    std::shared_ptr<CPT<T>> cpt;
public:
    Node(){};

    const std::shared_ptr<CPT<T>> getCpt() const {
        return cpt;
    }
    bool operator==(const Node &rhs) const {
        return id == rhs.id;
    }
    Node& operator=(const Node& source) {
        if(this != &source){
            this->n_states = source.n_states;
            this->name = source.name;
            this->id = source.id;
            this->cpt.reset();
            this->cpt = source.cpt;
            this->statuses = source.statuses;
        }

        return *this;
    };

    bool operator!=(const Node &rhs) const {
        return id != rhs.id;
    }


    Node(std::string &name, std::vector<Status> states, NodeId id, std::vector<T> probabilities, std::map<NodeId,std::vector<Status>> parents) {
        this->name=std::move(name);
        this->id=id;
        this->n_states=states.size();
        this->cpt=std::make_shared<CPT<T>>(probabilities,parents,states);
        this->statuses=states;
    };
    Node(std::string &name,std::vector<Status> states, NodeId id, std::shared_ptr<CPT<T>> cpt) {
        this->name=std::move(name);
        this->id=id;
        this->n_states=states.size();
        this->cpt=cpt;
        this->statuses=states;
    };

    Node(const Node &n) {
        name=n.name;
        id=n.id;
        n_states=n.n_states;
        statuses=n.statuses;
        cpt=n.cpt;
    }

    std::vector<Status> getStatuses() const{
        return this->statuses;
    }

    NodeId getId() const {
        return id;
    }

    std::string getName() const{
        return name;
    }

    void setId(NodeId id) {
        Node::id = id;
    }

    void resetCPT(){
        cpt.reset();
        cpt = std::make_shared<CPT<T>>();
    }

    bool operator<(const Node<T> &rhs) const {
        return id < rhs.id;
    }

    bool operator>(const Node<T> &rhs) const {
        return rhs < *this;
    }

    bool operator<=(const Node<T> &rhs) const {
        return !(rhs < *this);
    }

    bool operator>=(const Node<T> &rhs) const {
        return !(*this < rhs);
    }
};

template <class T>
class Arc{
private:
    Node<T> source, destination;
public:
    Arc(Node<T> n1, Node<T> n2) : source(n1),destination(n2){};

    Arc(const Arc &a) : source(a.source), destination(a.destination) {};

    Node<T> getSource() const{
        return source;
    }

    Node<T> getDestination() const{
        return destination;
    }
};


#endif //OUR_GRAPHELEMENTS_H
