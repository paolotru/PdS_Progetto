//
// Created by S290225 on 12/08/2021.
//

#ifndef OUR_VARIABLEELIMINATION_H
#define OUR_VARIABLEELIMINATION_H
#include "../graph/Graph.h"
#include <memory>
#include <utility>
#include <thread>
#include <mutex>

template <class T>
class VariableElimination {
    static T computeStatusProbability(std::shared_ptr<Graph<T>> g, Node<T> node, Status& s, std::map<Node<T>, std::vector<Node<T>>>& factors, std::vector<std::pair<std::map<NodeId,Status>, T>>& parallelization, std::mutex& m){
        std::vector<Node<T>> nodes = g->getNodes();
        std::vector<Node<T>> rightNodes;

        for(Node<T> n : nodes) {
            if(n != node)
                rightNodes.push_back(n);
        }
        std::map<Node<T>, Status> variables;
        T prob = 0;
        recursiveFunction(rightNodes, node, s, factors, variables, &prob, parallelization, m);
        return prob;
    }

    static void recursiveFunction(std::vector<Node<T>> nodes, Node<T>& node, Status s, std::map <Node<T>, std::vector<Node<T>>>& factors, std::map<Node<T>, Status> variables, T* p, std::vector<std::pair<std::map<NodeId,Status>, T>>& parallelization, std::mutex& m){
        if(nodes.empty()){
            std::map<NodeId, Status> toCheck;
            for(auto v = variables.begin(); v != variables.end(); v++){
                toCheck.insert(std::make_pair(v->first.getId(), v->second));
            }
            toCheck.insert(std::make_pair(node.getId(), s));
            m.lock();
            for(auto it = parallelization.begin(); it != parallelization.end(); it++){
                if(it->first == toCheck){
                    if(it->second == -1){
                        m.unlock();
                        T prob = computeProbability(factors, s, variables, node);
                        m.lock();
                        *p += prob;
                        it->second = prob;
                    }
                    else{
                        *p += it->second;
                    }
                    break;
                }
            }
            m.unlock();
            return;
        }

        auto nIt = nodes.back();
        nodes.pop_back();
        for(auto& status: nIt.getStatuses()){
            variables.insert({nIt,status});
            recursiveFunction(nodes, node, s, factors, variables, p, parallelization, m);
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

    static void fill_parallelization_data_structure(std::vector<std::pair<std::map<NodeId,Status>, T>>& parallelization, std::vector<Node<T>> nodes, int pos, std::map<NodeId, Status> map){
        if(pos == nodes.size()){
            auto pair = std::make_pair(map, (float)-1);
            parallelization.push_back(pair);
            return;
        }

        auto statuses = nodes[pos].getStatuses();
        for(auto s : statuses){
            map.insert(std::make_pair(nodes[pos].getId(), s));
            fill_parallelization_data_structure(parallelization, nodes, pos+1, map);
            map.erase(nodes[pos].getId());
        }
        return;
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
        std::mutex m, forParallelization;
        std::vector<std::pair<std::map<NodeId,Status>, T>> parallelization;
        std::map<NodeId, Status> datas;
        fill_parallelization_data_structure(parallelization, g->getNodes(), 0, datas);

        for(auto n: nodes){
            threads.emplace_back([&output, n, &factors, g, &m, &parallelization, &forParallelization](){
                if(!n.getCpt()->isHasDependence()) {
                    auto newNode = n;
                    m.lock();
                    output.addNode(newNode);
                    m.unlock();
                    return;
                }

                std::vector<Status> statuses = n.getStatuses();
                auto newNode = n;
                newNode.resetCPT();
                for(auto status: statuses){
                    T probability = computeStatusProbability(g, n, status, factors, parallelization, forParallelization);
                    VariableInformations vi(std::make_shared<std::map<NodeId, Status>>(), status);
                    ConditionalProbability<T> cp(vi,probability);
                    newNode.getCpt()->addProbability(cp);
                }

                m.lock();
                output.addNode(newNode);
                m.unlock();
            });
        }

        for(std::thread& t : threads)
            t.join();

        return output;
    };
};


#endif //OUR_VARIABLEELIMINATION_H