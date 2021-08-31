#include <iostream>
#include "network/reader.h"
#include "inference/VariableElimination.h"

int main() {
    auto bn = std::make_shared<BayesianNetwork<float>>();

    BNReader<float> reader;

    reader.loadNetworkFromFile("..\\exampleNetworks\\hailfinder.xdsl", bn);


    auto output = VariableElimination<float>::inferVariableProbability(bn->getGraph());
    auto nodes = output.getNodes();
    for(auto ito = nodes.begin(); ito != nodes.end(); ito++){
        std::cout << "\nNODE " << ito->getName() << std::endl;
        auto c = ito->getCpt();
        for(auto& tp : c->getCPTTable())
            std::cout << "P(" << ito->getName() << " = " << tp.getVariableInfo()->getStatus() << ") = " << tp.getProbability() << std::endl;

        std::cout << std::endl;
    }

    return 0;
}
