#ifndef OUR_CPT_H
#define OUR_CPT_H
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <map>
#include <iostream>

typedef int NodeId;
typedef std::string Status;

class VariableInformations{
private:
    std::map<NodeId, Status> parents;
    Status status;
public:
    VariableInformations(std::map<NodeId, Status> parents, Status status);

    Status getStatus(){
        return status;
    }
    std::map<NodeId, Status> getParents(){
        return parents;
    }
};

class ConditionalProbability{
private:
    std::shared_ptr<VariableInformations> v_info;
    float probability;
public:
    ConditionalProbability(std::shared_ptr<VariableInformations> vInfo, float probability);

    std::shared_ptr<VariableInformations> getVariableInfo(){
        return v_info;
    }
    float getProbability(){
        return probability;
    };

    bool checkParentVectors(std::map<NodeId, Status> variables, Status s){
        if(s != v_info->getStatus())
            return false;

        for(auto& cp : v_info->getParents()){
            bool found = false;
            for(auto& v : variables){
                if(v.first == cp.first && v.second == cp.second)
                    found = true;
            }

            if(!found)
                return false;
        }

        return true;
    }
};

class CPT{
private:
    bool hasDependence;
    std::vector<ConditionalProbability> cpt_table;
public:
    std::vector<ConditionalProbability> getCPTTable(){
        return this->cpt_table;
    }
    bool isHasDependence() const {
        return hasDependence;
    }

    void addProbability(ConditionalProbability p){
        cpt_table.push_back(p);
    }

    CPT(std::vector<float> probabilities, std::map<NodeId,std::vector<Status>> parents,std::vector<Status> states);

    CPT(): hasDependence(false){};

    void printCPT(){
        for(auto& tp : cpt_table)
            std::cout << "PROB " << tp.getProbability() <<std::endl;
    }
};


#endif //OUR_CPT_H
