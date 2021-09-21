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
#include <sysinfoapi.h>
#include <cmath>

template <class T>
class VariableElimination {
    static T computeStatusProbability(Node<T> node, Status& s, std::map<Node<T>, std::vector<Node<T>>>& factors, std::vector<std::pair<std::map<NodeId,Status>, T>>& parallelization, std::mutex& m){
        T p = 0;
        for(int i = 0; i < parallelization.size(); i++){
            auto map = parallelization[i].first;
            if(map.find(node.getId())->second == s){
                m.lock();
                if(parallelization[i].second == -1){
                    m.unlock();
                    T prob = computeProbability(factors, s, map, node);
                    m.lock();
                    p += prob;
                    parallelization[i].second = prob;
                }
                else{
                    p += parallelization[i].second;
                }
                m.unlock();
            }
        }


        return p;
    }

    static T computeProbability(std::map <Node<T>, std::vector<Node<T>>>& factors, Status s, std::map<NodeId, Status> toBeChecked, Node<T> node){
        T result = 1;
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

        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        int numCPU = sysinfo.dwNumberOfProcessors;

        int numberOfNodesForThread = ceil((float)nodes.size() / numCPU);
        auto start = nodes.begin();
        auto end = start + numberOfNodesForThread;
        for(int i = 0; i < numCPU; i++){
            threads.emplace_back([&output, start, end, &factors, g, &m, &parallelization, &forParallelization](){
                for(auto n = start; n != end; n++){
                    if(!n->getCpt()->isHasDependence()) {
                        auto newNode = *n;
                        m.lock();
                        output.addNode(newNode);
                        m.unlock();
                        return;
                    }
                    std::vector<Status> statuses = n->getStatuses();
                    auto newNode = *n;
                    newNode.resetCPT();
                    for(auto status: statuses){
                        T probability = computeStatusProbability(*n, status, factors, parallelization, forParallelization);
                        VariableInformations vi(std::make_shared<std::map<NodeId, Status>>(), status);
                        ConditionalProbability<T> cp(vi,probability);
                        newNode.getCpt()->addProbability(cp);
                    }

                    m.lock();
                    output.addNode(newNode);
                    m.unlock();
                }
            });
            if(end == nodes.end())
                break;

            start = end;
            end = start + numberOfNodesForThread;
            if(end > nodes.end())
            {
                end = nodes.end();
            }
        }

        for(std::thread& t : threads)
            t.join();

        return output;
    };
};


#endif //OUR_VARIABLEELIMINATION_H