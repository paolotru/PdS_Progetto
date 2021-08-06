#include "src/networks/BayesianNet.h"
#include "src/tools/ThreadPoolManager.h"
#include "src/networks/io/BNReader.h"
#include "src/inference/LazyPropagation.h"
#include "src/learning/io/CSVReader.h"

using namespace std;

int main()
{
	//auto started = std::chrono::high_resolution_clock::now();

	ThreadPoolManager::initialize(16);

	//for (auto i = 0; i < 50; i++) {

	//auto bn = std::make_shared<BayesianNetwork<float>>();

	//BNReader<float> reader;

	//reader.loadNetworkFromFile("..\\..\\..\\..\\Bayesian Network\\exampleNetworks\\barley.xdsl", bn);

	//std::vector<Evidence> evidence;

	//NodeId target = bn->idFromName("nmin");

	//LazyPropagation<float> lz(bn, evidence, 6, target);

	//auto res = lz.posteriorMarginal(target);

	//res.toString();

	//std::cout << i << std::endl;
//}

//auto done = std::chrono::high_resolution_clock::now();

//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(done - started).count();

	auto csvData = std::make_shared<CSVDataStructure<float>>();

	CSVReader<float> reader;

	reader.loadCSVDataFromFile("..\\..\\..\\..\\Bayesian Network\\exampleDataSets\\alarm.csv", csvData);

	return 0;
}