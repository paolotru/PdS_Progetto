#pragma once

#include <map>
#include <set>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <mutex>
#include <functional>
#include <algorithm>
#include <iomanip>

#include "../graphs/graphStructure/GraphElements.h"
#include "../tools/base/COW.h"

#define SPARSETHRESHOLD 0.7

//struct used to store information about the value of the variable and its probability
template <typename T = float>
struct StateProb {
	int m_state;
	T m_prob;

	StateProb(int state, T prob) : m_state(state), m_prob(prob) {}

	bool operator==(const StateProb<T>& sp) const {
		return m_state == sp.m_state && m_prob == sp.m_prob;
	}

	StateProb<T> operator+(const StateProb<T>& sp) const {
		return StateProb<T>(m_state, m_prob + sp.m_prob);
	}

	StateProb<T>& operator+=(const StateProb<T>& sp) {
		m_prob += sp.m_prob;
		return *this;
	}

	StateProb<T>& operator/=(const int div) {
		m_prob /= div;
		return *this;
	}
};

//struct used to store information about the id of a variable its number of states
struct VarStates {
	NodeId m_id;
	int m_nStates;
	bool m_found;

	VarStates() : m_id(-1), m_nStates(0), m_found(false) {}

	VarStates(NodeId id, int nStates) : m_id(id), m_nStates(nStates), m_found(false) {}

	bool operator<(const VarStates& vs) const {
		return m_id < vs.m_id;
	}

	bool operator==(const VarStates& vs) const {
		return m_id == vs.m_id && m_nStates == vs.m_nStates;
	}

	bool operator!=(const VarStates& vs) const {
		return m_id != vs.m_id || m_nStates != vs.m_nStates;
	}
};

//struct used to store information about the evidence introduced in the network.
//m_id: id of the variable
//m_state: state to which the variable is istantiated
struct Evidence {
	NodeId m_id;
	int m_state;

	Evidence(NodeId id, int state) : m_id(id), m_state(state) {}

	bool operator<(const Evidence& e) const {
		return m_id < e.m_id;
	}
};

//struct used to store the probability distribution of the CPT
template <typename T = float>
struct CPTData {
	std::vector<T> m_cpt;
	std::map<int, int> m_combinationsRemained;
	T m_sparseProb;
	int m_CPTSize;
	int m_sparseCount;
};

template <class T = float>
class CPT : private COW<CPTData<T>> {

public:

	//default CPT constructor without parameters
	CPT() : m_var(-1), m_instantiated(false) {
		construct();

		ptr()->m_sparseProb = -1;
		ptr()->m_sparseCount = -1;
	}

	//CPT constructor used during reading of source file containing the bayesian network
	CPT(const std::string& name, const NodeId id, const std::vector<std::string>& stateNames) : m_name(name), m_var(id), m_instantiated(false) {
		construct();

		ptr()->m_sparseProb = -1;
		ptr()->m_sparseCount = -1;

		for (int i = 0; i < stateNames.size(); i++) {
			m_valueStateName.emplace(i, stateNames[i]);
		}
	}

	//CPT constructor used to initialize the CPT's info but not the probability distribution
	CPT(const std::map<int, std::string>& valueStates, T sparseProb, int sparseCount, int CPTSize) :
		m_var(-1), m_valueStateName(valueStates), m_instantiated(false) {
		construct();

		ptr()->m_sparseProb = sparseProb;
		ptr()->m_sparseCount = sparseCount;
		ptr()->m_CPTSize = CPTSize;
	}

	//CPT constructor used to compute the potential of multiple CPTs.
	//It receives the evidences introduced in the bayesian network in order to calculate the combinations of states for each variable
	CPT(std::vector<CPT<T>>& cpts, NodeId variable = -1, bool evaluateImmediately = false) : m_var(variable), m_instantiated(false) {
		construct();

		ptr()->m_sparseProb = -1;
		ptr()->m_sparseCount = -1;

		for (auto& CPT : cpts) {
			for (auto& evidence : CPT.m_evidences)
				m_evidences.insert(evidence);
		}

		findVariablesOrderFromCPTs(cpts);

		if (variable > -1) {
			for (auto& CPT : cpts) {
				if (CPT.m_var == variable) {
					m_valueStateName = CPT.m_valueStateName;
					m_name = CPT.m_name;
				}
			}
		}

		if (evaluateImmediately)
			evaluateCPT(cpts);
	}

