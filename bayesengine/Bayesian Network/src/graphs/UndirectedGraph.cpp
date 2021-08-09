#include "UndirectedGraph.h"
#include "../tools/base/thread_pool.hpp"

#include <future>
#include <iostream>

UndirectedGraph::UndirectedGraph() : EdgeSet() {

}

UndirectedGraph::UndirectedGraph(const UndirectedGraph& ug) : EdgeSet(ug) {

}

UndirectedGraph::~UndirectedGraph() {

}

std::shared_ptr<std::set<Edge>> UndirectedGraph::cliqueMinTriang(std::shared_ptr<std::set<Edge>> F) {
	auto FPrime = std::make_shared<std::set<Edge>>(*F);

	auto FVertices = verticesFromEdges(F);

	auto edgeTuvs = std::make_shared<std::map<Edge, std::set<Edge>>>();

	//initialize all Tuvs to the empty set
	for (auto &uv : *FPrime) {
		edgeTuvs->emplace(uv, std::set<Edge>());
	}

	auto GFprime = std::make_shared<UndirectedGraph>(*this);

	GFprime->addEdges(*FPrime);

	//iterate over all edge uv untill there is an empty Tuv
	std::map<Edge, std::set<Edge>>::iterator uv;
	while ((uv = getNextEmptyTuv(edgeTuvs)) != edgeTuvs->end()) {
		//for every edge rs, such that uv belongs to the intersection between the common neighborhood of rs on G+F' and all the vertices of F
		//add uv to Trs
		for (auto &rs : *FPrime) {
			if (uv->first == rs) continue;

			auto CNgfprime = GFprime->commonNeighborhood(rs.getFirst(), rs.getSecond());

			auto CNgfprimeIntersectionVf = verticesIntersection(CNgfprime, FVertices);

			if (checkEdgeEndpointsInsideNodeSet(uv->first, CNgfprimeIntersectionVf)) {
				edgeTuvs->find(rs)->second.insert(uv->first);
			}
		}


		auto CNgfprimeUV = GFprime->commonNeighborhood(uv->first.getFirst(), uv->first.getSecond());

		auto CNgfprimeUVIntersectionVf = verticesIntersection(CNgfprimeUV, FVertices);

		NodeId u = uv->first.getFirst();
		NodeId v = uv->first.getSecond();

		//for each x that belongs to the intersection between the common neighborhood of uv on G+F' and all the vertices of F
		//such that either ux or vx belongs to F' 
		for (auto &x : *CNgfprimeUVIntersectionVf) {

			Edge ux(u, x);

			//if ux belongs to F'
			if (FPrime->find(ux) != FPrime->end()) {
				auto Tux = edgeTuvs->find(ux);

				//if yes then remove any edge vz that belongs to Tux from Tux
				for (auto vz = Tux->second.begin(); vz != Tux->second.end();) {
					if (vz->getFirst() == v || vz->getSecond() == v) {
						vz = Tux->second.erase(vz);
					}
					else {
						vz++;
					}
				}
			}

			Edge vx(v, x);

			//if vx belongs to F'
			if (FPrime->find(vx) != FPrime->end()) {
				auto Tvx = edgeTuvs->find(vx);

				//if yes then remove any edge uz that belongs to Tvx from Tvx
				for (auto uz = Tvx->second.begin(); uz != Tvx->second.end();) {
					if (uz->getFirst() == u || uz->getSecond() == u) {
						uz = Tvx->second.erase(uz);
					}
					else {
						uz++;
					}
				}
			}
		}

		FPrime->erase(uv->first);
		GFprime->removeEdge(uv->first);
		edgeTuvs->erase(uv);
	}

	return FPrime;

}

