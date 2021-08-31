#ifndef OUR_CPT_H
#define OUR_CPT_H
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <iostream>
#include "../probability/COW.h"

typedef int NodeId;
typedef std::string Status;

class VariableInformations{
private:
    std::map<NodeId, Status> parents;
    Status status;
public:
    VariableInformations(std::map<NodeId, Status> parents, Status status);
    VariableInformations(const VariableInformations& vi);
    Status getStatus(){
        return status;
    }
    std::map<NodeId, Status> getParents(){
        return parents;
    }

    void printParents(){
        for(auto c : parents)
            std::cout << " " << c.first << " " << c.second << ",";
    }

    bool operator==(const VariableInformations &rhs) const;

    bool operator!=(const VariableInformations &rhs) const;
};

template <class T>
class ConditionalProbability{
private:
    std::shared_ptr<VariableInformations> v_info;
    T probability;

public:
    ConditionalProbability(std::shared_ptr<VariableInformations> vInfo, T probability): v_info(std::move(vInfo)), probability(probability) {};

    ConditionalProbability(const ConditionalProbability& cp) {
        probability=cp.probability;
        v_info=cp.v_info;
    };

    std::shared_ptr<VariableInformations> getVariableInfo() const{
        return v_info;
    };

    T getProbability() const{
        return probability;
    };

    bool checkParentVectors(std::map<NodeId, Status> variables, NodeId n, Status s, NodeId cptId){
        if(n == cptId && s != v_info->getStatus())
            return false;

        if(variables.find(cptId) != variables.end() && variables.find(cptId)->second != v_info->getStatus())
            return false;

        for(auto& cp : v_info->getParents()){
            bool found = false;
            for(auto& v : variables){
                if(v.first == cp.first && v.second == cp.second)
                    found = true;
            }

            if(!found){
                if(n == cp.first && s == cp.second)
                    found = true;
            }

            if(!found)
                return false;
        }

        return true;
    }
};

template <class T>
class CPT{
private:
    bool hasDependence;
    std::vector<ConditionalProbability<T>> cpt_table;


public:
    void rec_f(int pos,int n,std::vector<std::map<NodeId,Status>>& v_map,std::map<NodeId,Status> map,const std::vector<NodeId> parentsId,const std::vector<std::vector<Status>> parentsStatuses){
        if(pos>=n){
            v_map.push_back(map);
            return;
        }

        for (auto s : parentsStatuses[pos]) {
            map[parentsId[pos]] = s;
            rec_f(pos+1, n,v_map,map,parentsId,parentsStatuses);
        }
    };
    std::vector<ConditionalProbability<T>> getCPTTable(){
        return this->cpt_table;
    }
    bool isHasDependence() const {
        return hasDependence;
    }

    void addProbability(ConditionalProbability<T> p){
        cpt_table.push_back(p);
    }

    CPT(std::vector<T> probabilities, std::map<NodeId,std::vector<Status>> parents, std::vector<Status> states) {
        if(parents.empty()) {
            hasDependence = false;
            std::map<NodeId,Status> m;
            for (int i = 0;i<states.size();i++) {
                std::shared_ptr<VariableInformations> vi_p = std::make_shared<VariableInformations>(m,states[i]);
                ConditionalProbability cp(vi_p,probabilities[i]);
                cpt_table.emplace_back(cp);
            }
        }else{
            hasDependence=true;
            int np=1;
            std::vector<NodeId> parentsId;
            std::vector<std::vector<Status>> parentsStatuses;
            for(auto & parent : parents) {
                parentsId.push_back(parent.first);
                parentsStatuses.push_back(parent.second);
                np *= parent.second.size();
            }
            int c=0;
            std::vector<std::map<NodeId,Status>> v_map;
            std::map<NodeId,Status> map;
            rec_f(0, parentsId.size(),v_map,map,parentsId,parentsStatuses);
            for(auto comb=v_map.begin(); comb != v_map.end(); comb++){

                for(auto s : states) {
                    std::shared_ptr<VariableInformations> vi_p = std::make_shared<VariableInformations>(*comb, s);
                    cpt_table.emplace_back(ConditionalProbability(vi_p, probabilities[c++]));
                }
            }
            map.clear();
            v_map.clear();
        }
    }

    CPT(): hasDependence(false){};

    void printCPT() {
        for (auto &tp : cpt_table) {
            if(hasDependence)
            {
                std::cout << "PARENTS : ";
                tp.getVariableInfo()->printParents();
                std::cout << std::endl;
            }

            std::cout << "Status: " << tp.getVariableInfo()->getStatus() << std::endl;
            std::cout << "PROB " << tp.getProbability() << std::endl;
        }
    }
};


#endif //OUR_CPT_H
