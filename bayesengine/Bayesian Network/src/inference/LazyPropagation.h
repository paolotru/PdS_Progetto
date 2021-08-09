#pragma once

#include "../graphs/JunctionTree.h"

template <class T = float>
class LazyPropagation {
public:
	//constructor
	//parameters:
	//- a pointer to the bayesian network
	//- a pointer to the junction tree
	//- the list of evidences to be introduced
	//- the clique root to be used for the algorithm
	//- the target node 
	LazyPropagation(std::shared_ptr<BayesianNetwork<T>> bn, std::vector<Evidence>& evidences, CliqueId cliqueRoot, NodeId targetNode) :
		m_bn(bn), m_jt(std::make_shared<JunctionTree<T>>(bn)), m_completed(false), m_evidences(evidences), m_oldTargetVariable(targetNode), m_rootClique(cliqueRoot) {
		runLazyPropagation(evidences, cliqueRoot, targetNode);
	}

	//calculates the posterior marginal probability of variable
	CPT<T> posteriorMarginal(NodeId var) {

		//if the target node is different from the previous one, updates the messages of the junction tree
		updateNetworkIfNeeded(var);

		for (auto& clique : *m_jt->m_cliques) {
			if (clique.checkIfNodeIsPresent(var)) {
				for (auto& potential : m_jt->m_cliquePotentials.at(clique.getCliqueId())) {
					if (potential.getCPTId() == var) {
						CPTs<T> potentials;

						CliqueId cliqueId = clique.getCliqueId();

						for (auto& potential : m_jt->m_cliquePotentials.at(cliqueId)) {
							potentials.push_back(potential);
						}

						for (auto& message : m_jt->m_cliqueMessagesReceived.at(cliqueId)) {
							potentials.push_back(message->m_potential);
						}

						std::set<NodeSeparated> nodes;
						for (auto& potential : potentials) {
							for (auto& variable : potential.getVariables()) {
								nodes.emplace(variable.m_id);
							}
						}

						NodeSet targetSet;
						targetSet.insert(var);

						m_bn->isDSeparated(nodes, targetSet, m_evidenceNodes);

						findRelevantPotential(potentials, nodes, var);

						CPT<T> varPotential(potentials, var, true);

						auto variables = varPotential.getVariables();

						CPT<T> varPotentialMarginalized = varPotential;

						for (auto& parent : variables) {
							if (parent.m_id != var) {
								varPotentialMarginalized = varPotentialMarginalized.marginalizeVariable(parent.m_id);
							}
						}

						return varPotentialMarginalized;
					}
				}

			}
		}
	}

private:
	//absorption phase
	//parameters:
	//- the list of potential
	//- the list of separated nodes
	//- the target node 
	//returns a pair containing, the new CPT and a vector containing the potentials used to get the new CPT
	std::pair<CPT<T>, CPTs<T>> absorptionPhase(CPTs<T>& RS, std::set<NodeSeparated>& separatedNodes, NodeId targetNode) {
		CPTs<T> RsPrime = std::move(findRelevantPotential(RS, separatedNodes, targetNode));

		for (auto& evidence : m_evidences) {
			instantiatePotentials(evidence, RsPrime);
		}

		CPT<T> RsPotential;

		if (RsPrime.size() > 1)
			RsPotential = CPT<T>(RsPrime);
		else if (RsPrime.size() == 1)
			RsPotential = RsPrime[0];

		return std::make_pair(RsPotential, RsPrime);
	}