std::shared_ptr<std::set<Edge>> UndirectedGraph::edgesToCompleteCliqueOfNeighborhood(const NodeId n) {
	auto neighoborhoodN = m_neighbours.at(n);

	auto missingEdges = std::make_shared<std::set<Edge>>();

	if (neighoborhoodN.size() == 0) 
		return missingEdges;

	for (auto it = neighoborhoodN.begin(); it != std::prev(neighoborhoodN.end()); it++) {
		for (auto it2 = std::next(it); it2 != neighoborhoodN.end(); it2++) {
			Edge e(*it, *it2);
			if (!checkIfExists(e))
				missingEdges->insert(e);
		}
	}

	return missingEdges;
}

std::shared_ptr<std::set<Edge>> UndirectedGraph::edgesToCompleteGraph() {
	std::vector<NodeId> nodes;
	auto missingEdges = std::make_shared<std::set<Edge>>();

	for (auto &n : m_neighbours) {
		nodes.push_back(n.first);
	}

	for (int i = 0; i < nodes.size() - 1; i++) {
		for (int j = i + 1; j < nodes.size(); j++) {
			Edge e(nodes[i], nodes[j]);
			if (!checkIfExists(e))
				missingEdges->insert(e);
		}
	}

	return missingEdges;
}

std::shared_ptr<std::map<NodeId, std::set<Edge>>> UndirectedGraph::findAllFillEdges() {
	auto F = std::make_shared<std::map<NodeId, std::set<Edge>>>();

	auto GAlpha = std::make_shared<UndirectedGraph>(*this);

	for (auto it = GAlpha->m_neighbours.rbegin(); it != GAlpha->m_neighbours.rend();) {
		F->emplace(it->first, *GAlpha->edgesToCompleteCliqueOfNeighborhood(it->first));

		GAlpha->addEdges(F->at(it->first));

		GAlpha->removeNode(it->first);

		it = GAlpha->m_neighbours.rbegin();
	}

	return F;
}

std::map<Edge, std::set<Edge>>::iterator UndirectedGraph::getNextEmptyTuv(std::shared_ptr<std::map<Edge, std::set<Edge>>> edgeTuvs) {
	for (auto it = edgeTuvs->begin(); it != edgeTuvs->end(); it++)
		if (it->second.empty()) return it;

	return edgeTuvs->end();
}

NodeId UndirectedGraph::getVertexWithMaxLabel(NodeMap& GElim, std::map<NodeId, int>& vertexLabelMap) {
	auto maxV = vertexLabelMap.find(GElim.begin()->first);

	for (auto it = std::next(GElim.begin()); it != GElim.end(); it++) {
		if (maxV->second < vertexLabelMap.find(it->first)->second) maxV = vertexLabelMap.find(it->first);
	}

	return maxV->first;
}

bool UndirectedGraph::isGraphDense() {
	float nNodes = m_neighbours.size();

	float nEdges = m_edges.size();

	float density = 2 * nEdges / (nNodes * (nNodes - 1));

	if (density <= 0.5f) return false;
	return true;
}

std::shared_ptr<std::vector<Clique>> UndirectedGraph::maximalCliqueSet() {
	auto cliques = std::make_shared<std::vector<Clique>>();

	auto GElim = m_neighbours;

	NodeMap GNum;

	std::map<NodeId, int> vertexLabelMap;

	NodeId vertexPreviouslyChosen = -1;

	for (auto &node : GElim) {
		vertexLabelMap.emplace(node.first, 0);
	}

	int n = GElim.size() - 1;

	int lambda = 0;

	int cliqueId = 0;

	for (int i = n; i > -1; i--) {
		NodeId xi = getVertexWithMaxLabel(GElim, vertexLabelMap);

		updateNeighborhoodForCliqueTree(GNum, xi);

		if (i != n && vertexLabelMap.at(xi) <= lambda) {
			NodeId xi_1 = vertexPreviouslyChosen;

			NodeSet ns = GNum.find(xi_1)->second;
			ns.insert(xi_1);

			auto it = ns.find(xi);
			if (it != ns.end())
				ns.erase(it);

			cliques->emplace_back(ns, cliqueId);
			cliqueId++;
		}

		lambda = vertexLabelMap.at(xi);

		for (auto &y : GElim.at(xi)) {
			vertexLabelMap.at(y)++;
		}

		vertexPreviouslyChosen = xi;

		for (auto& node : GElim.at(xi)) {
			GElim.at(node).erase(xi);
		}
		GElim.erase(xi);
	}

	NodeSet ns = GNum.find(vertexPreviouslyChosen)->second;
	ns.insert(vertexPreviouslyChosen);

	cliques->emplace_back(ns, cliqueId);

	return cliques;
}

