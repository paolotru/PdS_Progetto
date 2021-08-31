//
// Created by S290225 on 12/08/2021.
//

#ifndef OUR_VARIABLEELIMINATION_H
#define OUR_VARIABLEELIMINATION_H
#include "../graph/Graph.h"
#include <memory>
#include <thread>
#include <mutex>

template <class T>
class VariableElimination {
    static T computeStatusProbability(std::shared_ptr<Graph<T>> g, Node<T> node, Status& s, std::map<Node<T>, std::vector<Node<T>>>& factors){
        auto nodes = g->getNodes();
        std::vector<Node<T>> rightNodes;

        for(auto n : nodes) {
            if(n != node)
                rightNodes.push_back(n);
        }
        std::map<Node<T>, Status> variables;
        T prob = 0;
        recursiveFunction(rightNodes, node, s, factors, variables, &prob);
        return prob;
    }

    static void recursiveFunction(std::vector<Node<T>> nodes, Node<T>& node, Status s, std::map <Node<T>, std::vector<Node<T>>>& factors, std::map<Node<T>, Status> variables, T* p){
        if(nodes.empty()){
            *p += computeProbability(factors, s, variables, node);
            return;
        }

        auto nIt = nodes.back();
        nodes.pop_back();
        for(auto& status: nIt.getStatuses()){
            variables.insert({nIt,status});
            recursiveFunction(nodes, node, s, factors, variables, p);
            variables.erase(nIt);
        }
    }
    static T computeProbability(std::map <Node<T>, std::vector<Node<T>>>& factors, Status s, std::map<Node<T>, Status> variables, Node<T> node){
        T result = 1;

        std::map<NodeId, Status> toBeChecked;
        for(auto& it : variables){
            toBeChecked.insert(std::pair<NodeId, Status>(it.first.getId(),it.second));
        }

        for(auto& f : factors){
            std::vector<ConditionalProbability<T>> cpt = f.first.getCpt()->getCPTTable();
            for(auto& c : cpt){
                if(c.checkParentVectors(toBeChecked, node.getId(), s, f.first.getId())){
                    result *= c.getProbability();
                    break;
                }

            }
        }
        return result;
    }
public:
    static Graph<T> inferVariableProbability(std::shared_ptr<Graph<T>> g){
        std::vector<Arc<T>> arcs = g->getArcs();
        std::vector<Node<T>> nodes = g->getNodes();

        std::map<Node<T>, std::vector<Node<T>>> factors;

        for(auto& n : nodes){
            if(!n.getCpt()->isHasDependence()){
                std::vector<Node<T>> toInsert;
                factors.insert({n, toInsert});
            }

        }
        for(auto& a : arcs){
            Node<T> src = a.getSource();
            Node<T> dest = a.getDestination();

            auto nodeAlreadyPresent = factors.find(dest);
            if(nodeAlreadyPresent != factors.end()){
                nodeAlreadyPresent->second.push_back(src);
            }
            else{
                std::vector<Node<T>> toInsert;
                toInsert.push_back(src);
                factors.insert({dest, toInsert});
            }
        }
        Graph<T> output;

        std::vector<std::thread> threads;

        for(auto n: nodes){
            threads.emplace_back([&output, n, &factors, g](){
                if(!n.getCpt()->isHasDependence()) {
                    auto newNode = n;
                    output.addNode(newNode);
                    return;
                }

                std::vector<Status> statuses = n.getStatuses();
                auto newNode = n;
                newNode.resetCPT();
                for(auto status: statuses){
                    T probability = computeStatusProbability(g, n, status, factors);
                    auto vi = std::make_shared<VariableInformations>(std::map<NodeId, Status>(), status);
                    ConditionalProbability<T> cp(vi,probability);
                    newNode.getCpt()->addProbability(cp);
                }
                output.addNode(newNode);
            });
        }

        for(std::thread& t : threads)
            t.join();

        return output;
    };
};


#endif //OUR_VARIABLEELIMINATION_H