	~CPT() {}

	//boolean operator for comparison of CPTs
	bool operator==(const CPT<T>& c) const {
		return m_var == c.m_var && m_variablesOrder == c.m_variablesOrder && m_evidences == c.m_evidences;
	}

	//boolean operator for comparison of CPTs
	bool operator!=(const CPT<T>& c) const {
		return m_var != c.m_var || m_variablesOrder != c.m_variablesOrder || m_evidences != c.m_evidences;
	}

	//uses the same data contained in c for the CPTData of this CPT
	void duplicateCPT(CPT& c) {
		duplicate(c);
	}

	//adds variables order to the CPT
	void addVariablesOrder(std::vector<VarStates>& variablesOrder) {
		m_variablesOrder = std::move(variablesOrder);
	}

	void addProbabilities(std::vector<T>& probs) {
		clone_if_needed();

		ptr()->m_cpt = std::move(probs);
	}

	//adds probability distribution for a combination of variables to the CPT
	void addProbability(const T prob) {
		clone_if_needed();

		ptr()->m_cpt.push_back(prob);
	}

	//adds resulting states to the CPT
	void addResultingStates(std::vector<std::string>& resultingStates) {
		m_resultingStates = std::move(resultingStates);
	}

	//checks if the base CPT is sparse
	void checkSparseCPT() {
		std::map<T, int> valueCount;

		if (!ptr()->m_cpt.empty()) {

			ptr()->m_CPTSize = ptr()->m_cpt.size();

			for (auto& value : ptr()->m_cpt) {
				auto p = valueCount.find(value);
				if (p != valueCount.end()) {
					p->second++;

					//converts the matrix into a sparse representation if it is sparse
					if (p->second > (ptr()->m_CPTSize * SPARSETHRESHOLD)) {
						ptr()->m_sparseProb = p->first;
						ptr()->m_sparseCount = p->second;
						convertCPTToSparseCPT();
						return;
					}
				}
				else
					valueCount.emplace(value, 1);
			}
		}
	}

	//evaluates the CPT
	void evaluateCPT(std::vector<CPT<T>>& cpts) {
		int numOfCombinations = getNumberOfCombinations();

		int num_tasks = 0;

		//lambda function used for parallelizing the evaluation of the new CPT
		auto lambdaLoop = [&](int l) {
			std::vector<int> combination;
			combination.resize(m_variablesOrder.size());

			int blockSize = numOfCombinations / num_tasks;

			int end = (l == num_tasks - 1) ? numOfCombinations : (l + 1) * blockSize;

			//finds the starting position of this block of combinations
			findPositionCombination(l * blockSize, combination);

			calculateCPTData(cpts, l * blockSize, end, combination);
		};

		ptr()->m_cpt.resize(numOfCombinations);

		//if the number of combinations to be calculated are >= 10000, tries to parallelize the evaluation
		if (numOfCombinations >= 10000 && ThreadPoolManager::isThreadPoolInitialized()) {
			num_tasks = ThreadPoolManager::reserve_tasks_for_loop(0, numOfCombinations, 5000);

			if (num_tasks > 1)
				ThreadPoolManager::parallelizeLoopWithReservedThreads((int)0, num_tasks - 1, lambdaLoop, num_tasks);
		}

		//if the evaluation cannot be parallelized then it is done in the current thread
		if (numOfCombinations < 10000 || num_tasks <= 1) {

			std::vector<int> combination;

			getInitialCombination(combination);

			calculateCPTData(cpts, 0, numOfCombinations, combination);
		}

		checkSparseCPT();
	}

	//returns the CPT id
	int getCPTId() {
		return m_var;
	}

	//returns the list of variables of the CPT
	std::vector<VarStates> getVariables() {
		return m_variablesOrder;
	}

	//returns the list of variables of the CPT
	std::vector<NodeId> getVariablesIds() {
		std::vector<NodeId> variablesIds;
		for (auto& var : m_variablesOrder) {
			variablesIds.push_back(var.m_id);
		}
		return variablesIds;
	}

	//finds the position of a variable inside the vector of variables
	int getVariablePosition(const NodeId var) {
		for (int i = 0; i < m_variablesOrder.size(); i++) {
			if (m_variablesOrder[i].m_id == var) return i;
		}
		return -1;
	}