std::shared_ptr<std::set<Edge>> UndirectedGraph::minimalChordal(std::shared_ptr<std::map<NodeId, std::set<Edge>>> F) {
	auto addedEdges = std::make_shared<std::set<Edge>>();
	
	for (auto &node : *F){
		for (auto &edge : node.second) {
			addEdge(edge);
			addedEdges->insert(edge);
		}		
	}

	auto vertices = std::make_shared<NodeSet>();

	for (auto fi = F->begin(); fi != F->end(); fi++) {
		if (fi != F->begin())
			vertices->insert(std::prev(fi)->first);

		auto candidateSet = std::make_shared<std::set<Edge>>(fi->second);

		for (auto &uv : fi->second) {
			auto CNMi = commonNeighborhood(uv.getFirst(), uv.getSecond());

			auto CNMiIntersectionV = verticesIntersection(CNMi, vertices);

			for (auto x = CNMiIntersectionV->begin(); x != CNMiIntersectionV->end(); x++) {
				Edge e(*x, fi->first);

				if (!checkIfExists(e)) {
					candidateSet->erase(uv);
				}
			}
		}

		if (!candidateSet->empty()) {
			auto keepfill = cliqueMinTriang(candidateSet);
			for (auto &edge : *candidateSet) {
				removeEdge(edge);
				addedEdges->erase(edge);
			}

			for (auto & edge : *keepfill) {
				addEdge(edge);
				addedEdges->insert(edge);
			}
		}
	}

	return addedEdges;
}

void UndirectedGraph::thinningTriangulation(std::shared_ptr<std::set<Edge>> T) {
	auto R = std::make_shared<std::set<Edge>>(*T);

	NodeSet B;

	do {
		B.clear();

		for (auto &xy : *R) {
			auto intersection = commonNeighborhood(xy.getFirst(), xy.getSecond());

			if (checkIfIsClique(intersection)) {
				removeEdge(xy);
				T->erase(xy);
				B.insert(xy.getFirst());
				B.insert(xy.getSecond());

			}
		}
		R->clear();

		for (auto &edge : *T) {
			if (B.find(edge.getFirst()) != B.end() || B.find(edge.getSecond()) != B.end())
				R->insert(edge);
		}
	} while (!B.empty());
}

std::string UndirectedGraph::toString() {
	std::string s;

	for (auto &edge : m_edges) {
		s += edge.toString() + "\n";
	}

	return s;
}

void UndirectedGraph::triangulateGraph() {
	if (isGraphDense())
		triangulateDenseGraph();
	else
		triangulateNonDenseGraph();
}

void UndirectedGraph::triangulateDenseGraph() {
	auto F = edgesToCompleteGraph();

	auto FPrime = cliqueMinTriang(F);

	addEdges(*FPrime);
}

void UndirectedGraph::triangulateNonDenseGraph() {	
	//auto tempNeigh = m_neighbours;

	auto F = findAllFillEdges();

	//m_neighbours = tempNeigh;

	minimalChordal(F);

	//thinningTriangulation(addedEdges);
}

void UndirectedGraph::updateNeighborhoodForCliqueTree(NodeMap& GNum, NodeId n) {
	auto nodeNeighbours = m_neighbours.find(n);
	GNum.emplace(n, NodeSet());
	for (auto &node : nodeNeighbours->second) {
		auto GNumN = GNum.find(node);
		if (GNumN != GNum.end()) {
			GNumN->second.insert(n);
			GNum.at(n).insert(node);
		}
	}
}