	//absorption phase for the collect evidence phase of lazy propagation
	//parameters:
	//- the list of potential
	//- the separator to the parent clique
	//- the clique id
	//- the root clique id
	//- the target node 
	std::pair<CPT<T>, CPTs<T>> collectAbsorptionPhase(CPTs<T>& RS, const Separator* parentSeparator, CliqueId clique, CliqueId cliqueRoot, NodeId targetNode) {
		for (auto& CPT : m_jt->m_cliquePotentials.at(clique)) {
			RS.push_back(CPT);
		}

		std::set<NodeSeparated> separatedNodes;

		for (auto& CPT : RS) {
			for (auto& node : CPT.getVariables())
				separatedNodes.emplace(node.m_id);
		}

		//finds the dSeparated nodes in the network and inserts them into separatedNodes
		m_bn->isDSeparated(separatedNodes, m_jt->m_cliques->at(parentSeparator->getOther(clique)).getNodes(), m_evidenceNodes);

		return absorptionPhase(RS, separatedNodes, targetNode);
	}

	//collect evidence not parallel
	//parameters:
	//- the clique id
	//- the parent clique id
	//- the root clique id
	//-the target node
	std::shared_ptr<Message<T>> collectEvidence(CliqueId clique, CliqueId parentClique, CliqueId cliqueRoot, NodeId targetNode) {
		std::shared_ptr<Message<T>> messageToBeSent = nullptr;

		CPTs<T> RS;

		const Separator* parentSeparator;

		for (auto& separator : m_jt->m_cliqueSeparators.at(clique)) {
			if (separator.getOther(clique) != parentClique) {
				auto receivedMessage = collectEvidence(separator.getOther(clique), clique, cliqueRoot, targetNode);

				//if the message received is valid then checks whether it is a new message or an update message
				if (receivedMessage != nullptr) {
					if (receivedMessage->m_isMessageNew) {
						if (receivedMessage->m_position == -1) {
							receivedMessage->m_position = m_jt->m_cliqueMessagesReceived.at(clique).size();
							m_jt->m_cliqueMessagesReceived.at(clique).push_back(receivedMessage);
						}
						else {
							m_jt->m_cliqueMessagesReceived.at(clique).at(receivedMessage->m_position) = receivedMessage;
						}
					}
				}
			}
			else {
				parentSeparator = &separator;
			}
		}

		//inserts into RS the potentials received with the messages
		for (auto& message : m_jt->m_cliqueMessagesReceived.at(clique)) {
			RS.push_back(message->m_potential);
		}

		if (parentClique != -1) {

			//runs the absorption phase
			auto potential_relevantPotentials = collectAbsorptionPhase(RS, parentSeparator, clique, cliqueRoot, targetNode);

			CPT<T> RsPotential = potential_relevantPotentials.first;
			CPTs<T> RsPrime = std::move(potential_relevantPotentials.second);

			//creates the message to send to the parent clique
			messageToBeSent = messageCreation(RsPotential, RsPrime, parentSeparator, clique);
		}

		return messageToBeSent;
	}