	//returns the probability of a combination of variables
	T getProbability(std::vector<int>& combination) const {
		int pos = findCombinationPosition(combination);

		if (ptr()->m_sparseProb > -1) {
			auto it = ptr()->m_combinationsRemained.find(pos);

			if (it == ptr()->m_combinationsRemained.end()) return ptr()->m_sparseProb;
			else return ptr()->m_cpt[it->second];
		}
		else
			return ptr()->m_cpt[pos];
	}

	//returns the probability distribution of a combination of variables
	std::vector<StateProb<T>> getProbabilityDistribution(std::vector<int>& variablesCombination) const {
		std::vector<int> combination(variablesCombination);
		combination.push_back(0);

		std::vector<StateProb<T>> probDistribution;

		while (combination[m_variablesOrder.size() - 1] < m_variablesOrder[m_variablesOrder.size() - 1].m_nStates) {
			int pos = findCombinationPosition(combination);

			if (ptr()->m_sparseProb > -1) {
				auto it = ptr()->m_combinationsRemained.find(pos);

				if (it == ptr()->m_combinationsRemained.end()) probDistribution.emplace_back(combination[m_variablesOrder.size() - 1], ptr()->m_sparseProb);
				else probDistribution.emplace_back(combination[m_variablesOrder.size() - 1], ptr()->m_cpt[it->second]);
			}
			else
				probDistribution.emplace_back(combination[m_variablesOrder.size() - 1], ptr()->m_cpt[pos]);

			combination[m_variablesOrder.size() - 1]++;
		}

		return probDistribution;
	}

	//returns the sparse probability
	T getSparseProb() const {
		return ptr()->m_sparseProb;
	}

	int getStatesNum() const {
		return m_valueStateName.size();
	}

	//instantiate the variable to the evidence introduced.
	//returns a new CPT instantiated to evidence e
	CPT<T> instantiateVariable(const Evidence e) {
		clone_if_needed();

		int variablePos = getVariablePosition(e.m_id);

		if (variablePos != -1) {

			std::map<int, std::string> instantiatedMap;

			//if the variable to be instantiated is the conditioned variable, then all the states are removed except for the one instantiated
			if (e.m_id == m_var) {
				instantiatedMap.emplace(e.m_id, m_valueStateName.find(e.m_id)->second);
			}

			CPT<T> instantiatedCPT(instantiatedMap, ptr()->m_sparseProb, ptr()->m_sparseCount, ptr()->m_CPTSize / m_valueStateName.size());

			instantiatedCPT.m_variablesOrder = m_variablesOrder;
			instantiatedCPT.m_variablesOrder[variablePos].m_nStates = 1;
			instantiatedCPT.m_evidences.emplace(e.m_id, e.m_state);

			for (auto& evidence : m_evidences) {
				instantiatedCPT.m_evidences.insert(evidence);
			}

			int nStatesOfFollowingVariables = 1;

			//finds the number of combinations of variables following the one to be marginalized inside the variables order
			for (int i = variablePos + 1; i < m_variablesOrder.size(); i++) {
				nStatesOfFollowingVariables *= m_variablesOrder[i].m_nStates;
			}

			int nStatesOfPrecedingVariables = 1;

			//finds the number of combinations of variables preceding the one to be marginalized inside the variables order
			for (int i = 0; i < variablePos; i++) {
				nStatesOfPrecedingVariables *= m_variablesOrder[i].m_nStates;
			}

			int nCombinationsToSkip = nStatesOfFollowingVariables * m_variablesOrder[variablePos].m_nStates;

			int initialPos = nStatesOfFollowingVariables * e.m_state;

			//stores the new position pf the remaining combinations in case the original CPT is sparse
			int newPos = 0;

			for (int i = 0; i < nStatesOfPrecedingVariables; i++) {
				int pos = initialPos;

				for (int j = 0; j < nStatesOfFollowingVariables; j++) {
					if (ptr()->m_sparseProb > -1) {
						auto it = ptr()->m_combinationsRemained.find(pos + j);

						if (it != ptr()->m_combinationsRemained.end()) {
							instantiatedCPT.ptr()->m_cpt.push_back(ptr()->m_cpt[it->second]);
							instantiatedCPT.ptr()->m_combinationsRemained.emplace(pos + j, newPos);
						}

						newPos++;
					}
					else
						instantiatedCPT.ptr()->m_cpt.push_back(ptr()->m_cpt[pos + j]);
				}

				initialPos += nCombinationsToSkip;
			}

			instantiatedCPT.m_resultingStates = m_resultingStates;
			instantiatedCPT.m_instantiated = true;

			//if the original was not sparse, checks is the new one is sparse
			if (ptr()->m_sparseProb == -1)
				instantiatedCPT.checkSparseCPT();


			return instantiatedCPT;
		}

		return *this;
	}

