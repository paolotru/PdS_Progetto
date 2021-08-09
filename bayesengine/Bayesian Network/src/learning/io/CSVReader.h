#include "base/rapidcsv.h"
#include "CSVDataStructure.h"

#include <memory>

template<class T = float>
class CSVReader {
public:
	CSVReader(){}

	void loadCSVDataFromFile(const std::string& fileName, std::shared_ptr<CSVDataStructure<T>> csvData, const char& separator = ';') {
		auto doc = std::make_shared<rapidcsv::Document>(fileName, rapidcsv::LabelParams(0, 0), rapidcsv::SeparatorParams(separator));

		csvData->m_variableNames = doc->GetColumnNames();
		csvData->m_mappedValues.resize(csvData->m_variableNames.size());
		csvData->m_CSVData.resize(csvData->m_variableNames.size());

		for (unsigned int i = 0; i < doc->GetRowCount(); i++) {
			for (unsigned int j = 0; j < doc->GetColumnCount(); j++) {
				std::string value = doc->GetCell<std::string>(j, i);

				unsigned int mappedValue = tryInsertValueAndGetMappedValue(csvData, value, j);

				updateCSVData(csvData, j, mappedValue);
			}
		}
	}

private:

	unsigned int tryInsertValueAndGetMappedValue(std::shared_ptr<CSVDataStructure<T>> csvData, const std::string& value, unsigned int variableId) {
		auto it = csvData->m_mappedValues.at(variableId).find(value);
		
		if (it != csvData->m_mappedValues.at(variableId).end())
			return it->second;
		else {
			csvData->m_mappedValues.at(variableId).emplace_hint(csvData->m_mappedValues.at(variableId).end(), value, csvData->m_mappedValues.at(variableId).size());
			return csvData->m_mappedValues.at(variableId).size() - 1;
		}
	}

	void updateCSVData(std::shared_ptr<CSVDataStructure<T>> csvData, unsigned int variableId, unsigned int mappedValue) {
		int size = csvData->m_CSVData.at(variableId).size();
		
		if (size > 0) {
			if (csvData->m_CSVData.at(variableId).at(size - 1).m_value == mappedValue)
				csvData->m_CSVData.at(variableId).at(size - 1).m_repetitions++;
			else
				csvData->m_CSVData.at(variableId).emplace_back(mappedValue);
		}
		else {
			csvData->m_CSVData.at(variableId).emplace_back(mappedValue);
		}
	}
};