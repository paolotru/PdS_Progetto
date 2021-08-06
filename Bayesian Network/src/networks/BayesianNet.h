#pragma once

#include <iostream>

#include "../graphs/DAG.h"
#include "../tools/VariableNodeMap.h"
#include "../tools/ThreadPoolManager.h"
#include "../probability/CPT.h"

template <class T = float>
class BayesianNetwork {
public:

	//constructor
	BayesianNetwork() : m_bn(std::make_shared<DAG>()) {

	}

	//copy constructor
	BayesianNetwork(const BayesianNetwork& bn) {
		m_bn = bn.m_bn;
	}

	//destructor
	~BayesianNetwork() {
		m_bn.reset();
	}

	//copy operator
	BayesianNetwork& operator=(const BayesianNetwork& bn) {
		if (this != &bn) {
			m_bn = bn.m_bn;
		}

		return *this;
	}

	//boolean operator
	bool operator==(const BayesianNetwork& bn) const {
		if (*m_bn == *(bn.m_bn)) return true;
		else false;
	}

	//adds and arc to the graph
	void addArc(NodeId node1, NodeId node2) {
		m_bn->addArc(node1, node2);
	}

	//adds arcs to the graphs using the dependencies obtained from the CPTS
	void addArcsFromCPTs() {
		for (auto& CPT: m_cpt) {
			std::vector<VarStates> parents = CPT.getVariables();

			for (auto it2 = parents.begin(); it2 != std::prev(parents.end()); it2++) {
				m_bn->addArc(it2->m_id, CPT.getCPTId());
			}
		}

		m_bn->setNumberOfNodes(m_vnm.getNumberOfVariables());
	}

	//void addProbabilities(const NodeId n, const std::vector<int>& var, const std::vector<StateProb<T>>& probs) {
	//	m_cpt.find(n)->second.addProbability(var, probs);
	//}

	//adds an empty variable to the graph, wihtout connecting it.
	//used for debug purposes
	void addVariable(const std::string& variableName) {
		NodeId id = m_vnm.addVariable(variableName);
		CPT<T> cpt;
		m_cpt.push_back(cpt);
	}

	//adds a variable to the graph, without connecting it
	//used for debug purposes
	void addVariable(const std::string& variableName, CPT<T> cpt) {
		m_vnm.addVariable(variableName);

		for (auto &CPT : m_cpt) {
			if (CPT.isCPTDataEqualTo(cpt)) {
				cpt.duplicateCPT(CPT);
				m_nodeWithSameCPT.emplace(cpt.getCPTId(), CPT.getCPTId());
				break;
			}
		}

		m_cpt.push_back(cpt);
	}

	//adds a variable to the graph, using the same CPT of another variable, without connecting it
	//used for debug purposes
	void addVariable(NodeId source, const std::string& variableName) {
		m_vnm.addVariable(variableName);
		NodeId id = m_vnm.idFromName(variableName);
		CPT<T> sourceCPT = m_cpt.at(source);
		m_cpt.push_back(sourceCPT);

		if (m_nodeWithSameCPT.find(source) != m_nodeWithSameCPT.end()) {
			m_nodeWithSameCPT.emplace(id, m_nodeWithSameCPT.find(source)->second);
		}
		else {
			m_nodeWithSameCPT.emplace(id, source);
		}
	}

	//wrapper function that decides if the check for sparse CPTs can be done in parallel or not
	void checkSparseCPTs() {
		if (ThreadPoolManager::isThreadPoolInitialized() > 0)
			checkSparseCPTsParallel();
		else
			checkSparseCPTsUnparallel();
	}

	void checkSparseCPTsUnparallel() {
		for (auto &CPT : m_cpt) {
			if (m_nodeWithSameCPT.find(CPT.getCPTId()) == m_nodeWithSameCPT.end()) {
				CPT.checkSparseCPT();
			}
		}
	}

	void checkSparseCPTsParallel() {
		auto it = m_cpt.begin();

		auto lambda = [&](int i) {
			if (m_nodeWithSameCPT.find(std::next(it, i)->getCPTId()) == m_nodeWithSameCPT.end()) {
				std::next(it, i)->checkSparseCPT();
			}
		};

		ThreadPoolManager::getInstance()->parallelize_loop(0, (int)m_cpt.size() - 1, lambda);
	}

	void checkSparseCPT(const NodeId n) {
		m_cpt.at(n).checkSparseCPT();
	}

	//onyl for debug purposes at the moment
	//void displayCPT(const NodeId n) const {
	//	m_cpt.find(n)->second.displayCPT();
	//}

	//returns the CPT of node n
	CPT<T> getCPT(const NodeId n) {
		return m_cpt.at(n);
	}

	//wrapper function that returns the set of nodes that are not barren,  given the set of target variables and the evidences introduced
	std::shared_ptr<NodeSet> getNotBarrenVariables(NodeId targetVariable, const std::vector<Evidence>& evidences) {
		return m_bn->getNotBarrenVariables(targetVariable, evidences);
	}

	//wrapper function for converting the DAG of the bayesian network into the corresponding moralized graph
	std::shared_ptr<UndirectedGraph> getChordalGraph() {
		auto undGraph = m_bn->moralizeDAG();
		undGraph->triangulateGraph();
		return undGraph;
	}

	int getNumberOfVariables() {
		return m_vnm.getNumberOfVariables();
	}

	//wrapper function that returns the number of states of a given variable node
	int getVariableStatesNum(const NodeId node) {
		return m_cpt.at(node).getStatesNum();
	}

	bool hasChildren(const NodeId node) {
		return m_bn->hasChildren(node);
	}

	//returns the id of a variable given its name
	NodeId idFromName(const std::string& variableName) {
		return m_vnm.idFromName(variableName);
	}

	//wrapper function that checks if a NodeSet X is dSeparated from a NodeSet J given a NodeSet L (the nodes in L are the evidences introduced)
	//returns a vector of bool, where each item is the result for each node in NodeSet x
	void isDSeparated(std::set<NodeSeparated>& X, NodeSet& J, NodeSet& L) {
		m_bn->isDSeparated(X, J, L);
	}

	//wrapper function that removes a node from the DAG
	void removeArc(NodeId node1, NodeId node2) {
		m_bn->removeArc(node1, node2);
	}

	//wrapper function for changing the probability of a combination of states inside the CPT of node n
	void updateProbabilities(const NodeId n, const std::vector<int>& var, T prob) {
		m_cpt.at(n).updateProbabilities(var, prob);
	}

private:
	std::shared_ptr<DAG> m_bn;
	CPTs<T> m_cpt;
	std::map<NodeId, NodeId> m_nodeWithSameCPT;
	VariableNodeMap m_vnm;
};