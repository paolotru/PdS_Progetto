#ifndef OUR_READER_H
#define OUR_READER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "base/rapidxml.hpp"
#include "BayesianNetwork.h"
#include "../graph/GraphElements.h"
using namespace rapidxml;

//reader class for bayesian network stored in .xdsl files
//the template parameter is used to define the precision of the probability red from the file

template <class T>
class BNReader {
public:
    BNReader() = default;;

    //loads the bayesian network from the given file
    void loadNetworkFromFile(const std::string& fileName, std::shared_ptr<BayesianNetwork<T>> bn) {
        auto doc = std::make_shared<xml_document<>>();
        std::ifstream inputFile(fileName);
        auto buffer = std::make_shared<std::stringstream>();
        *buffer << inputFile.rdbuf();
        inputFile.close();
        std::string content(buffer->str());
        doc->parse<0>(&content[0]);

        xml_node<>* pRoot = doc->first_node();

        xml_node<>* pNodes = pRoot->first_node("nodes");

        int CPTcounter = 0;

        //reading all variables in file
        for (xml_node<>* pNode = pNodes->first_node("cpt"); pNode; pNode = pNode->next_sibling()) {

            xml_attribute<>* attr = pNode->first_attribute("id");

            std::string varName;

            std::vector<std::string> stateNames;

            int nStates;

            std::string parents;

            std::vector<VariableInformations> variablesOrder;

            std::vector<std::vector<int>> variablesCombinations;

            std::string probDistribution;

            std::string resultingStates;

            std::vector<std::string> resultingStatesSplitted;

            //reading variable name
            varName.append(attr->value());

            //reading all properties of the variable
            for (xml_node<>* pStates = pNode->first_node("state"); pStates; pStates = pStates->next_sibling()) {
                std::string name(pStates->name());

                //reading variable's states
                if (name == "state") {
                    attr = pStates->first_attribute("id");
                    if (attr != NULL)
                        stateNames.emplace_back(attr->value());
                } else if (name == "resultingstates") {
                    resultingStates.append(pStates->value());
                }
                    //reading cpt of the variable
                else if (name == "probabilities") {
                    probDistribution.append(pStates->value());
                }
                    //reading the parents order of the variable
                else if (name == "parents") {
                    parents.append(pStates->value());
                }
            }
            std::map<NodeId,std::vector<Status>> parentsM = splitParents(parents, bn);
            std::vector<T> probabilities = splitProbabilities(probDistribution);


/*controllare che non ci sia altra cpt identica, stessi stati e padri e fare nodo*/
/*************************************************************************************/
            Node<T> n(varName,stateNames,-1,probabilities,parentsM);
            bn->addNode(n);
            if(!parentsM.empty()){
                std::vector<NodeId> parentsId;
                for(auto &p : parentsM)
                    parentsId.push_back(p.first);
                bn->addArcs(n,parentsId);
            }
        }
    }
    //stateNames è insieme di stati in cui si trova noda
    //probabilità da associare
    //padri con il rispettivo stato
    bool checkEqualCPT(std::vector<ConditionalProbability<T>> cpt_table, std::vector<std::string> stateNames,std::vector<T> probabilities,std::map<NodeId,std::vector<Status>> parentsM, std::shared_ptr<BayesianNetwork<T>> bn) {
        bool flag;
        for (auto nodo: bn->getNodeMap()) { //ciclo su tutti i nodi
            flag= true;
                for (auto c: nodo.getSecond()->getCpt()->getCPTTable() && flag) { //cpt table
                    //controllo se hanno stessa dimensione ( vettore )
                    if (cpt_table.size() != c.size())
                        flag = false;
                    if (flag) {
                        int tot = 0;
                        auto prob = c->getProbability(); //prob
                        for (auto varinf: c->getVariableInfo()) { //varinf: stato nodo + stato dei vari padri
                            for (auto var: cpt_table->getVariableInfo()) { //ciclo su stati del nodo di interesse
                                if (varinf->getStatus() == var->getStatus() &&
                                    varinf->getParents() == var->getParents() && prob == cpt_table.getProbability())
                                    tot++;
                            }
                        }
                        if (tot == cpt_table.size()) {
                            //faccio un make shared e break
                            cpt_table = nullptr;
                            cpt_table = std::make_shared<T>(c);
                            break;
                        }
                    }
                }
        }
        return flag;
    }
private:

    std::map<NodeId,std::vector<Status>> splitParents(std::string& parents, std::shared_ptr<BayesianNetwork<T>> bn) {
        int pos;
        NodeId parentId;
        std::vector<Status> parent_states;
        std::map<NodeId,std::vector<Status>> parentsM;
        if (parents.empty())
            return parentsM;
        while ((pos = parents.find(" ")) != std::string::npos) {
            std::string parent = parents.substr(0, pos);
            parents = parents.substr(pos + 1);
            parentId=bn->idFromName(parent);
            parent_states=bn->getStatesById(parentId);
            parentsM.insert(std::pair<NodeId,std::vector<Status>>(parentId,parent_states));
        }
        parentId=bn->idFromName(parents);
        parent_states=bn->getStatesById(parentId);
        parentsM.insert(std::pair<NodeId,std::vector<Status>>(parentId,parent_states));
        return parentsM;
    }

    //splits the string containing the probability distribution and adds each combination of states with its probability to the CPT
    std::vector<T> splitProbabilities(std::string& probDistribution) {
        int pos;
        std::vector<T> probs;

        while ((pos = probDistribution.find(" ")) != std::string::npos) {
            T prob = std::stof(probDistribution.substr(0, pos));
            probDistribution = probDistribution.substr(pos + 1);
            probs.push_back(prob);

        }

        T prob = std::stof(probDistribution);

        probs.push_back(prob);

        return probs;
    }
};


#endif //OUR_READER_H
