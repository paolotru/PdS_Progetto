#pragma once

#include <memory>

#include "EdgeSet.h"

typedef int CliqueId;

//separator used for junctionTree
class Separator {
public:
	//constructor
	//param:
	//c1 and c2 are the CliqueIds of the cliques connected by this separator
	//nodes is the intersection of nodes between the 2 cliques
	Separator(CliqueId c1, CliqueId c2, std::set<NodeId>& nodes);

	//copy constructor
	Separator(const Separator& s);

	//destructor
	~Separator();

	//copy operator
	Separator& operator=(const Separator& s);

	//boolean operator
	bool operator==(const Separator& s) const;

	bool operator<(const Separator& s) const;

	bool checkIfVarIsPresent(const NodeId var) const;

	//return CliqueId of separator's first clique
	CliqueId getFirst() const;

	//return CliqueId of separator's second clique
	CliqueId getSecond() const;

	//return CliqueId of separator's other clique
	CliqueId getOther(CliqueId node) const;

	//changes the separator's first clique
	void setFirst(CliqueId node);

	//changes the separator's second clique
	void setSecond(CliqueId node);

	//check wether the intersection of 2 cliques is empty or not
	bool isIntersectionEmpty() const;

	std::string toString() const;

private:
	CliqueId m_clique1, m_clique2;
	NodeSet m_nodes;
};

class Clique : public EdgeSet {
public:
	Clique();

	Clique(NodeId n1, NodeId n2, CliqueId id);

	Clique(NodeId n1, NodeId n2, NodeId n3, CliqueId id);

	Clique(std::set<NodeId>& nodes, CliqueId id);

	~Clique();

	bool operator< (const Clique& c) const;

	//finds the intersection with another clique
	//returns the separator defining the intersection
	Separator intersectionWithClique(const Clique& c);

	bool checkIfNodeIsPresent(NodeId node);

	CliqueId getCliqueId();

	std::string toString();

private:
	CliqueId m_cliqueId;
};