#include <vector>
#include <map>
#include <string>

struct RunLengthItem {
	unsigned int m_value;
	unsigned int m_repetitions;

	RunLengthItem(unsigned int value) : m_value(value), m_repetitions(1){}
};

template <typename T = float>
struct CSVDataStructure {
	//vector containing the names of each variable
	std::vector<std::string> m_variableNames;
	//vector containing for each variable, the mapping state name - index
	std::vector<std::map<std::string, unsigned int>> m_mappedValues;
	//matrix containing the data of the csv file using run length coding to save space
	std::vector<std::vector<RunLengthItem>> m_CSVData;

	CSVDataStructure(){}
};