//
// Created by S290225 on 09/08/2021.
//

#include "CPT.h"

CPT::CPT(std::vector<float> probabilities, std::map<NodeId,std::vector<Status>> parents,std::vector<Status> states) {
    if(parents.empty()) {
        hasDependence = false;
        std::map<NodeId,Status> m;
        for (int i = 0;i<states.size();i++) {
            std::shared_ptr<VariableInformations> vi_p = std::make_shared<VariableInformations>(VariableInformations(m,states[i]));
            cpt_table.emplace_back(ConditionalProbability(vi_p,probabilities[i]));
        }
    }else{
        hasDependence=true;
        std::map<NodeId,Status> m;
        for (int i = 0;i<states.size();i++) {

            std::shared_ptr<VariableInformations> vi_p = std::make_shared<VariableInformations>(VariableInformations(m,states[i]));
            cpt_table.emplace_back(ConditionalProbability(vi_p,probabilities[i]));
        }
    }
}
