#include <iostream>
#include "network/reader.h"
#include "inference/VariableElimination.h"

int main() {
    auto bn = std::make_shared<BayesianNetwork<float>>();

    BNReader reader;

    reader.loadNetworkFromFile("..\\exampleNetworks\\prova_1.xdsl", bn);
    std::cout <<"Nodi letti: " <<bn->getNodeMap().size() <<std::endl;
    std::cout <<"Archi letti: "  << bn->getGraph()->getArcs().size() << std::endl;

    auto g = bn->getGraph();
    auto n = g->getNodes();

   for(auto it = n.begin(); it != n.end(); it++){
        std::cout << "\nNODE " << it->getId() << std::endl;
        auto c = it->getCpt();
        c->printCPT();
   }

   Graph output = VariableElimination::inferVariableProbability(g);
   auto nodes = output.getNodes();

   for(auto it = nodes.begin(); it != nodes.end(); it++){
       std::cout << "\nNODE " << it->getId() << std::endl;
       auto c = it->getCpt();
       c->printCPT();
   }

    return 0;
}
