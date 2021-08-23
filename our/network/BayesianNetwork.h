//
// Created by S290225 on 11/08/2021.
//

#ifndef OUR_BAYESIANNETWORK_H
#define OUR_BAYESIANNETWORK_H

#include "../graph/Graph.h"

template <class T = float>
class BayesianNetwork {
private:
//    std::shared_ptr<DAG> m_bn;
//    CPTs<T> m_cpt;
//    std::map<NodeId, NodeId> m_nodeWithSameCPT;
//    VariableNodeMap m_vnm;
    std::shared_ptr<Graph> graph;
    std::map<std::string, Node> nodeMap;

public:
    //constructor
    BayesianNetwork() : graph(std::make_shared<Graph>()) {

    }
    //copy constructor
    BayesianNetwork(const BayesianNetwork& bn) {
        bn = bn.graph;
    }

    //destructor
    ~BayesianNetwork() {
        graph.reset();
    }

    //copy operator
    BayesianNetwork& operator=(const BayesianNetwork& bn) {
        if (this != &bn) {
            graph = bn.graph;
        }
        return *this;
    }

    std::map<std::string, Node> getNodeMap(){
        return nodeMap;
    };

    std::shared_ptr<Graph> getGraph(){
        return graph;
    }

    //boolean operator
    bool operator==(const BayesianNetwork& bn) const {
        if (*bn == *(bn.graph))
            return true;
        else
            return false;
    }
    void addNode(Node n){
        nodeMap.insert(std::pair<std::string,Node>(n.getName(),n));  //TODO Make shared?
        graph->addNode(n);
    }
    NodeId idFromName(const std::string& variableName) {
        auto x=nodeMap.find(variableName);
        if(x !=nodeMap.end())
            return x->second.getId();
        else
            return -1;
    }
    std::vector<Status> getStatesById(NodeId parent){
        return graph->getNodeById(parent).getStatuses();
    }
    //adds and arc to the graph
//    void addArc(NodeId node1, NodeId node2) {
//        graph->addArc(node1, node2);
//    }
//
//    //adds arcs to the graphs using the dependencies obtained from the CPTS
//    void addArcsFromCPTs() {
//        for (auto& CPT: m_cpt) {
//            std::vector<VarStates> parents = CPT.getVariables();
//
//            for (auto it2 = parents.begin(); it2 != std::prev(parents.end()); it2++) {
//                m_bn->addArc(it2->m_id, CPT.getCPTId());
//            }
//        }
//
//        m_bn->setNumberOfNodes(m_vnm.getNumberOfVariables());
//    }
//
//    //void addProbabilities(const NodeId n, const std::vector<int>& var, const std::vector<StateProb<T>>& probs) {
//    //	m_cpt.find(n)->second.addProbability(var, probs);
//    //}
//
//    //adds an empty variable to the graph, wihtout connecting it.
//    //used for debug purposes
//    void addVariable(const std::string& variableName) {
//        NodeId id = m_vnm.addVariable(variableName);
//        CPT<T> cpt;
//        m_cpt.push_back(cpt);
//    }
//
//    //adds a variable to the graph, without connecting it
//    //used for debug purposes
//    void addVariable(const std::string& variableName, CPT<T> cpt) {
//        m_vnm.addVariable(variableName);
//
//        for (auto &CPT : m_cpt) {
//            if (CPT.isCPTDataEqualTo(cpt)) {
//                cpt.duplicateCPT(CPT);
//                m_nodeWithSameCPT.emplace(cpt.getCPTId(), CPT.getCPTId());
//                break;
//            }
//        }
//
//        m_cpt.push_back(cpt);
//    }
//
//    //adds a variable to the graph, using the same CPT of another variable, without connecting it
//    //used for debug purposes
//    void addVariable(NodeId source, const std::string& variableName) {
//        m_vnm.addVariable(variableName);
//        NodeId id = m_vnm.idFromName(variableName);
//        CPT<T> sourceCPT = m_cpt.at(source);
//        m_cpt.push_back(sourceCPT);
//
//        if (m_nodeWithSameCPT.find(source) != m_nodeWithSameCPT.end()) {
//            m_nodeWithSameCPT.emplace(id, m_nodeWithSameCPT.find(source)->second);
//        }
//        else {
//            m_nodeWithSameCPT.emplace(id, source);
//        }
//    }
//
//    //onyl for debug purposes at the moment
//    //void displayCPT(const NodeId n) const {
//    //	m_cpt.find(n)->second.displayCPT();
//    //}
//
//    //returns the CPT of node n
//    CPT<T> getCPT(const NodeId n) {
//        return m_cpt.at(n);
//    }
//
//    int getNumberOfVariables() {
//        return m_vnm.getNumberOfVariables();
//    }
//
//    //wrapper function that returns the number of states of a given variable node
//    int getVariableStatesNum(const NodeId node) {
//        return m_cpt.at(node).getStatesNum();
//    }
//
//    bool hasChildren(const NodeId node) {
//        return m_bn->hasChildren(node);
//    }
//
//    //returns the id of a variable given its name

//
//
//    //wrapper function that removes a node from the DAG
//    void removeArc(NodeId node1, NodeId node2) {
//        m_bn->removeArc(node1, node2);
//    }
//
//    //wrapper function for changing the probability of a combination of states inside the CPT of node n
//    void updateProbabilities(const NodeId n, const std::vector<int>& var, T prob) {
//        m_cpt.at(n).updateProbabilities(var, prob);
//    }

};

#endif //OUR_BAYESIANNETWORK_H
