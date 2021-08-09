#pragma once

#include "graphStructure/Clique.h"

class UndirectedGraph : public EdgeSet {
public:
	UndirectedGraph();

	UndirectedGraph(const UndirectedGraph& ug);

	~UndirectedGraph();

	//finds all the maximal cliques inside the triangulated graph using a brute force approach.
	//returns a vector containing the cliques found
	//TO-DO: improve it using an heuristic
	std::shared_ptr<std::vector<Clique>> maximalCliqueSet();

	//triangulates the moralized graph
	void triangulateGraph();

	std::string toString();

private:
	//returns the set of edges required to find a minimum triangulation for a moral graph
	std::shared_ptr<std::set<Edge>> cliqueMinTriang(std::shared_ptr<std::set<Edge>> F);

	//returns the set of edges the would make the neighoborhood of n a clique
	std::shared_ptr<std::set<Edge>> edgesToCompleteCliqueOfNeighborhood(const NodeId n);

	//returns the set of edges that would make the undirected graph a complete graph
	std::shared_ptr<std::set<Edge>> edgesToCompleteGraph();

	//finds all fill edges needed to triangulate a non dense graph
	std::shared_ptr<std::map<NodeId,std::set<Edge>>> findAllFillEdges();

	//returns the iterator to the next entry which has an empty set<Edge>
	std::map<Edge, std::set<Edge>>::iterator getNextEmptyTuv(std::shared_ptr < std::map<Edge, std::set<Edge>>> edgeTuvs);

	//returns the node with max label inside vertexLabelMap
	NodeId getVertexWithMaxLabel(NodeMap& GElim, std::map<NodeId, int>& vertexLabelMap);

	//checks whether the graph is dense or not
	//returns true if it is dense, false otherwise
	bool isGraphDense();

	//returns a chordal graph given GAlpha as source
	std::shared_ptr<std::set<Edge>> minimalChordal(std::shared_ptr<std::map<NodeId, std::set<Edge>>> GAlpha);

	//eliminates useless edges added during triangulation
	void thinningTriangulation(std::shared_ptr<std::set<Edge>> addedEdges);

	//traingulates dense graphs
	void triangulateDenseGraph();

	//traingulates non dense graphs
	void triangulateNonDenseGraph();

	//adds node n to neighborhood GNum
	void updateNeighborhoodForCliqueTree(NodeMap& GNum, NodeId n);
};