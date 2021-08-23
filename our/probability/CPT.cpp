//
// Created by S290225 on 09/08/2021.
//

#include "CPT.h"


CPT::CPT(std::vector<float> probabilities, std::map<NodeId,std::vector<Status>> parents, std::vector<Status> states) {
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
        std::vector<NodeId> parentsId;
        std::vector<std::vector<Status>> parentsStatuses;
        for(auto & parent : parents) {
            parentsId.push_back(parent.first);
            parentsStatuses.push_back(parent.second);
        }
        int c=0;
        for (int i = 0; i < parents.size(); i++) {

            for (int k = 0; k < parentsStatuses[i].size(); ++k) {
                std::map<NodeId,Status> map;
                map[parentsId[i]]=(parentsStatuses[i])[k];          //TODO: make shared?

                for (int j = 0; j < states.size(); ++j) {
                    std::shared_ptr<VariableInformations> vi_p = std::make_shared<VariableInformations>(map, states[i]);
                    cpt_table.emplace_back(ConditionalProbability(vi_p, probabilities[c++]));
                }
            }
        }
    }
}

VariableInformations::VariableInformations(std::map<NodeId, Status> parents, Status status) {
    this->parents=std::move(parents);
    this->status=std::move(status);
}

VariableInformations::VariableInformations(const VariableInformations& vi) {
    parents=vi.parents;
    status=vi.status;
}

bool VariableInformations::operator==(const VariableInformations &rhs) const {
    return parents == rhs.parents &&
           status == rhs.status;
}

bool VariableInformations::operator!=(const VariableInformations &rhs) const {
    return !(rhs == *this);
}

ConditionalProbability::ConditionalProbability(std::shared_ptr<VariableInformations> vInfo, float probability)
        : v_info(std::move(vInfo)), probability(probability) {}

ConditionalProbability::ConditionalProbability(const ConditionalProbability& cp) {
    probability=cp.probability;
    v_info=cp.v_info;
}
