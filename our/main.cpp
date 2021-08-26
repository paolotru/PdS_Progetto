#include <iostream>
#include "network/reader.h"
#include "inference/VariableElimination.h"

int main() {
    auto bn = std::make_shared<BayesianNetwork<float>>();

    BNReader<float> reader;

    reader.loadNetworkFromFile("..\\exampleNetworks\\Coma.xdsl", bn);


    auto output = VariableElimination<float>::inferVariableProbability(bn->getGraph());
    auto nodes = output.getNodes();
    for(auto ito = nodes.begin(); ito != nodes.end(); ito++){
        std::cout << "\nNODE " << ito->getName() << std::endl;
        std::cout << "CPT HAS DEPENDENCE : " << ito->getCpt()->isHasDependence() << std::endl;
        auto c = ito->getCpt();
        c->printCPT();
    }

    return 0;
}
