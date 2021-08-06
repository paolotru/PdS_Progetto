#pragma once

#include "graphStructure/Clique.h"
#include "../networks/BayesianNet.h"
#include "../probability/CPT.h"

//struct used to define the data passed with messages between cliques
template <typename T = float>
struct Message {
	CliqueId m_sourceClique;
	CPT<T> m_potential;
	bool m_isMessageNew;
	int m_position;

	Message(CliqueId sourceClique) : m_sourceClique(sourceClique), m_isMessageNew(true), m_position(-1){
	}
};

struct DisjointSet {
	CliqueId m_parentClique;
	int m_rank;

	DisjointSet() : m_parentClique(-1), m_rank(0) {}

	DisjointSet(CliqueId parentClique) : m_parentClique(parentClique), m_rank(0) {}
};

typedef std::set<Separator> SeparatorSet;

template <typename T = float>
using Messages = std::vector<std::shared_ptr<Message<T>>>;

template <class T = float>
class JunctionTree {
public:
	JunctionTree() {}

	//constructor
	//receives the list of cliques found and the CPTs to be associated with each clique
	JunctionTree(std::shared_ptr<BayesianNetwork<T>> bn) {
		auto chordalGraph = bn->getChordalGraph();

		m_cliques = chordalGraph->maximalCliqueSet();
		
		m_cliqueMessagesReceived.resize(m_cliques->size());
		m_cliquePotentials.reserve(m_cliques->size());
		m_cliqueSeparators.resize(m_cliques->size());

		findJunctionTree(m_cliques->size());
		findCliquePotential(bn);
	}

	//JunctionTree(const JunctionTree& jt) {}

	//~JunctionTree();

	void addClique(NodeId n1, NodeId n2, CliqueId id) {
		m_cliques->emplace_back(n1, n2, id);
	}

	void addClique(NodeId n1, NodeId n2, NodeId n3, CliqueId id) {
		m_cliques->emplace_back(n1, n2, n3, id);
	}

private:
	bool detectCycle(std::vector<DisjointSet>& nodes, CliqueId clique1, CliqueId clique2) {
		CliqueId root1 = pathCompressionFindParentClique(nodes, clique1);
		CliqueId root2 = pathCompressionFindParentClique(nodes, clique2);

		if (root1 == root2) return true;

		unionDisjointSetsByRank(nodes, root1, root2);
		return false;
	}

	//finds all the separators given the clique list
	void findAllSeparators(SeparatorSet& sepSet) {
		for (auto it = m_cliques->begin(); it != std::prev(m_cliques->end()); it++) {
			for (auto it2 = std::next(it); it2 != m_cliques->end(); it2++) {
				Separator s = it->intersectionWithClique(*it2);

				if (!s.isIntersectionEmpty())
					sepSet.insert(s);
			}
		}
	}

	//uses Kruskal's algorithm to find a maximum-weight clique tree given the sets of cliques and separators
	void findJunctionTree(int nCliques) {
		SeparatorSet sepSet;

		findAllSeparators(sepSet);

		int count = 0;

		std::vector<DisjointSet> nodes;
		nodes.resize(nCliques);

		for (int i = 0; i < nodes.size(); i++) {
			nodes[i].m_parentClique = i;
		}

		for (auto& separator : sepSet) {
			if (!detectCycle(nodes, separator.getFirst(), separator.getSecond())) {
				m_cliqueSeparators.at(separator.getFirst()).insert(separator);
				m_cliqueSeparators.at(separator.getSecond()).insert(separator);

				count++;
				if (count == m_cliques->size() - 1) return;
			}
		}

		int k = 0;
	}

	//given the map node-CPT, finds the potential of each clique
	void findCliquePotential(std::shared_ptr<BayesianNetwork<T>> bn) {
		std::vector<bool> cptsAlreadyInserted;
		cptsAlreadyInserted.resize(bn->getNumberOfVariables(), false);

		std::map<NodeId, CliqueId> potentialCliqueForCPT;

		for (auto& clique : *m_cliques) {
			auto nodes = clique.getNodes();

			CPTs<T> cliqueCPTs;

			for (auto& node : nodes) {
				const auto variables = bn->getCPT(node).getVariables();

				if (!cptsAlreadyInserted[node]) {
					if (variables.size() == 1) {
						potentialCliqueForCPT.emplace(node, clique.getCliqueId());
					}
					else {

						bool variablePresent = true;

						for (auto it = variables.begin(); it != std::prev(variables.end()); it++) {
							if (nodes.find(it->m_id) == nodes.end()) variablePresent = false;
						}

						if (variables.size() > 1 && variablePresent && !cptsAlreadyInserted[node]) {
							for (auto it = variables.begin(); it != std::prev(variables.end()); it++) {
								if (potentialCliqueForCPT.find(it->m_id) != potentialCliqueForCPT.end()) {
									cliqueCPTs.push_back(bn->getCPT(it->m_id));
									potentialCliqueForCPT.erase(it->m_id);
									cptsAlreadyInserted[it->m_id] = true;
								}
							}
							cliqueCPTs.push_back(bn->getCPT(node));
							cptsAlreadyInserted[node] = true;
						}
					}
				}
			}

			m_cliquePotentials.push_back(cliqueCPTs);
		}

		for (auto& nodeClique : potentialCliqueForCPT) {
			m_cliquePotentials.at(nodeClique.second).push_back(bn->getCPT(nodeClique.first));
		}
	}

	CliqueId findParentClique(std::vector<DisjointSet>& nodes, CliqueId node) {
		CliqueId parent = nodes[node].m_parentClique;

		if (parent == node) return node;
		else return findParentClique(nodes, parent);
	}

	CliqueId pathCompressionFindParentClique(std::vector<DisjointSet>& nodes, CliqueId node) {
		CliqueId parent = nodes[node].m_parentClique;
		if (parent == node) return node;
		else {
			CliqueId parentNode = findParentClique(nodes, parent);
			nodes[node].m_parentClique = parentNode;
			return parentNode;
		}
	}

	void unionDisjointSetsByRank(std::vector<DisjointSet>& nodes, CliqueId root1, CliqueId root2) {
		if (nodes[root1].m_rank < nodes[root2].m_rank) {
			nodes[root1].m_parentClique = root2;
		}
		else {
			nodes[root2].m_parentClique = root1;
			if (nodes[root1].m_rank == nodes[root2].m_rank)
				nodes[root1].m_rank++;
		}
	}

	//updates the adjacency set of each clique with the separator chosen by Kruskal's algorithm
	void updateCliqueSeparators(Clique& clique, const Separator& sep) {
		
	}

	std::shared_ptr<std::vector<Clique>> m_cliques;
	std::vector<SeparatorSet> m_cliqueSeparators;
	std::vector<Messages<T>> m_cliqueMessagesReceived;
	std::vector<CPTs<T>> m_cliquePotentials;

	template <typename T>
	friend class LazyPropagation;
};