	//checks whether the CPTs contains the same distribution probabilities although they depend on different variables
	bool isCPTDataEqualTo(const CPT<T>& cpt) {
		if (!ptr()->m_cpt.empty() && floatingPointCompare(ptr()->m_sparseProb, -1))
			return ptr()->m_cpt == cpt.ptr()->m_cpt;
		else if (!ptr()->m_cpt.empty() && ptr()->m_sparseProb > -1)
			return ptr()->m_sparseProb == cpt.ptr()->m_sparseProb && ptr()->m_cpt == cpt.ptr()->m_cpt;
		else
			return floatingPointCompare(ptr()->m_sparseProb, cpt.ptr()->m_sparseProb);
	}

	//checks whether the CPT is empty or not
	bool isEmpty() {
		return m_variablesOrder.empty();
	}

	//checks whether the CPT has been instantiated or not
	bool isInstantiated() {
		return m_instantiated;
	}

	//checks whether this potential is equal to potential that will be obtained after marginalizing the nodes passed in nodesToMarginalize
	bool isCPTEqualToNewCPT(CPT<T>& cpt, const std::vector<NodeId>& nodesToMarginalize) {
		auto variablesOrder = cpt.m_variablesOrder;
		for (auto& varToMarginalize : nodesToMarginalize) {
			for (auto it = variablesOrder.begin(); it != variablesOrder.end();) {
				if (it->m_id == varToMarginalize) {
					it = variablesOrder.erase(it);
					break;
				}
				else
					it++;
			}
		}

		return variablesOrder == m_variablesOrder;
	}

	//checks whether the CPT is unitary or not
	bool isUnitary() {
		return floatingPointCompare(ptr()->m_sparseProb, 1) && ptr()->m_cpt.empty();
	}

	bool isVariableOrderEqualTo(const std::vector<VarStates>& variableOrder) {
		return m_variablesOrder == variableOrder;
	}

	//returns a new CPT where the variable var has been marginalized out
	CPT<T> marginalizeVariable(const NodeId var) {
		clone_if_needed();

		int varPos = getVariablePosition(var);

		//if the variable to be marginalized is present marginalizes the CPT, else returns this CPT
		if (varPos != -1) {

			//if the marginalized variable is the conditioned variable, the resulting CPT is unitary if it is not instantied
			if (var == m_var && m_evidences.find(var) == m_evidences.end()) {
				return CPT<T>(m_valueStateName, 1, 0, 0);
			}
			else {

				CPT<T> marginalizedCPT(m_valueStateName, -1, 0, ptr()->m_CPTSize);

				std::vector<VarStates> newParentsOrder;

				//copies the CPT's parents execpt var into a new vector
				std::copy(m_variablesOrder.begin(), m_variablesOrder.begin() + varPos, std::back_inserter(newParentsOrder));
				std::copy(m_variablesOrder.begin() + varPos + 1, m_variablesOrder.end(), std::back_inserter(newParentsOrder));

				marginalizedCPT.m_variablesOrder = std::move(newParentsOrder);
				marginalizedCPT.m_evidences = m_evidences;

				VarStates variable = m_variablesOrder[varPos];

				int nStatesOfFollowingVariables = 1;

				//finds the number of combinations of variables following the one to be marginalized inside the variables order
				for (int i = varPos + 1; i < m_variablesOrder.size(); i++) {
					nStatesOfFollowingVariables *= m_variablesOrder[i].m_nStates;
				}

				int nStatesOfPrecedingVariables = 1;

				//finds the number of combinations of variables preceding the one to be marginalized inside the variables order
				for (int i = 0; i < varPos; i++) {
					nStatesOfPrecedingVariables *= m_variablesOrder[i].m_nStates;
				}

				int initialPos = 0;

				//moves initialPos to the next combination of preceding variables when all 
				//combinations of following variables have been explored
				for (int l = 0; l < nStatesOfPrecedingVariables; l++) {

					initialPos = nStatesOfFollowingVariables * m_variablesOrder[varPos].m_nStates * l;

					//explores all combinations of following variables
					for (int k = 0; k < nStatesOfFollowingVariables; k++) {

						int pos = initialPos;

						pos += k;

						T prob = 0;

						for (int j = 0; j < variable.m_nStates; j++) {
							if (ptr()->m_sparseProb > -1) {
								auto it = ptr()->m_combinationsRemained.find(pos);

								if (it == ptr()->m_combinationsRemained.end()) prob += ptr()->m_sparseProb;
								else prob += ptr()->m_cpt[it->second];
							}
							else
								prob += ptr()->m_cpt[pos];

							pos += nStatesOfFollowingVariables;
						}

						marginalizedCPT.ptr()->m_cpt.push_back(roundValue(prob));
					}

				}

				marginalizedCPT.m_resultingStates = m_resultingStates;

				marginalizedCPT.checkSparseCPT();

				return marginalizedCPT;
			}
		}
		else {
			this;
		}

	}