	//collect evidence not parallel
	//parameters:
	//- the clique id
	//- the parent clique id
	//- the root clique id
	//- the target node 
	std::shared_ptr<Message<T>> collectEvidenceParallel(CliqueId clique, CliqueId parentClique, CliqueId cliqueRoot, NodeId targetNode) {
		std::shared_ptr<Message<T>> messageToBeSent = nullptr;

		CPTs<T> RS;

		//keeps track of the messages that have to be collected from other threads
		std::vector<std::future<std::shared_ptr<Message<T>>>> resultMessagesFuture;

		//keeps track of the local instances of collectEvidence that are waiting to be executed
		std::vector<std::function<std::shared_ptr<Message<T>>()>*> localCollectToBePerformed;

		//lambda function that will be passed to the thread pool for its execution
		auto collectLambda = [&](CliqueId clique, CliqueId parentClique, CliqueId cliqueRoot)
			-> std::shared_ptr<Message<T>> {
			return collectEvidenceParallel(clique, parentClique, cliqueRoot, targetNode);
		};

		const Separator* parentSeparator;

		for (auto& separator : m_jt->m_cliqueSeparators.at(clique)) {
			if (separator.getOther(clique) != parentClique) {

				//submits the job to the thread pool only if it isn't already full
				auto res = ThreadPoolManager::trySubmitJob(collectLambda, separator.getOther(clique), clique, cliqueRoot);

				if (res.first) {
					//if the job has been submitted, save the relative future
					resultMessagesFuture.push_back(std::move(res.second));
				}
				else {
					std::unique_lock<std::mutex> ul(m_taskToBePerformedMutex);

					//if the threadpool is full, a pointer to the function to be executed is stored in a queue
					auto f = new std::function<std::shared_ptr<Message<T>>()>(std::bind(collectLambda, separator.getOther(clique), clique, cliqueRoot));

					m_collectToBePerformed.push(f);
					m_cv.notify_one();

					localCollectToBePerformed.push_back(f);
				}
			}
			else {
				parentSeparator = &separator;
			}
		}

		//vector of collected messages
		std::vector<std::shared_ptr<Message<T>>> resultMessages;

		for (auto& localJob : localCollectToBePerformed) {
			{
				std::unique_lock<std::mutex> ul(m_taskRunningMutex);
				//checks whether the instances of collectEvidence are running in another thread or not
				//if they are not running in another thread then they are executed in the current thread
				//if they are running in another thread then moves the corresponding future in the vector of messages to be collected				
				if (m_collectRunning.find(localJob) == m_collectRunning.end()) {
					//the instances currently running is marked are completed so it is not executed by another thread
					m_collectCompleted.insert(localJob);
					ul.unlock();

					resultMessages.push_back(localJob->operator()());
				}
				else {
					resultMessagesFuture.push_back(std::move(m_collectRunning.at(localJob)));
				}
			}
		}

		//the messages are collected from the corresponding future
		for (auto& resultFuture : resultMessagesFuture) {
			resultMessages.push_back(resultFuture.get());
		}

		for (auto& receivedMessage : resultMessages) {
			//if the message received is valid then checks whether it is a new message or an update message
			if (receivedMessage != nullptr) {
				if (receivedMessage->m_isMessageNew) {
					if (receivedMessage->m_position == -1) {
						receivedMessage->m_position = m_jt->m_cliqueMessagesReceived.at(clique).size();
						m_jt->m_cliqueMessagesReceived.at(clique).push_back(receivedMessage);
					}
					else
						m_jt->m_cliqueMessagesReceived.at(clique).at(receivedMessage->m_position) = receivedMessage;
				}
			}
		}

		for (auto& message : m_jt->m_cliqueMessagesReceived.at(clique)) {
			RS.push_back(message->m_potential);
		}

		if (parentClique != -1) {

			//runs the absorption phase
			auto potential_relevantPotentials = collectAbsorptionPhase(RS, parentSeparator, clique, cliqueRoot, targetNode);

			CPT<T> RsPotential = potential_relevantPotentials.first;
			CPTs<T> RsPrime = std::move(potential_relevantPotentials.second);

			//creates the message to send to the parent clique
			messageToBeSent = messageCreation(RsPotential, RsPrime, parentSeparator, clique);
		}
		else {
			m_completed.store(true);
			m_cv.notify_one();
		}

		return messageToBeSent;
	}

