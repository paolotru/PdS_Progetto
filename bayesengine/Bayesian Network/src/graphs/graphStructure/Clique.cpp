#include "Clique.h"

Clique::Clique() : EdgeSet(), m_cliqueId(-1) {
}

Clique::Clique(NodeId n1, NodeId n2, CliqueId id) : EdgeSet(), m_cliqueId(id) {
	addEdge(n1, n2);
}

Clique::Clique(NodeId n1, NodeId n2, NodeId n3, CliqueId id) : EdgeSet(), m_cliqueId(id) {
	addEdge(n1, n2);
	addEdge(n1, n3);
	addEdge(n2, n3);
}

Clique::Clique(std::set<NodeId>& nodes, CliqueId id): EdgeSet(), m_cliqueId(id) {
	for (auto it = nodes.begin(); it != std::prev(nodes.end()); it++)
		for (auto it2 = std::next(it); it2 != nodes.end(); it2++)
			addEdge(*it, *it2);
}

Clique::~Clique() {
}

bool Clique::operator< (const Clique& c) const {
	return m_cliqueId < c.m_cliqueId;
}

bool Clique::checkIfNodeIsPresent(NodeId node) {
	if (m_neighbours.find(node) != m_neighbours.end()) return true;
	return false;
}

CliqueId Clique::getCliqueId() {
	return m_cliqueId;
}

Separator Clique::intersectionWithClique(const Clique& c) {
	std::set<NodeId> nodes;

	auto it1 = m_neighbours.begin();
	auto it2 = c.m_neighbours.begin();

	for (it1, it2 ;;) {
		if (it1->first == it2->first) {
			nodes.insert(it1->first);
			it1++;
			it2++;
		}

		else if (it1->first < it2->first) it1++;
		else it2++;

		if (it1 == m_neighbours.end() || it2 == c.m_neighbours.end()) break;
	}

	Separator s(m_cliqueId, c.m_cliqueId, nodes);
	return s;
}

std::string Clique::toString() {
	std::string s;

	for (auto it = m_edges.begin(); it != m_edges.end(); it++) {
		s += it->toString() + "\n";
	}

	return s;
}

Separator::Separator(CliqueId c1, CliqueId c2, std::set<NodeId>& nodes) : m_clique1(std::min(c1, c2)), m_clique2(std::max(c1, c2)), m_nodes(nodes) {
}

Separator::Separator(const Separator& s) {
	m_clique1 = s.m_clique1;
	m_clique2 = s.m_clique2;
	m_nodes = s.m_nodes;
}

Separator::~Separator() {

}

Separator& Separator::operator=(const Separator& s) {
	if (this != &s) {
		m_clique1 = s.m_clique1;
		m_clique2 = s.m_clique2;
		m_nodes = s.m_nodes;
	}
	return *this;
}

bool Separator::operator==(const Separator& s) const {
	return m_clique1 == s.m_clique1 && m_clique2 == s.m_clique2;
}

bool Separator::operator<(const Separator& s) const {
	if (m_nodes.size() > s.m_nodes.size()) return true;
	else if (m_nodes.size() == s.m_nodes.size() && m_clique1 < s.m_clique1) return true;
	else if (m_nodes.size() == s.m_nodes.size() && m_clique1 == s.m_clique1 && m_clique2 < s.m_clique2) return true;
	return false;
}

bool Separator::checkIfVarIsPresent(const NodeId var) const {
	return m_nodes.find(var) != m_nodes.end();
}

CliqueId Separator::getFirst() const {
	return m_clique1;
}

CliqueId Separator::getSecond() const {
	return m_clique2;
}

CliqueId Separator::getOther(CliqueId clique) const {
	if (clique == m_clique1) return m_clique2;
	return m_clique1;
}

void Separator::setFirst(CliqueId clique) {
	m_clique1 = clique;
}

void Separator::setSecond(CliqueId clique) {
	m_clique2 = clique;
}

bool Separator::isIntersectionEmpty() const {
	return m_nodes.empty();
}

std::string Separator::toString() const {
	std::string s;
	auto it = m_nodes.begin();
	for (it; it != std::prev(m_nodes.end()); it++)
		s += std::to_string(*it) + "--";
	s += std::to_string(*it);
	return s;
}