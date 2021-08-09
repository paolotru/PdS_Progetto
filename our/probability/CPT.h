#ifndef OUR_CPT_H
#define OUR_CPT_H
#include <string>
#include <vector>
#include <map>
#include "../graph/GraphElements.h"

typedef std::string Status;

class VariableInformations{
private:
    std::map<Node, Status> parents;
    Status status;
};
class ConditionalProbability{
private:
    std::shared_ptr<VariableInformations> v_info;
    float probability;
};

class CPT {
private:
    std::vector<ConditionalProbability> cpt_table;
};


#endif //OUR_CPT_H
