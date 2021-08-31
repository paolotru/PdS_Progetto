//
// Created by S290225 on 11/08/2021.
//

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
            std::tuple<bool, std::shared_ptr<CPT<T>>> t = checkEqualCPT(stateNames, probabilities, parentsM, bn);
            if(t.first == false)
                Node<T> n(varName,stateNames,-1,probabilities,parentsM);
            else
                Node<T> n(varName,-1,t.second);
            bn->addNode(n);
            if(!parentsM.empty()){
                std::vector<NodeId> parentsId;
                for(auto &p : parentsM)
                    parentsId.push_back(p.first);
                bn->addArcs(n,parentsId);
            }
        }

//        bn.checkSparseCPTs();
//
//        bn.addArcsFromCPTs();
    }
    //stateNames è insieme di stati in cui si trova noda
    //probabilità da associare
    //padri con il rispettivo stato
    std::tuple<bool, std::shared_ptr<CPT<T>>> checkEqualCPT(std::vector<std::string> stateNames, std::vector<T> probabilities, std::map<NodeId,std::vector<Status>> parentsM, std::shared_ptr<BayesianNetwork<T>> bn) {
        int tot;
        auto nodes = bn->getGraph()->getNodes();
        for (auto node: nodes) {
            tot = 0;
            //controllo se hanno stessa dimensione ( vettore )
            auto cptTable = node->getCpt()->getCPTTable();

            if (probabilities.size() != cptTable.size())
                continue;

            T prob = cptTable->getProbability();
            for (auto varinf: cptTable->getVariableInfo()) { //varinf: stato node + stato dei vari padri
                for(int i = 0; i < stateNames.size(); i++) { //ciclo su stati del node di interesse
                    if (varinf->getStatus() == stateNames[i] &&
                    varinf->getParents() == pare && prob == cpt_table.getProbability())
                        tot++;
                }
            }
            if (tot == probabilities.size()) {
                return std::tuple<bool, std::shared_ptr<CPT<T>>>(true, cptTable);
            }
        }

        return std::tuple<bool, std::shared_ptr<CPT<T>>>(false, nullptr);
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

    //splits the string containing the resulting states into a vector of string. returns the number of states found.
    //used for deterministic variables
//    int splitResultingStates(std::string& resultingStates, std::vector<std::string>& resultingStatesSplitted) {
//        if (resultingStates.empty()) return 0;
//        int pos;
//        while ((pos = resultingStates.find(" ")) != std::string::npos) {
//            std::string state = resultingStates.substr(0, pos);
//            resultingStates = resultingStates.substr(pos + 1);
//
//            resultingStatesSplitted.push_back(state);
//        }
//
//        resultingStatesSplitted.push_back(resultingStates);
//
//        return resultingStatesSplitted.size();
//    }
};


#endif //OUR_READER_H