	//distribute evidence not parallel
	//parameters:
	//- the clique id
	//- the parent clique id
	//- the root clique id
	//- the target node 
	//- a pointer to the message received from the parent clique
	void distributeEvidence(CliqueId clique, CliqueId parentClique, CliqueId cliqueRoot, NodeId targetNode, std::shared_ptr<Message<T>> receivedMessage) {
		std::shared_ptr<Message<T>> messageToBeSent = nullptr;

		CPTs<T> RS;

		for (auto& CPT : m_jt->m_cliquePotentials.at(clique)) {
			RS.push_back(CPT);
		}

		//if the message received is valid then checks whether it is a new message or an update message
		if (receivedMessage != nullptr) {
			if (receivedMessage->m_isMessageNew) {
				if (receivedMessage->m_position == -1)
					m_jt->m_cliqueMessagesReceived.at(clique).push_back(receivedMessage);
				else {
					receivedMessage->m_position = m_jt->m_cliqueMessagesReceived.at(clique).size();
					m_jt->m_cliqueMessagesReceived.at(clique).at(receivedMessage->m_position) = receivedMessage;
				}
			}
		}

		std::set<NodeSeparated> separatedNodes;

		getNodesFromCliqueAndPotentials(clique, separatedNodes);

		//finds the dSeparated nodes in the network and inserts them into separatedNodes
		m_bn->isDSeparated(separatedNodes, m_jt->m_cliques->at(clique).getNodes(), m_evidenceNodes);

		for (auto& separator : m_jt->m_cliqueSeparators.at(clique)) {
			if (separator.getOther(clique) != parentClique) {
				for (auto& message : m_jt->m_cliqueMessagesReceived.at(clique)) {
					if (message->m_sourceClique != separator.getOther(clique)) {
						RS.push_back(message->m_potential);
					}
				}

				//runs the absorption phase
				auto potential_relevantPotentials = absorptionPhase(RS, separatedNodes, targetNode);

				CPT<T> RsPotential = potential_relevantPotentials.first;
				CPTs<T> RsPrime = std::move(potential_relevantPotentials.second);

				//creates the message to send to the child clique
				messageToBeSent = messageCreation(RsPotential, RsPrime, &separator, clique);

				distributeEvidence(separator.getOther(clique), clique, cliqueRoot, targetNode, messageToBeSent);

				messageToBeSent = nullptr;
			}
		}
	}

