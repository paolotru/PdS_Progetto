#include <iostream>
#include <sysinfoapi.h>
#include "network/reader.h"
#include "inference/VariableElimination.h"

int main() {
    auto bn = std::make_shared<BayesianNetwork<double>>();

    BNReader<double> reader;

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int numCPU = sysinfo.dwNumberOfProcessors;

    std::cout << numCPU << std::endl;

    clock_t start = clock();
    reader.loadNetworkFromFile("..\\exampleNetworks\\ARM_properties_net_alt.xdsl", bn);

    auto input = bn->getGraph();
    auto inNodes = input->getNodes();
    std::cout << "Nodi letti: " << inNodes.size() << std::endl;
    for(auto i : inNodes) {
        if(i.getStatuses().size() == 8){
            std::cout << i.getCpt()->getCPTTable().size() << std::endl;
        }
    }

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
