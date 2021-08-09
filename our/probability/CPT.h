#ifndef OUR_CPT_H
#define OUR_CPT_H
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "../graph/GraphElements.h"



class VariableInformations{
private:
    std::map<Node, Status> parents;
    Status status;
};

class ConditionalProbability{

    std::shared_ptr<VariableInformations> v_info;
    float probability;

public:
    std::shared_ptr<VariableInformations> getVariableInfo(){
        return v_info;
    }

    float getProbability(){
        return probability;
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
};


#endif //OUR_CPT_H
