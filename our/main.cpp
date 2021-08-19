#include <iostream>
#include "network/reader.h"

int main() {
    auto bn = std::make_shared<BayesianNetwork<float>>();

    BNReader<float> reader;

    reader.loadNetworkFromFile("..\\..\\..\\..\\Bayesian Network\\exampleNetworks\\barley.xdsl", bn);
    return 0;
}
