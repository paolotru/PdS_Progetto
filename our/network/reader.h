//
// Created by S290225 on 11/08/2021.
//

#ifndef OUR_READER_H
#define OUR_READER_H


#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "base/rapidxml.hpp"
#include "BayesianNetwork.h"
#include "../graph/GraphElements.h"

typedef float T;
using namespace rapidxml;

//reader class for bayesian network stored in .xdsl files
//the template parameter is used to define the precision of the probability red from the file


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
                }
                else if (name == "resultingstates") {
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
                //else if (name == "property") {
                //	attr = pStates->first_attribute("id");
                //	std::string id(attr->value());


                //if (id == "VID") {
                //	varName.append(pStates->value());
                //}

                //else if (id == "parents_order") {
                //	parents.append(pStates->value());
                //}

                //else if (id == "cpt") {
                //	probDistribution.append(pStates->value());

                //}
                //}

            }
//            std::cout << "varName: " << varName << std::endl;
//            std::cout << "parents: " << parents << std::endl;
//            for (auto &s : stateNames) {
//                std::cout<<s<<std::endl;
//            }
            std::map<NodeId,std::vector<Status>> parentsM = splitParents(parents, bn);
            std::vector<float> probabilities = splitProbabilities(probDistribution);

//            std::cout << "splitparents: " << std::endl;
//            std::cout << "splitparents size: "<<parentsM.size() << std::endl;
//            for(auto it = parentsM.begin(); it != parentsM.end(); it++)
//                std::cout << it->first << std::endl;

            Node n(varName,stateNames,0,probabilities,parentsM);
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

private:

    ////given the vector of variables for the current CPT, inserts the initial combination into combination
    //void getInitialCombination(std::vector<VarStates>& states, std::vector<int>& combination) {
    //	for (int i = 0; i < states.size(); i++) {
    //		combination.push_back(0);
    //	}
    //}

    ////given the vector of variables for the current CPT, inserts in combination the next one to the that already stored
    //void getNextCombination(std::vector<VarStates>& states, std::vector<int>& combination) {
    //	int inc = 1;

    //	for (int i = combination.size() - 1; i > -1; i--) {
    //		combination[i] += inc;
    //		combination[i] = combination[i] % states[i].m_nStates;
    //		if (combination[i] != 0) break;
    //	}
    //}

    ////returns the number of combinations of states for the current CPT
    //int getNumberOfCombinations(std::vector<VarStates>& states) {
    //	int numCombinations = 1;
    //	for (auto it = states.begin(); it != states.end(); it++)
    //		numCombinations *= it->m_nStates;

    //	return numCombinations;
    //}

    //splits the string of parents into a vector containing their id and their number of states
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
    std::vector<float> splitProbabilities(std::string& probDistribution) {
        int pos;
        std::vector<float> probs;

        while ((pos = probDistribution.find(" ")) != std::string::npos) {
            float prob = std::stof(probDistribution.substr(0, pos));
            probDistribution = probDistribution.substr(pos + 1);
            probs.push_back(prob);

        }

        float prob = std::stof(probDistribution);

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
