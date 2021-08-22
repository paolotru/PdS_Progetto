#include <iostream>
#include "network/reader.h"

int main() {
    auto bn = std::make_shared<BayesianNetwork<float>>();

    BNReader reader;

    reader.loadNetworkFromFile(".\\exampleNetworks\\prova_1.xdsl", bn);
    return 0;
}
