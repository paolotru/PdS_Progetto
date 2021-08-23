#include <iostream>
#include "network/reader.h"

int main() {
    auto bn = std::make_shared<BayesianNetwork<float>>();

    BNReader reader;

    reader.loadNetworkFromFile("..\\exampleNetworks\\prova_1.xdsl", bn);
    std::cout <<"Nodi letti: " <<bn->getNodeMap().size() <<std::endl;
    std::cout <<"Archi letti: "  << bn->getGraph()->getArcs().size() << std::endl;
    return 0;
}
