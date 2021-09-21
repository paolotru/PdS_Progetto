//
// Created by S290225 on 09/08/2021.
//

#include "CPT.h"

VariableInformations::VariableInformations(std::shared_ptr<std::map<NodeId, Status>> parents, Status status) {
    this->parents=std::move(parents);
    this->status=std::move(status);
}

VariableInformations::VariableInformations(const VariableInformations& vi) {
    parents=vi.parents;
    status=vi.status;
}

bool VariableInformations::operator==(const VariableInformations &rhs) const {
    return *parents == *rhs.parents && status == rhs.status;
}

bool VariableInformations::operator!=(const VariableInformations &rhs) const {
    return !(rhs == *this);
}