	void setCPTId(const NodeId id) {
		m_var = id;
	}

	//updates probability distribution of a combination of variables
	void updateProbabilities(const std::vector<int>& combination, const T prob) {
		clone_if_needed();

		int pos = findCombinationPosition(combination);

		if (ptr()->m_sparseProb > -1) {
			int i = 0;
			for (auto& posRemaining : ptr()->m_combinationsRemained) {
				if (posRemaining.first > pos) {
					auto res = ptr()->m_combinationsRemained.emplace(pos, i);

					if (res.second) {
						ptr()->m_cpt.insert(ptr()->m_cpt.begin() + i, prob);
					}
				}
				i++;
			}
		}
		else
			ptr()->m_cpt[pos] = prob;
	}

	//prints the CPT
	void toString() const {
		if (ptr()->m_sparseProb > -1) {
			for (auto& nameState : m_valueStateName) {
				auto it = ptr()->m_combinationsRemained.find(nameState.first);
				if (it == ptr()->m_combinationsRemained.end())
					std::cout << nameState.second << ": " << std::setprecision(5) << ptr()->m_sparseProb << std::endl;
				else
					std::cout << nameState.second << ": " << std::setprecision(5) << ptr()->m_cpt[it->second] << std::endl;
			}

		}
		else {
			for (auto& nameState : m_valueStateName) {
				std::cout << nameState.second << ": " << std::setprecision(5) << ptr()->m_cpt[nameState.first] << std::endl;
			}
		}
	}

private:
	void calculateCPTData(std::vector<CPT<T>>& CPTs, int firstIndex, int lastIndex, std::vector<int>& combination) {
		std::vector<int> partialCombination;
		partialCombination.resize(m_variablesOrder.size());

		//for each combination, computes the product of the corresponding probabilities from each parent CPT
		for (int i = firstIndex; i < lastIndex; i++) {
			T prob = 1;

			for (auto& CPT : CPTs) {
				//creates the partial combination needed to get the probability from the parent CPT it2
				for (int i = 0; i < CPT.m_variablesOrder.size(); i++) {

					int pos = getVariablePosition(CPT.m_variablesOrder[i].m_id);

					partialCombination[i] = combination[pos];
				}

				prob *= CPT.getProbability(partialCombination);

				if (floatingPointCompare(prob, 0)) break;
			}

			ptr()->m_cpt[i] = roundValue(prob);

			getNextCombination(combination);
		}
	}

	//converts the CPT into a sparse representation
	void convertCPTToSparseCPT() {
		std::vector<int> combination;

		getInitialCombination(combination);

		auto itHint = ptr()->m_combinationsRemained.begin();

		auto newCPT = std::make_shared<std::vector<T>>();
		newCPT->reserve(ptr()->m_cpt.size() * (1 - SPARSETHRESHOLD) + 1);

		int j = 0;
		for (int i = 0; i < ptr()->m_cpt.size(); i++) {
			if (!floatingPointCompare(ptr()->m_cpt[i], ptr()->m_sparseProb)) {
				newCPT->push_back(ptr()->m_cpt[i]);
				ptr()->m_combinationsRemained.emplace_hint(itHint, i, j);
				itHint = ptr()->m_combinationsRemained.end();
				j++;
			}

			getNextCombination(combination);
		}

		ptr()->m_cpt = std::move(*newCPT);
	}

