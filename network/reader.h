#ifndef OUR_READER_H
#define OUR_READER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>

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

            std::string parents;

            std::string probDistribution;

            std::string resultingStates;

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
            Node<T> n{};
            std::shared_ptr<CPT<T>> t = checkEqualCPT(stateNames, probabilities, parentsM, bn);
            if(t == nullptr) {
                Node<T> n1(varName, stateNames, -1, probabilities, parentsM);
                n=n1;
            }
            else {
                Node<T> n1(varName,stateNames, -1, t);
                n=n1;
            }
            bn->addNode(n);
            if(!parentsM.empty()){
                std::vector<NodeId> parentsId;
                for(auto &p : parentsM)
                    parentsId.push_back(p.first);
                bn->addArcs(n,parentsId);
            }
        }
    }
private:
    std::shared_ptr<CPT<T>> checkEqualCPT(std::vector<std::string> stateNames, std::vector<T> probabilities, std::map<NodeId,std::vector<Status>> parentsM, std::shared_ptr<BayesianNetwork<T>> bn) {
        int tot;
        auto nodes = bn->getGraph()->getNodes();
        for (auto node: nodes) {
            tot = 0;
            auto cpt = node.getCpt();
            if (probabilities.size() != cpt->getCPTTable().size())
                continue;
            CPT<T> testCpt(probabilities, parentsM, stateNames);
            if(testCpt == *cpt){
                return cpt;
            }
//            std::vector<NodeId> parentsId;
//            std::vector<std::vector<Status>> parentsStatuses;
//            for(auto & parent : parentsM) {
//                parentsId.push_back(parent.first);
//                parentsStatuses.push_back(parent.second);
//            }
//            std::vector<std::map<NodeId,Status>> v_map;
//            std::map<NodeId,Status> map;
//            CPT<T>::rec_f(0,parentsM.size(),v_map,map,parentsId,parentsStatuses);
//            for(auto comb=v_map.begin(); comb != v_map.end(); comb++){
//                for(auto s : stateNames) {
//
//                }
//            }
//            T prob = cptTable->getProbability();
//            for (auto varinf: cptTable->getVariableInfo()) { //varinf: stato node + stato dei vari padri
//                for(int i = 0; i < stateNames.size(); i++) { //ciclo su stati del node di interesse
//                    if (varinf->getStatus() == stateNames[i] &&
//                    varinf->getParents() == pare && prob == cpt_table.getProbability())
//                        tot++;
//                }
//            }
//            if (tot == probabilities.size()) {
//                return std::pair<bool, std::shared_ptr<CPT<T>>>(true, cptTable);
//            }
        }
        return nullptr;
    }

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
