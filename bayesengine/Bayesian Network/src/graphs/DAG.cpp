#include "DAG.h"

#include "../probability/CPT.h"

#include <algorithm>

DAG::DAG() : ArcSet(), m_reverseDAG(nullptr) {
}

DAG::DAG(const DAG& d) : ArcSet(d), m_reverseDAG(nullptr) {
}

DAG::~DAG() {
}

void DAG::addNodesToCheck(NodeId n, std::vector<NodeId>& nodeSetToCheck) {
	auto sourceNode = m_parents.find(n);

	if (sourceNode != m_parents.end()) {
		for (auto &node : sourceNode->second) {
			nodeSetToCheck.push_back(node);
		}
	}
}

std::shared_ptr<DAG> DAG::createReverseDAG() {
	std::call_once(m_reverseDAGCreated, [&]() {
		m_reverseDAG = std::make_shared<DAG>(*this);

		for (auto &arc : m_arcs) {
			m_reverseDAG->addArc(arc.getTail(), arc.getHead());
		}
		});

	return std::make_shared<DAG>(*m_reverseDAG);
}

void DAG::dConnectedNodes(NodeSet& J, std::shared_ptr<ArcPairSet> legalArcPairs, NodeSet& R) {
	std::set<Arc> arcs(m_arcs);
	std::set<Arc> arcsForThisIteration;
	std::set<Arc> arcsFoundForNextIteration;

	NodeId S = getNumberOfNodes();

	for (auto &x : J) {
		Arc a(S, x);
		addArc(a);

		arcsForThisIteration.insert(a);

		R.insert(x);
	}

	int i = 0;

	bool legalPairFound = true;

	while (legalPairFound) {
		legalPairFound = false;
		for (auto it = m_arcs.begin(); it != m_arcs.end();) {

			auto it2 = arcsForThisIteration.begin();

			for (it2; it2 != arcsForThisIteration.end(); it2++) {
				NodeId adj = it->isAdjacent(*it2);
				if (!it->isReverse(*it2) && adj != -1) {
					if (i == 0) {
						arcsFoundForNextIteration.insert(*it);
						legalPairFound = true;
						R.insert(it->getOther(adj));
						it = m_arcs.erase(it);
						break;
					}
					else {
						std::pair<Arc, Arc> aps(*it, *it2);
						std::pair<Arc, Arc> apsRev(*it2, *it);
						auto found = legalArcPairs->find(aps);
						auto foundRev = legalArcPairs->find(apsRev);
						if (found != legalArcPairs->end() || foundRev != legalArcPairs->end()) {
							legalPairFound = true;
							arcsFoundForNextIteration.insert(*it);
							R.insert(it->getOther(adj));
							it = m_arcs.erase(it);
							break;
						}
					}
				}
			}

			if (!legalPairFound || it2 == arcsForThisIteration.end()) it++;
		}

		arcsForThisIteration = std::move(arcsFoundForNextIteration);

		i++;
	}
}

std::shared_ptr<ArcPairSet> DAG::findHeadToHeadArcs(NodeId n) {
	auto headToHeadArcs = std::make_shared<ArcPairSet>();

	auto nParents = m_parents.find(n);

	if (nParents != m_parents.end() && nParents->second.size() > 1) {
		for (auto it = nParents->second.begin(); it != std::prev(nParents->second.end()); it++) {
			Arc a1(*it, n);
			for (auto it2 = std::next(it); it2 != nParents->second.end(); it2++) {
				if (*it2 != *it) {
					Arc a2(*it2, n);
					headToHeadArcs->emplace(a1, a2);
				}
			}
		}
	}

	return headToHeadArcs;
}

std::shared_ptr<ArcPairSet> DAG::findLegalPairsOfArcs(NodeSet& L, std::shared_ptr<NodeSet> descendents) {
	auto legalArcPairs = std::make_shared<ArcPairSet>();

	for (auto &descendent : *descendents) {
		legalArcPairs->merge(*findHeadToHeadArcs(descendent));
	}

	for (auto &l : L) {
		legalArcPairs->merge(*findNotHeadToHeadArcs(l));
	}

	return legalArcPairs;
}