	//distribute evidence not parallel
	//parameters:
	//- the clique id
	//- the parent clique id
	//- the root clique id
	//- the target node 
	//- a pointer to the message received from the parent clique
	void distributeEvidenceParallel(CliqueId clique, CliqueId parentClique, CliqueId cliqueRoot, NodeId targetNode, std::shared_ptr<Message<T>> receivedMessage) {
		std::shared_ptr<Message<T>> messageToBeSent = nullptr;

		CPTs<T> RS;

		//keeps track of the messages that have to be collected from other threads
		std::vector<std::future<bool>> resultDistributeFuture;

		//keeps track of the local instances of collectEvidence that are waiting to be executed
		std::vector<std::function<void()>*> localDistributeToBePerformed;

		for (auto& CPT : m_jt->m_cliquePotentials.at(clique)) {
			RS.push_back(CPT);
		}

		int RSInitialSize = RS.size();

		//if the message received is valid then checks whether it is a new message or an update message
		if (receivedMessage != nullptr) {
			if (receivedMessage->m_isMessageNew) {
				if (receivedMessage->m_position == -1)
					m_jt->m_cliqueMessagesReceived.at(clique).push_back(receivedMessage);
				else {
					receivedMessage->m_position = m_jt->m_cliqueMessagesReceived.at(clique).size();
					m_jt->m_cliqueMessagesReceived.at(clique).at(receivedMessage->m_position) = receivedMessage;
				}
			}
		}

		std::set<NodeSeparated> separatedNodes;

		getNodesFromCliqueAndPotentials(clique, separatedNodes);

		//finds the dSeparated nodes in the network and inserts them into separatedNodes
		m_bn->isDSeparated(separatedNodes, m_jt->m_cliques->at(clique).getNodes(), m_evidenceNodes);

		for (auto& separator : m_jt->m_cliqueSeparators.at(clique)) {
			if (separator.getOther(clique) != parentClique) {
				for (auto& message : m_jt->m_cliqueMessagesReceived.at(clique)) {
					if (message->m_sourceClique != separator.getOther(clique)) {
						RS.push_back(message->m_potential);
					}
				}

				//runs the absorption phase
				auto potential_relevantPotentials = absorptionPhase(RS, separatedNodes, targetNode);

				CPT<T> RsPotential = potential_relevantPotentials.first;
				CPTs<T> RsPrime = std::move(potential_relevantPotentials.second);

				//creates the message to send to the child clique
				messageToBeSent = messageCreation(RsPotential, RsPrime, &separator, clique);

				//lambda function that will be passed to the thread pool for its execution
				auto distributeLambda = [this](CliqueId clique, CliqueId parentClique, CliqueId cliqueRoot, NodeId targetNode, std::shared_ptr<Message<T>> message) {
					distributeEvidenceParallel(clique, parentClique, cliqueRoot, targetNode, message);
				};

				//submits the job to the thread pool only if it isn't already full
				auto res = ThreadPoolManager::trySubmitJob(distributeLambda, separator.getOther(clique), clique, cliqueRoot, targetNode, messageToBeSent);

				if (res.first)
					//if the job has been submitted, save the relative future
					resultDistributeFuture.push_back(std::move(res.second));
				else {
					std::unique_lock<std::mutex> ul(m_taskToBePerformedMutex);

					//if the threadpool is full, a pointer to the function to be executed is stored in a queue
					auto f = new std::function<void()>(std::bind(distributeLambda, separator.getOther(clique), clique, cliqueRoot, targetNode, messageToBeSent));

					m_distributeToBePerformed.push(f);
					m_cv.notify_one();

					localDistributeToBePerformed.push_back(f);
				}

				messageToBeSent = nullptr;

				//removes the potentials added associated to the separators, leaving only the potentials of this clique
				RS.erase(RS.begin() + RSInitialSize, RS.end());
			}
		}

		for (auto& localJob : localDistributeToBePerformed) {
			{
				std::unique_lock<std::mutex> ul(m_taskRunningMutex);
				//checks whether the instances of collectEvidence are running in another thread or not
				//if they are not running in another thread then they are executed in the current thread
				//if they are running in another thread then moves the corresponding future in the vector of messages to be collected				
				if (m_distributeRunning.find(localJob) == m_distributeRunning.end()) {
					//the instances currently running is marked are completed so it is not executed by another thread
					m_distributeCompleted.insert(localJob);
					ul.unlock();

					localJob->operator()();
				}
				else {
					resultDistributeFuture.push_back(std::move(m_distributeRunning.at(localJob)));
				}
			}
		}

		for (auto& result : resultDistributeFuture) {
			result.get();
		}

		if (parentClique == -1) {
			m_completed.store(true);
			m_cv.notify_one();
		}
	}

	//finds the relevenat potentials to be passed to next clique.
	//removes potentials from the vector which aren't connected to the nodes contained in separatedNodes
	//removes potentials that contain barren head variable
	CPTs<T> findRelevantPotential(CPTs<T>& potentials, std::set<NodeSeparated>& separatedNodes, NodeId targetNode) {
		CPTs<T> relevantPotential;

		NodeSet relevantNodes;
		NodeSet nodesAlreadyIncluded;

		//removes potentials the aren't d-connected to the nodes contained in separatedNodes
		for (auto& CPT : potentials) {
			for (auto& node : separatedNodes) {
				if (!node.m_separated && CPT.getVariablePosition(node.m_id) != -1) {
					relevantPotential.push_back(CPT);

					for (auto& var : CPT.getVariables()) {
						relevantNodes.insert(var.m_id);
					}

					if (CPT.getCPTId() != -1)
						nodesAlreadyIncluded.insert(CPT.getCPTId());

					break;
				}
			}
		}

		for (auto& CPT : potentials) {
			if (CPT.getCPTId() != -1 && nodesAlreadyIncluded.find(CPT.getCPTId()) == nodesAlreadyIncluded.end() && relevantNodes.find(CPT.getCPTId()) != relevantNodes.end())
				relevantPotential.push_back(CPT);
		}

		//removes CPTs that contains barren head variables
		removeBarrenHeadVariables(relevantPotential, targetNode);

		return relevantPotential;
	}

