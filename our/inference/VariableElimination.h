//
// Created by LucaS on 09/08/2021.
//

#ifndef PDS_PROGETTO_VARIABLEELIMINATION_H
#define PDS_PROGETTO_VARIABLEELIMINATION_H
#include "../graph/Graph.h"
#include "../graph/GraphElements.h"
#include <memory>

class VariableElimination {
public:
    static void inferVariableProbability(Graph& g);
};


#endif //PDS_PROGETTO_VARIABLEELIMINATION_H
