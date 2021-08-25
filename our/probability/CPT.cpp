//
// Created by S290225 on 09/08/2021.
//

#include "CPT.h"

void rec_f(int pos,int n,std::vector<std::map<NodeId,Status>> &v_map,std::map<NodeId,Status> map,const std::vector<NodeId>& parentsId,const std::vector<std::vector<Status>>& parentsStatuses);

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
        int np=1;
        std::vector<NodeId> parentsId;
        std::vector<std::vector<Status>> parentsStatuses;
        for(auto & parent : parents) {
            parentsId.push_back(parent.first);
            parentsStatuses.push_back(parent.second);
            np *= parent.second.size();
        }
        int c=0;
        std::vector<std::map<NodeId,Status>> v_map;          //TODO: make shared?
        std::map<NodeId,Status> map;
        rec_f(0,np-1,v_map,map,parentsId,parentsStatuses);
        for (auto &comb : v_map) {
            for (auto &s : states) {
                std::shared_ptr<VariableInformations> vi_p = std::make_shared<VariableInformations>(comb, s);
                cpt_table.emplace_back(ConditionalProbability(vi_p, probabilities[c++]));
            }
        }
        map.clear();
        v_map.clear();
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

void rec_f(int pos,int n,std::vector<std::map<NodeId,Status>>& v_map,std::map<NodeId,Status> map,const std::vector<NodeId>& parentsId,const std::vector<std::vector<Status>>& parentsStatuses){
    if(pos>=n){
        v_map.push_back(map);
        return;
    }
    for (auto &s: parentsStatuses[pos]) {
        map[parentsId[pos]]=s;
        rec_f(pos+1,n,v_map,map,parentsId,parentsStatuses);
    }
}