	//inserts into nodesToMarginalize the list of nodes that have to marginalized
	void getNodesAfterMarginalization(CPT<T>& CPT, const Separator* parentSeparator, std::vector<VarStates>& nodesAfterMarginalization) {
		nodesAfterMarginalization = CPT.getVariables();
		for (auto it = nodesAfterMarginalization.begin(); it != nodesAfterMarginalization.end();) {
			if (!parentSeparator->checkIfVarIsPresent(it->m_id))
				it = nodesAfterMarginalization.erase(it);
			else
				it++;
		}
	}

	//inserts into nodes all the nodes from potentials of this clique and potentials received through messages and on which the d-Separation algorithm will be applied
	void getNodesFromCliqueAndPotentials(CliqueId clique, std::set<NodeSeparated>& nodes) {
		for (auto& CPT : m_jt->m_cliquePotentials.at(clique)) {
			for (auto& var : CPT.getVariables())
				nodes.insert(var.m_id);
		}

		for (auto& message : m_jt->m_cliqueMessagesReceived.at(clique)) {
			for (auto& var : message->m_potential.getVariables())
				nodes.insert(var.m_id);
		}
	}

	//instantiates a potential to the evidences introduced
	void instantiatePotentials(const Evidence& e, CPTs<T>& potentials) {
		for (auto& CPT : potentials) {
			CPT = CPT.instantiateVariable(e);
		}
	}

	//checks whether a variable is a barren variable
	//returns true if it is barren, false otherwise
	bool isBarrenVariable(NodeId node) {
		return m_notBarrenVariables->find(node) == m_notBarrenVariables->end();
	}

	//checks whether the new potential that will be inserted into the message will be equal or different from the old potential
	bool potentialToBeUpdated(CPT<T> newRsPotential, CPT<T> oldRsPotential, const Separator* parentSeparator) {
		std::vector<VarStates> nodesAfterMarginalization;

		//gets the list of variables that will be present in the potential after marginalization
		getNodesAfterMarginalization(newRsPotential, parentSeparator, nodesAfterMarginalization);

		//if the list of variables inside the old potential is equal to the new one then the new potential won't be calculated,
		//instead the old one will be used
		if (oldRsPotential.isVariableOrderEqualTo(nodesAfterMarginalization))
			return false;
		else
			return true;
	}

	//lazy propagation without parallelization
	void LazyPropagationSerialAlgorithm(CliqueId cliqueRoot, NodeId targetNode) {
		collectEvidence(cliqueRoot, -1, cliqueRoot, targetNode);

		distributeEvidence(cliqueRoot, -1, cliqueRoot, targetNode, nullptr);
	}