std::shared_ptr<ArcPairSet> DAG::findNotHeadToHeadArcs(NodeId n) {
	auto notHeadToHeadArcs = std::make_shared<ArcPairSet>();

	auto nChildren = m_children.find(n);
	auto nParents = m_parents.find(n);

	if (nChildren != m_children.end()) {
		if (nChildren->second.size() > 1) {
			for (auto it = nChildren->second.begin(); it != std::prev(nChildren->second.end()); it++) {
				Arc a1(n, *it);
				for (auto it2 = std::next(it); it2 != nChildren->second.end(); it2++) {
					if (*it2 != *it) {
						Arc a2(n, *it2);
						notHeadToHeadArcs->emplace(a1, a2);
					}
				}
			}
		}

		if (nParents != m_parents.end()) {
			for (auto &nParent : nParents->second) {
				Arc a1(nParent, n);
				for (auto &nChild : nChildren->second) {
					if (nChild != nParent) {
						Arc a2(n, nChild);
						notHeadToHeadArcs->emplace(a1, a2);
					}
				}
			}
		}
	}

	return notHeadToHeadArcs;
}

void DAG::findParentsOfNode(NodeId X, std::shared_ptr<NodeSet> parents) {

	auto Xparents = m_parents.find(X);

	std::vector<NodeId> parentsToCheck;

	if (Xparents != m_parents.end()) {
		for (auto &Xparent : Xparents->second) {
			addNodesToCheck(Xparent, parentsToCheck);
			parents->insert(Xparent);
		}

		for (int i = 0; i < parentsToCheck.size(); i++) {
			addNodesToCheck(parentsToCheck[i], parentsToCheck);
			parents->insert(parentsToCheck[i]);
		}
	}
}

std::shared_ptr<NodeSet> DAG::findParentsOfNodeset(const NodeSet& L) {
	auto resultNodes = std::make_shared<NodeSet>();

	std::vector<std::future<bool>> resultsFuture;

	for (auto &l : L) {
		resultNodes->insert(l);
		findParentsOfNode(l, resultNodes);
	}

	return resultNodes;
}

void DAG::isDSeparated(std::set<NodeSeparated>& X, NodeSet& J, NodeSet& L) {
	auto descendents = findParentsOfNodeset(L);

	auto GPrime = createReverseDAG();

	auto Fc = findLegalPairsOfArcs(L, descendents);

	NodeSet dConnNodes;

	GPrime->dConnectedNodes(J, Fc, dConnNodes);

	for (auto it = X.begin(); it != X.end(); it++) {
		if (dConnNodes.find(it->m_id) == dConnNodes.end()) {
			NodeSeparated ns(it->m_id, true);
			auto itNext = std::next(it);
			auto itHint = X.erase(it);
			it = itNext;
			X.insert(itHint, std::move(ns));

			if (it == X.end()) return;
		}
	}
}

std::shared_ptr<NodeSet> DAG::getNotBarrenVariables(NodeId targetVariable, const std::vector<Evidence>& evidences) {
	NodeSet nodes;

	nodes.insert(targetVariable);

	for (int i = 0; i < evidences.size(); i++) {
		nodes.insert(evidences[i].m_id);
	}

	return findParentsOfNodeset(nodes);
}

std::shared_ptr<UndirectedGraph> DAG::moralizeDAG() {
	auto undGraph = std::make_shared<UndirectedGraph>();
	for (auto &arc : m_arcs) {
		undGraph->addEdge(arc.getHead(), arc.getTail());
	}

	for (auto &parent : m_parents) {
		if (parent.second.size() > 1) {
			for (auto it2 = parent.second.begin(); it2 != std::prev(parent.second.end()); it2++) {
				for (auto it3 = std::next(it2); it3 != parent.second.end(); it3++) {
					undGraph->addEdge(*it2, *it3);
				}
			}
		}
	}

	return undGraph;
}