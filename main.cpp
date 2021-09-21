#include <iostream>
#include "network/reader.h"
#include "inference/VariableElimination.h"

int main() {
    auto bn = std::make_shared<BayesianNetwork<double>>();

    BNReader<double> reader;

    clock_t start = clock();
    reader.loadNetworkFromFile("..\\exampleNetworks\\ARM_properties_net_alt.xdsl", bn);

    auto output = VariableElimination<double>::inferVariableProbability(bn->getGraph());
    auto nodes = output.getNodes();
    for(auto ito = nodes.begin(); ito != nodes.end(); ito++){
        std::cout << "\nNODE " << ito->getName() << std::endl;
        auto c = ito->getCpt();
        for(auto& tp : c->getCPTTable())
            std::cout << "P(" << ito->getName() << " = " << tp.getVariableInfo().getStatus() << ") = " << tp.getProbability() << std::endl;

        std::cout << std::endl;
    }

    clock_t end = clock();

    std::cout <<  "TIME: " << (float) (end-start)/CLOCKS_PER_SEC*1000.0 << std::endl;
    return 0;
}