	//parallelized lazy propagation
	void LazyPropagationParallelAlgorithm(CliqueId cliqueRoot, NodeId targetNode) {

		//lambda function that will be passed to the thread pool for its execution
		auto collectLambda = [&](CliqueId clique, CliqueId parentClique, CliqueId cliqueRoot) {
			collectEvidenceParallel(clique, parentClique, cliqueRoot, targetNode);
		};

		//submits the initial job to the thread pool
		std::future<bool> completed = std::move(ThreadPoolManager::getInstance()->submit(collectLambda, cliqueRoot, -1, cliqueRoot));

		while (true) {
			std::function<std::shared_ptr<Message<T>>()>* task;

			{
				std::unique_lock<std::mutex> ul(m_taskToBePerformedMutex);
				//waits until there is a job to be executed that isn't executed by the thread pool or the initial job is finished
				m_cv.wait(ul, [&] {return !m_collectToBePerformed.empty() || (m_collectToBePerformed.empty() && m_completed.load()); });
				if (!m_completed.load()) {
					//gets the first job to be executed
					task = m_collectToBePerformed.front();
					m_collectToBePerformed.pop();
				}
				else
					break;
			}

			{
				std::unique_lock<std::mutex> ul(m_taskRunningMutex);
				//if the job hasn't been executed already, marks it as running and executes it
				if (m_collectCompleted.find(task) == m_collectCompleted.end()) {

					std::promise<std::shared_ptr<Message<T>>> promise;
					std::future<std::shared_ptr<Message<T>>> future = promise.get_future();
					//inserts its future inside the map so the thread waiting for it can get the result
					m_collectRunning.emplace(task, std::move(future));
					ul.unlock();

					//runs the job
					promise.set_value(task->operator()());
				}
			}
		}

		m_completed.store(false);

		//lambda function that will be passed to the thread pool for its execution
		auto distributeLambda = [&](CliqueId clique, CliqueId parentClique, CliqueId cliqueRoot) {
			distributeEvidenceParallel(cliqueRoot, -1, cliqueRoot, targetNode, nullptr);
		};

		//submits the initial job to the thread pool
		completed = std::move(ThreadPoolManager::getInstance()->submit(distributeLambda, cliqueRoot, -1, cliqueRoot));

		while (true) {
			std::function<void()>* task;

			{
				std::unique_lock<std::mutex> ul(m_taskToBePerformedMutex);
				//waits until there is a job to be executed that isn't executed by the thread pool or the initial job is finished
				m_cv.wait(ul, [&] {return !m_distributeToBePerformed.empty() || (m_distributeToBePerformed.empty() && m_completed.load()); });
				if (!m_completed.load()) {
					//gets the first job to be executed
					task = m_distributeToBePerformed.front();
					m_distributeToBePerformed.pop();
				}
				else
					break;
			}

			{
				std::unique_lock<std::mutex> ul(m_taskRunningMutex);
				//if the job hasn't been executed already, marks it as running and executes it
				if (m_distributeCompleted.find(task) == m_distributeCompleted.end()) {

					std::promise<bool> promise;
					std::future<bool> future = promise.get_future();
					//inserts its future inside the map so the thread waiting for it can get the result
					m_distributeRunning.emplace(task, std::move(future));
					ul.unlock();

					//runs the job
					task->operator()();
					promise.set_value(true);
				}
			}
		}
	}

	//marginalizes the potentials, wrt to the nodes contained in the separator
	CPT<T> marginalizePotential(CPT<T>& potential, const Separator* parentSeparator) {
		CPT<T> marginalizedPotential = potential;

		for (auto& var : potential.getVariables()) {
			if (!parentSeparator->checkIfVarIsPresent(var.m_id)) {
				marginalizedPotential = marginalizedPotential.marginalizeVariable(var.m_id);
			}
		}

		return marginalizedPotential;
	}

	//creates the message to be sent the parent/child clique
	//parameters:
	//- RsPotential: the potential to insert into the message
	//- RsPrime: the list of potentials to be used to calculate the potential
	//- separator: the separator to the parent/child clique
	//- the id of this clique
	std::shared_ptr<Message<T>> messageCreation(CPT<T>& RsPotential, CPTs<T>& RsPrime, const Separator* separator, CliqueId clique) {
		std::shared_ptr<Message<T>> messageToBeSent = nullptr;

		if (!RsPotential.isEmpty()) {
			std::shared_ptr<Message<T>> oldMessage = nullptr;

			//checks whether there is already a message sent from this clique in the child/parent clique
			for (auto& mess : m_jt->m_cliqueMessagesReceived.at(separator->getOther(clique))) {
				if (mess->m_sourceClique == clique) {
					oldMessage = mess;
					break;
				}
			}

			//if there isn't an old message or the old message contains a potential that is different from the current one need
			//evaluates the potential
			if (oldMessage == nullptr || (oldMessage != nullptr && potentialToBeUpdated(RsPotential, oldMessage->m_potential, separator))) {
				RsPotential.evaluateCPT(RsPrime);
				RsPotential = marginalizePotential(RsPotential, separator);

				if (!RsPotential.isUnitary()) {
					messageToBeSent = std::make_shared<Message<T>>(clique);
					messageToBeSent->m_potential = RsPotential;
					//if there is an old message, copies the position at which this message will be placed in the parent/child list of messages.
					if (oldMessage != nullptr)
						messageToBeSent->m_position = oldMessage->m_position;
				}
			}
			else {
				messageToBeSent = oldMessage;
				messageToBeSent->m_isMessageNew = false;
			}
		}

		return messageToBeSent;
	}

