#include <iostream>
#include "network/reader.h"
#include "inference/VariableElimination.h"

int main() {
    auto bn = std::make_shared<BayesianNetwork<float>>();

    BNReader reader;

    reader.loadNetworkFromFile("..\\exampleNetworks\\Coma.xdsl", bn);

   Graph output = VariableElimination::inferVariableProbability(bn->getGraph());
   std::vector<Node> nodes = output.getNodes();
   for(auto ito = nodes.begin(); ito != nodes.end(); ito++){
       std::cout << "\nNODE " << ito->getName() << std::endl;
       std::cout << "CPT HAS DEPENDENCE : " << ito->getCpt()->isHasDependence() << std::endl;
       auto c = ito->getCpt();
       c->printCPT();
   }

    return 0;
}