	//finds the combination of variables state at position pos in the CPT
	void findPositionCombination(int pos, std::vector<int>& combination) {
		int base = 1;

		for (int i = 1; i < m_variablesOrder.size(); i++) {
			base *= m_variablesOrder[i].m_nStates;
		}

		for (int i = 1; i < m_variablesOrder.size(); i++) {
			combination[i - 1] = pos / base;
			pos = pos % base;
			base /= m_variablesOrder[i].m_nStates;
		}

		combination[m_variablesOrder.size() - 1] = pos;
	}

	//calculate the index at which the combination is stored in the CPT
	int findCombinationPosition(const std::vector<int>& combination) const {
		int pos = 0;

		int base = 1;

		for (int i = m_variablesOrder.size() - 1; i > -1; i--) {
			pos += (combination[i]) * base;

			base *= m_variablesOrder[i].m_nStates;
		}

		return pos;
	}

	bool floatingPointCompare(const T n1, const T n2) const {
		if (abs(n1 - n2) < 1.0e-8) return true;
		return abs(n1 - n2) < 1.0e-8 * std::max(abs(n1), abs(n2));
	}

	//given the evidences introduced into the network, inserts the initial combination into combination
	void getInitialCombination(std::vector<int>& combination) {
		for (auto& variable : m_variablesOrder) {
			if (m_evidences.find(variable.m_id) == m_evidences.end()) {
				combination.push_back(0);
			}
			else {
				combination.push_back(m_evidences.find(variable.m_id)->second);
			}
		}
	}

	//given the evidences introduced into the network, inserts in combination the next one to the that already stored
	void getNextCombination(std::vector<int>& combination) {
		int i = combination.size() - 1;

		if (!m_evidences.empty()) {
			while (true) {
				if (m_evidences.find(m_variablesOrder[i].m_id) == m_evidences.end()) break;
				else i--;
			}
		}

		int inc = 1;

		for (i; i > -1; i--) {
			if (!m_evidences.empty() && m_evidences.find(m_variablesOrder[i].m_id) == m_evidences.end()) {
				combination[i] += inc;
				combination[i] = combination[i] % m_variablesOrder[i].m_nStates;
				if (combination[i] != 0) break;
			}
			else {
				combination[i] += inc;
				combination[i] = combination[i] % m_variablesOrder[i].m_nStates;
				if (combination[i] != 0) break;
			}
		}
	}

	//given the evidences introduced into the network, returns the number of combinations of states for the current CPT
	int getNumberOfCombinations() {
		int numCombinations = 1;
		for (auto& variable : m_variablesOrder) {
			if (m_evidences.find(variable.m_id) == m_evidences.end())
				numCombinations *= variable.m_nStates;
		}

		return numCombinations;
	}

	//generates the variables order for this CPT taking the parent CPTs as source
	void findVariablesOrderFromCPTs(std::vector<CPT<T>>& CPTs) {
		std::set<VarStates> temp;

		for (auto& CPT : CPTs) {
			for (auto& variable : CPT.m_variablesOrder) {
				auto ins = temp.insert(variable);

				if (ins.second)
					m_variablesOrder.push_back(variable);
			}
		}

		for (auto& var : m_variablesOrder) {
			for (auto& CPT : CPTs) {
				if (CPT.m_var != -1 && var.m_id == CPT.m_var) {
					var.m_found = true;
					break;
				}
				else if(CPT.m_var == -1){
					auto variables = CPT.m_variablesOrder;
					for (auto& var2 : variables) {
						if (var2.m_id == var.m_id && !var.m_found) {
							var.m_found = var2.m_found;
							break;
						}
					}
				}
			}
		}
	}

	float roundValue(float value) {
		float v1 = (int)(value * 1e8 + 0.5);
		return (float)v1 / 1e8;
	}

	std::string m_name;
	NodeId m_var;
	std::vector<VarStates> m_variablesOrder;
	std::vector<std::string> m_resultingStates;
	std::map<int, std::string> m_valueStateName;
	std::map<NodeId, int> m_evidences;
	bool m_instantiated;
};

template <typename T = float>
using CPTs = std::vector<CPT<T>>;