	//removes CPTs that have barren head variable
	void removeBarrenHeadVariables(CPTs<T>& potentials, NodeId targetNode) {
		NodeSet nodesRemoved;

		for (auto it = potentials.begin(); it != potentials.end();) {
			if (it->getCPTId() != -1 && isBarrenVariable(it->getCPTId())) {
				nodesRemoved.insert(it->getCPTId());
				it = potentials.erase(it);
			}
			else
				it++;
		}

		for (auto it = potentials.begin(); it != potentials.end();) {
			if (it->getCPTId() == -1) {
				bool variableRemoved = false;
				for (auto it2 = nodesRemoved.begin(); it2 != nodesRemoved.end(); it2++) {
					if (it->getVariablePosition(*it2) != -1) {
						it = potentials.erase(it);
						variableRemoved = true;
						break;
					}
				}

				if (!variableRemoved)
					it++;
			}
			else
				it++;
		}
	}

	//executes the lazy propagation algorithm
	void runLazyPropagation(std::vector<Evidence>& evidences, CliqueId cliqueRoot, NodeId targetNode) {
		for (int i = 0; i < evidences.size(); i++)
			m_evidenceNodes.insert(evidences[i].m_id);

		m_notBarrenVariables = m_bn->getNotBarrenVariables(targetNode, m_evidences);

		//decides whether to run the algorithm in parallel or not given the number of threads in the thread pool
		if (ThreadPoolManager::isThreadPoolInitialized()) {
			LazyPropagationParallelAlgorithm(cliqueRoot, targetNode);
		}
		else {
			LazyPropagationSerialAlgorithm(cliqueRoot, targetNode);
		}
	}

	//checks whether the messages need to be updated. if yes re-runs the lazy propagation algorithm
	void updateNetworkIfNeeded(NodeId target) {
		if (target != m_oldTargetVariable)
			runLazyPropagation(m_evidences, m_rootClique, target);
	}

	std::shared_ptr<BayesianNetwork<T>> m_bn;
	std::shared_ptr<JunctionTree<T>> m_jt;
	std::shared_ptr<NodeSet> m_notBarrenVariables;
	CliqueId m_rootClique;
	NodeId m_oldTargetVariable;
	std::vector<Evidence> m_evidences;
	NodeSet m_evidenceNodes;

	//queue containing function pointers to jobs that needs to be executed
	std::queue<std::function<std::shared_ptr<Message<T>>()>*> m_collectToBePerformed;
	//map containing function pointers of functions that are being executed and the relative future used to retreive the result
	std::map<std::function<std::shared_ptr<Message<T>>()>*, std::future<std::shared_ptr<Message<T>>>> m_collectRunning;
	//set containing function pointers of function that have already comleted execution
	std::set<std::function<std::shared_ptr<Message<T>>()>*> m_collectCompleted;

	//queue containing function pointers to jobs that needs to be executed
	std::queue<std::function<void()>*> m_distributeToBePerformed;
	//map containing function pointers of functions that are being executed and the relative future used to retreive the result
	std::map<std::function<void()>*, std::future<bool>> m_distributeRunning;
	//set containing function pointers of function that have already comleted execution
	std::set<std::function<void()>*> m_distributeCompleted;

	std::mutex m_taskToBePerformedMutex;
	std::mutex m_taskRunningMutex;
	std::condition_variable m_cv;
	std::atomic<bool> m_completed;
};