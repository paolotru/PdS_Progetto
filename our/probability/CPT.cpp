//
// Created by S290225 on 09/08/2021.
//

#include "CPT.h"

#include <utility>

CPT::CPT(std::vector<float> probabilities, std::map<NodeId,std::vector<Status>> parents,std::vector<Status> states) {
    if(parents.empty()) {
        std::cout << states.size() << std::endl;
        hasDependence = false;
        std::map<NodeId,Status> m;
        for (int i = 0;i<states.size();i++) {
            std::shared_ptr<VariableInformations> vi_p = std::make_shared<VariableInformations>(m,states[i]);
            cpt_table.emplace_back(ConditionalProbability(vi_p,probabilities[i]));
            std::cout << cpt_table.size() << std::endl;
        }
    }else{
        hasDependence=true;
        std::vector<NodeId> parentsId;
        std::vector<std::vector<Status>> parentsStatuses;
        for(auto & parent : parents) {
            parentsId.push_back(parent.first);
            parentsStatuses.push_back(parent.second);
        }
        for (int i = 0; i < parents.size(); i++) {

            for (int k = 0; k < parentsStatuses[i].size(); ++k) {
                std::map<NodeId,Status> map;
                map[parentsId[i]]=(parentsStatuses[i])[k];          //TODO: make shared?

                for (int j = 0; j < states.size(); ++j) {
                    std::shared_ptr<VariableInformations> vi_p = std::make_shared<VariableInformations>(map, states[i]);
                    cpt_table.emplace_back(ConditionalProbability(vi_p, probabilities[i]));
                }
            }
        }
    }
}

VariableInformations::VariableInformations(std::map<NodeId, Status> parents, Status status) {
    parents=std::move(parents);
    status=std::move(status);
}

ConditionalProbability::ConditionalProbability(std::shared_ptr<VariableInformations> vInfo, float probability)
        : v_info(std::move(vInfo)), probability(probability) {}
